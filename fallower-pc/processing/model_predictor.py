import os
import time
import queue
import threading
import numpy as np
import onnxruntime as ort
from typing import Optional, Tuple, List, Dict, Any
from datetime import datetime

# Gerekli core ve data modüllerini import et
from core.config import Configuration, ModelConfig
from core.logger import Logger
from core.event_bus import EventBus, Event
from core.enums import EventType
from data.models import PredictionResult


class ModelPredictor:
    """ONNX modelini kullanarak tahmin yapar."""
    def __init__(self):
        self.config = Configuration()
        self.logger = self.config.get_logger()
        self.event_bus = EventBus()

        # Model Configuration
        self.current_model_config: Optional[ModelConfig] = None
        self.model_path: Optional[str] = None
        self.class_labels: List[str] = []
        self.confidence_threshold: float = 0.7
        self.input_shape: Tuple = (1, 30, 1024)

        # ONNX Runtime Session
        self.session: Optional[ort.InferenceSession] = None
        self.input_name: Optional[str] = None
        self.output_name: Optional[str] = None
        
        # Doğrudan kuyruk ve metrikler
        self._direct_queue = None
        self._direct_mode = False
        self._timing_stats = {
            'total_predictions': 0,
            'total_time': 0,
            'max_time': 0,
            'min_time': float('inf'),
            'last_10_times': []
        }
        
        # Modelin Input/Output şablonu
        self._input_template = None

        # Tahmin işleme
        self._prediction_queue = queue.Queue(maxsize=10000)  # 100'den 10000'e çıkarıldı
        self._prediction_thread: Optional[threading.Thread] = None
        self._running = False

        # Load initial model configuration
        self._update_model_configuration()

        self.event_bus.subscribe(EventType.WINDOW_READY_FOR_PREDICTION, self.handle_window_ready)
    
    def _update_model_configuration(self):
        """Update model configuration from config"""
        self.current_model_config = self.config.get_current_model()
        model_config = self.config.get('model')
        
        self.model_path = model_config.get('path')
        self.class_labels = model_config.get('class_labels', [])
        self.confidence_threshold = model_config.get('confidence_threshold', 0.7)
        self.input_shape = tuple(model_config.get('input_shape', (1, 30, 1024)))

        # Log model info
        if self.current_model_config:
            self.logger.info(f"Model configuration updated:")
            self.logger.info(f"  Model: {self.current_model_config.name}")
            self.logger.info(f"  Pipeline: {self.current_model_config.pipeline.value}")
            self.logger.info(f"  File: {self.current_model_config.filename}")
            self.logger.info(f"  Confidence threshold: {self.confidence_threshold}")

        # Validate model file
        if not self.model_path or not os.path.exists(self.model_path):
            self.logger.error(f"ONNX model file not found at path: {self.model_path}")
            self.model_path = None
        if not self.class_labels or not isinstance(self.class_labels, list):
            self.logger.error("Class labels are missing or not a list in configuration.")
            self.class_labels = []
    
    def setup_direct_mode(self, direct_queue=None):
        """DataProcessor ile doğrudan iletişim için kurulum"""
        if direct_queue:
            self._direct_queue = direct_queue
        else:
            self._direct_queue = queue.Queue(maxsize=100)
        
        self._direct_mode = True
        self.logger.info("Direct queue mode activated with DataProcessor")
        return self._direct_queue

    def load_model(self) -> bool:
        """ONNX Runtime optimizasyonları ile model yükleme"""
        if not self.model_path:
            self.logger.error("Model path not configured")
            return False
        
        try:
            self.logger.info(f"Loading ONNX model from: {self.model_path}")
            
            # ONNX Runtime SessionOptions ile optimizasyon
            from onnxruntime.capi.onnxruntime_pybind11_state import SessionOptions, GraphOptimizationLevel
            
            session_options = SessionOptions()
            session_options.graph_optimization_level = GraphOptimizationLevel.ORT_ENABLE_ALL
            session_options.intra_op_num_threads = 4  # CPU çekirdek sayısına göre ayarlayın
            session_options.execution_mode = ort.ExecutionMode.ORT_SEQUENTIAL
            session_options.enable_mem_pattern = True
            session_options.enable_mem_reuse = True
            
            # Provider seçimi
            available_providers = ort.get_available_providers()
            providers_to_use = []
            
            if 'CUDAExecutionProvider' in available_providers:
                cuda_options = {
                    'device_id': 0,
                    'arena_extend_strategy': 'kNextPowerOfTwo',
                    'gpu_mem_limit': 2 * 1024 * 1024 * 1024,  # 2GB
                    'cudnn_conv_algo_search': 'EXHAUSTIVE'
                }
                providers_to_use.append(('CUDAExecutionProvider', cuda_options))
            elif 'TensorrtExecutionProvider' in available_providers:
                providers_to_use.append('TensorrtExecutionProvider')
            
            providers_to_use.append('CPUExecutionProvider')
            
            self.logger.info(f"Using providers: {providers_to_use}")
            
            # Optimize edilmiş session oluştur
            self.session = ort.InferenceSession(
                self.model_path, 
                sess_options=session_options,
                providers=providers_to_use
            )
            
            # Model input/output info
            inputs = self.session.get_inputs()
            outputs = self.session.get_outputs()
            
            if not inputs or not outputs:
                raise ValueError("Model has no inputs or outputs")
            
            self.input_name = inputs[0].name
            self.output_name = outputs[0].name
            
            # Input şablonu oluştur
            input_shape = inputs[0].shape
            if len(input_shape) == 4:  # (batch_size, channels, height, width)
                # Dinamik batch boyutu olabilir
                batch_size = 1
                channels = input_shape[1] if input_shape[1] != None else 3
                height = input_shape[2] if input_shape[2] != None else 30
                width = input_shape[3] if input_shape[3] != None else 688
                
                # Şablonu oluştur (belleği önceden ayır)
                self._input_template = np.zeros((batch_size, channels, height, width), dtype=np.float32)
            
            model_input_shape = inputs[0].shape
            model_output_shape = outputs[0].shape
            
            self.logger.info(f"Model Input: name='{self.input_name}', shape={model_input_shape}")
            self.logger.info(f"Model Output: name='{self.output_name}', shape={model_output_shape}")
            self.logger.info(f"Model loaded successfully: {self.current_model_config.name}")
            
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to load model: {e}", exc_info=True)
            self.session = None
            self.event_bus.publish(Event(EventType.ERROR_OCCURRED, data={"error": str(e), "source": "ModelLoader"}, source="ModelPredictor"))
            return False

    def reload_model(self) -> bool:
        """Reload model with new configuration"""
        self.logger.info("Reloading model with new configuration...")
        
        # Stop current session
        if self.session:
            self.session = None
            self._input_template = None
        
        # Update configuration
        self._update_model_configuration()
        
        # Load new model
        return self.load_model()

    def change_model(self, model_key: str) -> bool:
        """Change to a different model"""
        if not self.config.set_current_model(model_key):
            return False
        
        # Reload with new model
        success = self.reload_model()
        
        if success:
            self.logger.info(f"Successfully changed to model: {model_key}")
            # Reset timing stats for new model
            self._timing_stats = {
                'total_predictions': 0,
                'total_time': 0,
                'max_time': 0,
                'min_time': float('inf'),
                'last_10_times': []
            }
        else:
            self.logger.error(f"Failed to change to model: {model_key}")
        
        return success

    def get_available_models(self) -> Dict[str, str]:
        """Get list of available models"""
        models = self.config.get_available_models()
        return {key: config.name for key, config in models.items()}

    def get_current_model_info(self) -> Dict[str, Any]:
        """Get current model information"""
        if not self.current_model_config:
            return {}
        
        return {
            'name': self.current_model_config.name,
            'filename': self.current_model_config.filename,
            'pipeline': self.current_model_config.pipeline.value,
            'confidence_threshold': self.current_model_config.confidence_threshold,
            'class_labels': self.current_model_config.class_labels,
            'loaded': self.session is not None
        }

    def start(self) -> bool:
        if self._running:
            self.logger.warning("ModelPredictor is already running.")
            return True

        if not self.load_model(): # Modeli yüklemeyi dene
             self.logger.error("ModelPredictor cannot start because model loading failed.")
             return False # Model yüklenemezse başlatma

        self._running = True
        self._prediction_thread = threading.Thread(target=self._prediction_loop, name="PredictionThread")
        self._prediction_thread.daemon = True
        self._prediction_thread.start()
        
        # Start metodunda thread önceliğini ayarla
        try:
            import os
            if os.name == 'posix':  # Linux
                import resource
                # Nice değerini düşür (-20 en yüksek, 19 en düşük öncelik)
                os.nice(-10)
            elif os.name == 'nt':  # Windows
                import psutil
                p = psutil.Process()
                p.nice(psutil.HIGH_PRIORITY_CLASS)
        except ImportError:
            self.logger.warning("psutil not available, thread priority not set")
        except Exception as e:
            self.logger.warning(f"Could not set thread priority: {e}")
        
        self.logger.info("ModelPredictor started.")
        return True

    def stop(self):
        if not self._running:
            return

        self.logger.info("Stopping ModelPredictor...")
        self._running = False

        if self._prediction_thread and self._prediction_thread.is_alive():
             self._prediction_queue.put(None)
             if self._direct_queue:
                 self._direct_queue.put(None)
             self._prediction_thread.join(timeout=2.0)
             if self._prediction_thread.is_alive():
                 self.logger.warning("Prediction thread did not exit gracefully.")

        self._prediction_thread = None
        self.session = None # Modeli bellekten kaldır
        self.logger.info("ModelPredictor stopped.")

    def handle_window_ready(self, event: Event):
        if not self._running or not self.session or self._direct_mode: # Doğrudan modda EventBus'dan dinleme
            return
        if event.event_type == EventType.WINDOW_READY_FOR_PREDICTION:
            try:
                self._prediction_queue.put_nowait(event.data)
            except queue.Full:
                self.logger.warning("Prediction queue full! Discarding window.")

    def _prediction_loop(self):
        self.logger.info("Prediction loop started.")
        
        # Son 10 tahmin süresi için sliding window
        time_window = []
        window_size = 10
        
        while self._running:
            try:
                # Doğrudan modda mı yoksa EventBus kuyruğu üzerinden mi veri al
                if self._direct_mode and self._direct_queue:
                    try:
                        window_info = self._direct_queue.get(timeout=1.0)
                    except queue.Empty:
                        continue
                else:
                    window_info = self._prediction_queue.get(timeout=1.0)
                
                if window_info is None:
                    break
                
                if not self.session:
                    self.logger.warning("Skipping prediction, session not available")
                    if self._direct_mode and self._direct_queue:
                        self._direct_queue.task_done()
                    else:
                        self._prediction_queue.task_done()
                    continue
                
                # Zamanlama başlat
                start_time = time.perf_counter()
                
                window_data = window_info.get("window_data")
                time_range = window_info.get("time_range")
                
                if window_data is None or time_range is None:
                    self.logger.warning("Incomplete window info")
                    # task_done çağrılmalı — yoksa queue.join() deadlock olur
                    if self._direct_mode and self._direct_queue:
                        self._direct_queue.task_done()
                    else:
                        self._prediction_queue.task_done()
                    continue
                
                # İstatistikleri artır (_predict içindeki debug logları doğru çalışsın)
                self._timing_stats['total_predictions'] += 1
                
                # Tahmin yap
                prediction_result = self._predict(window_data, time_range)
                
                # Zamanlama bitir ve istatistikleri güncelle
                end_time = time.perf_counter()
                elapsed_ms = (end_time - start_time) * 1000
                
                # Zamanlama istatistiklerini güncelle
                self._timing_stats['total_time'] += elapsed_ms
                self._timing_stats['max_time'] = max(self._timing_stats['max_time'], elapsed_ms)
                self._timing_stats['min_time'] = min(self._timing_stats['min_time'], elapsed_ms)
                
                # Son 10 tahminin ortalaması için
                self._timing_stats['last_10_times'].append(elapsed_ms)
                if len(self._timing_stats['last_10_times']) > 10:
                    self._timing_stats['last_10_times'].pop(0)
                
                # Her 100 tahminde bir istatistik logla
                # if self._timing_stats['total_predictions'] % 100 == 0:
                #     avg_time = self._timing_stats['total_time'] / self._timing_stats['total_predictions']
                #     avg_last_10 = sum(self._timing_stats['last_10_times']) / len(self._timing_stats['last_10_times'])
                #     fps = 1000 / avg_last_10 if avg_last_10 > 0 else 0
                    
                #     self.logger.info(
                #         f"Prediction stats ({self.current_model_config.name}): "
                #         f"Avg={avg_time:.2f}ms, Recent={avg_last_10:.2f}ms, "
                #         f"FPS={fps:.1f}, Min={self._timing_stats['min_time']:.2f}ms, "
                #         f"Max={self._timing_stats['max_time']:.2f}ms"
                #     )
                
                if prediction_result:
                    self.event_bus.publish(Event(
                        EventType.PREDICTION_READY,
                        data=prediction_result,
                        source="ModelPredictor"
                    ))
                
                # Task done
                if self._direct_mode and self._direct_queue:
                    self._direct_queue.task_done()
                else:
                    self._prediction_queue.task_done()
                
            except queue.Empty:
                continue
            except Exception as e:
                self.logger.error(f"Error in prediction loop: {e}", exc_info=True)
                time.sleep(0.1)
        
        self.logger.info("Prediction loop finished")

# model_predictor.py içine eklenecek debug kodu

    def _predict(self, window_data: np.ndarray, time_range: Tuple[datetime, datetime]) -> Optional[PredictionResult]:
        """Optimize edilmiş tahmin - daha az kopyalama"""
        if not self.session or not self.input_name or not self.output_name or not self.class_labels:
            return None
        
        try:
            # ===========================================
            #  DEBUG: MODEL INPUT ANALYSIS
            # ===========================================
            
            # 1. Window veri analizi
            window_mean = np.mean(window_data)
            window_std = np.std(window_data)
            window_min = np.min(window_data)
            window_max = np.max(window_data)
            window_hash = hash(window_data.tobytes())  # Veri benzersizliği kontrolü
            
            # 2. İlk ve son frame karşılaştırması
            first_frame_mean = np.mean(window_data[0])
            last_frame_mean = np.mean(window_data[-1])
            frame_variance = np.var(np.mean(window_data, axis=1))  # Frame'ler arası varyans
            
            # 3. Log debug bilgileri (her 10 tahmin de bir)
            self._debug_predict_count = getattr(self, '_debug_predict_count', 0) + 1
            if self._debug_predict_count % 10 == 0:
                self.logger.info("MODEL INPUT DEBUG:")
                self.logger.info(f"  Model: {self.current_model_config.name}")
                self.logger.info(f"  Pipeline: {self.current_model_config.pipeline.value}")
                self.logger.info(f"  Window Shape: {window_data.shape}")
                self.logger.info(f"  Data Hash: {window_hash} (değişiyor mu?)")
                self.logger.info(f"  Mean: {window_mean:.4f}, Std: {window_std:.4f}")
                self.logger.info(f"  Range: [{window_min:.4f}, {window_max:.4f}]")
                self.logger.info(f"  First Frame Mean: {first_frame_mean:.4f}")
                self.logger.info(f"  Last Frame Mean: {last_frame_mean:.4f}")
                self.logger.info(f"  Frame Variance: {frame_variance:.6f} (düşükse problem!)")
                
                # İlk birkaç değeri göster
                first_values = window_data[0][:10]
                last_values = window_data[-1][:10]
                self.logger.info(f"  First Frame [0:10]: {first_values}")
                self.logger.info(f"  Last Frame [0:10]: {last_values}")
            
            # Model inputu hazırla
            # Önceden ayrılmış şablonu kullan
            if self._input_template is not None:
                # Şablonu sıfırla (daha hızlı)
                self._input_template.fill(0)

                # Veriyi şablona yerleştir
                # 1-channel model için: sadece ilk channel'a koy
                # 3-channel model için: tüm channel'lara aynı değeri koy
                for c in range(self._input_template.shape[1]):
                    self._input_template[0, c] = window_data

                input_data = self._input_template
            else:
                # Şablon yoksa, geleneksel yolla input oluştur
                input_data = window_data.astype(np.float32)
                # (30, 688) -> (1, 1, 30, 688)
                input_data = np.expand_dims(np.expand_dims(input_data, axis=0), axis=0)

                # FIXED: Check expected channel count from input_shape
                expected_channels = self.input_shape[1] if len(self.input_shape) >= 2 else 1

                if expected_channels > 1:
                    # Multi-channel model: repeat data across channels
                    # (1, 1, 30, 688) -> (1, 3, 30, 688)
                    input_data = np.repeat(input_data, expected_channels, axis=1)
                # else: Keep as 1-channel (1, 1, 30, 688)
            
            # ===========================================
            #  DEBUG: PROCESSED INPUT ANALYSIS  
            # ===========================================
            
            processed_mean = np.mean(input_data)
            processed_std = np.std(input_data)
            processed_hash = hash(input_data.tobytes())
            
            if self._timing_stats['total_predictions'] % 10 == 0:
                self.logger.info(" PROCESSED INPUT DEBUG:")
                self.logger.info(f"  Processed Shape: {input_data.shape}")
                self.logger.info(f"  Processed Hash: {processed_hash}")
                self.logger.info(f"  Processed Mean: {processed_mean:.4f}, Std: {processed_std:.4f}")
                
                # İlk channel'ın ilk frame'inin ilk 10 değeri
                sample_values = input_data[0, 0, 0, :10]
                self.logger.info(f"  Sample Input [0,0,0,:10]: {sample_values}")
            
            # Modeli çalıştır
            input_feed = {self.input_name: input_data}
            outputs = self.session.run([self.output_name], input_feed)
            raw_scores = outputs[0][0]
            
            # ===========================================
            #  DEBUG: MODEL OUTPUT ANALYSIS
            # ===========================================
            
            if self._timing_stats['total_predictions'] % 10 == 0:
                self.logger.info(" MODEL OUTPUT DEBUG:")
                self.logger.info(f"  Raw Scores: {raw_scores}")
                self.logger.info(f"  Score Range: [{np.min(raw_scores):.4f}, {np.max(raw_scores):.4f}]")
            
            # Sonuçları işle
            probabilities = self._softmax(raw_scores)
            predicted_class_index = np.argmax(probabilities)
            confidence = probabilities[predicted_class_index]
            
            # ===========================================
            #  DEBUG: FINAL RESULTS
            # ===========================================
            
            if self._timing_stats['total_predictions'] % 10 == 0:
                self.logger.info(" PREDICTION DEBUG:")
                self.logger.info(f"  All Probabilities: {probabilities}")
                self.logger.info(f"  Predicted Index: {predicted_class_index}")
                self.logger.info(f"  Confidence: {confidence:.4f}")
                self.logger.info("-" * 50)
            
            # Eşik kontrolü
            if confidence >= self.confidence_threshold and predicted_class_index < len(self.class_labels):
                predicted_label = self.class_labels[predicted_class_index]
                result = PredictionResult(
                    label=predicted_label,
                    confidence=float(confidence),
                    time_range=time_range,
                    raw_scores=raw_scores
                )
                return result
            else:
                if predicted_class_index >= len(self.class_labels):
                    self.logger.warning(f"Predicted class index {predicted_class_index} is out of bounds for labels list (size {len(self.class_labels)}).")
                return None
                
        except Exception as e:
            self.logger.error(f"Prediction error: {e}", exc_info=True)
            return None
            
    def _softmax(self, x: np.ndarray) -> np.ndarray:
        """Numerically stable softmax."""
        if x.ndim == 0 : # Skaler ise softmax anlamsız
            return np.array([1.0]) if np.isfinite(x) else np.array([np.nan])
        if x.size == 0: # Boş dizi ise
             return np.array([])

        # Stabilite için en büyük elemanı çıkar
        max_val = np.max(x, axis=-1, keepdims=True)
        # max_val nan veya inf ise sonuçlar da öyle olur
        if not np.all(np.isfinite(max_val)):
             self.logger.warning("Non-finite values detected in softmax input max. Output may be NaN/Inf.")

        exp_x = np.exp(x - max_val)
        sum_exp_x = np.sum(exp_x, axis=-1, keepdims=True)

        # Sıfıra bölme hatasını önle
        probabilities = np.divide(exp_x, sum_exp_x, out=np.full_like(exp_x, np.nan), where=sum_exp_x!=0)

        return probabilities

    @staticmethod
    def parse_logits_string(logits_str: str, class_labels: list) -> Optional[tuple]:
        """
        Embedded sistemden gelen ham logit string'ini parse et ve sınıf tahmini yap.
        SerialInterface'ten buraya taşındı: sınıflandırma işi ModelPredictor'a ait.

        Args:
            logits_str: "[6.18, -3.12, ...]" formatında string
            class_labels: Sınıf isimlerinin listesi

        Returns:
            (class_name, confidence_pct, probabilities) veya None
        """
        try:
            logits_str = logits_str.strip()
            if not (logits_str.startswith('[') and logits_str.endswith(']')):
                return None

            logits = [float(x.strip()) for x in logits_str[1:-1].split(',')]

            if len(logits) != len(class_labels):
                return None

            logits_np = np.array(logits, dtype=np.float32)
            exp_l = np.exp(logits_np - np.max(logits_np))
            probs = exp_l / np.sum(exp_l)

            predicted_idx = int(np.argmax(probs))
            return (class_labels[predicted_idx], float(probs[predicted_idx]) * 100.0, probs)

        except Exception:
            return None