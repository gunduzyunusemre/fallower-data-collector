# START OF FILE data_processor.py
from typing import Optional
import numpy as np
import threading
import queue
import time

from core.config import Configuration, PreprocessingPipeline
from core.logger import Logger
from core.event_bus import EventBus, Event
from core.enums import EventType, SystemStatus # EventType'ı import ettiğinizden emin olun
from data.models import Frame, FrameWindow

class DataProcessor:
    """Gelen Frame'leri işler ve tahmin için pencereler oluşturur."""
    def __init__(self):
        self.config = Configuration()
        self.logger = self.config.get_logger()
        self.event_bus = EventBus()

        # Yapılandırma
        proc_config = self.config.get('processing')
        self.window_size = proc_config.get('window_size')
        self.stride = proc_config.get('stride')
        self.expected_features = proc_config.get('frame_feature_count')
        self.target_length = self.expected_features  # Modelin beklediği hedef uzunluk
        
        # Normalization parameters
        self.normalize_target_min = proc_config.get('normalize_target_min', -1000)
        self.normalize_target_max = proc_config.get('normalize_target_max', 1000)

        # Durum
        self.frame_window = FrameWindow(self.window_size, self.stride)
        self._processing_enabled = False

        # Doğrudan SerialInterface ile iletişim için kuyruk
        self._frame_queue = queue.Queue(maxsize=1000)
        self._direct_mode = False
        self._processing_thread = None
        
        # Doğrudan ModelPredictor ile iletişim
        self._model_predictor = None
        self._direct_prediction_queue = None
        
        # pyfftw optimizasyonu için
        self._use_pyfftw = False
        try:
            import pyfftw
            pyfftw.interfaces.cache.enable()
            self._use_pyfftw = True
            self.logger.info("Using pyfftw for faster FFT operations")
        except ImportError:
            self.logger.info("pyfftw not available, using numpy.fft")

        # Olay abonelikleri
        self.event_bus.subscribe(EventType.FRAME_RECEIVED, self.handle_frame_received)
        self.event_bus.subscribe(EventType.STATUS_CHANGED, self.handle_status_change)

        self.logger.info(f"DataProcessor initialized (Window: {self.window_size}, Stride: {self.stride}, Features: {self.expected_features}).")
        
        # Log current preprocessing pipeline
        current_pipeline = self.config.get_preprocessing_pipeline()
        self.logger.info(f"Current preprocessing pipeline: {current_pipeline.value}")

    def setup_direct_mode(self, frame_queue=None):
        """SerialInterface ile doğrudan kuyruk modunu ayarlar"""
        if frame_queue:
            self._frame_queue = frame_queue
        self._direct_mode = True
        
        # Processing thread'i başlat
        self._processing_thread = threading.Thread(
            target=self._processing_loop, 
            name="DataProcessingThread"
        )
        self._processing_thread.daemon = True
        self._processing_thread.start()
        self.logger.info("Direct mode activated - processing thread started")
    
    def set_model_predictor(self, model_predictor, prediction_queue=None):
        """ModelPredictor ile doğrudan bağlantı"""
        self._model_predictor = model_predictor
        self._direct_prediction_queue = prediction_queue
        self.logger.info("Direct connection established with ModelPredictor")

    def handle_status_change(self, event: Event):
        if event.event_type == EventType.STATUS_CHANGED:
            new_status: SystemStatus = event.data
            if new_status == SystemStatus.STREAMING:
                if not self._processing_enabled:
                    self.logger.info("Streaming started, enabling frame processing.")
                    self._processing_enabled = True
                    self.frame_window.clear()  # Yeni akış/işleme başında pencereyi temizle
            else:  # Diğer durumlar (READY, ERROR, STOPPING, etc.)
                if self._processing_enabled:
                    self.logger.info(f"Status changed to {new_status.name}, disabling frame processing.")
                    self._processing_enabled = False
                    
    def handle_frame_received(self, event: Event):
        if not self._processing_enabled or self._direct_mode:
            return

        if event.event_type == EventType.FRAME_RECEIVED:
            frame: Frame = event.data
            try:
                # 1. Frame'i işle (raw -> numpy)
                processed_data = frame.process(self.expected_features)

                data_for_window = self._adjust_frame_size(processed_data, frame.frame_id, frame.timestamp)

                # 3. Frame'i pencereye ekle ve hazır olup olmadığını kontrol et
                if data_for_window is not None:
                    window_ready = self.frame_window.add_frame(data_for_window, frame.timestamp)
                else:
                    self.logger.error(f"Frame {frame.frame_id}: Processed data is None, cannot add to window.")
                    window_ready = False

                # 4. Pencere tahmin için hazırsa olayı yayınla
                if window_ready:
                    self.publish_window()

            except ValueError as e:
                self.logger.error(f"Error processing Frame {frame.frame_id}: {e}")
                self.event_bus.publish(Event(EventType.ERROR_OCCURRED, data={"error": str(e), "frame_id": frame.frame_id}, source="DataProcessor"))
            except Exception as e:
                self.logger.error(f"Unexpected error handling Frame {frame.frame_id}: {e}", exc_info=True)
                self.event_bus.publish(Event(EventType.ERROR_OCCURRED, data={"error": str(e), "frame_id": frame.frame_id}, source="DataProcessor"))

    def _processing_loop(self):
        """Doğrudan mod kuyruğundan frame'leri işler"""
        self.logger.info("Processing loop started.")
        while True:  # Sürekli çalışacak loop
            try:
                # Frame'i al (timeout ile)
                frame = self._frame_queue.get(timeout=0.5)
                
                # Stop sinyali kontrolü
                if frame is None:
                    break
                
                # Processing disabled ise frame'i atla
                if not self._processing_enabled:
                    self._frame_queue.task_done()
                    continue
                
                # Gelen frame'i işle
                try:
                    processed_data = frame.process(self.expected_features)
                    
                    # DEBUG: Frame ve timestamp bilgisini logla
                    # self.logger.info(f"DEBUG: Processing frame {frame.frame_id}, timestamp: {frame.timestamp}")
                    
                    # Timestamp'i geçir - bu çok kritik!
                    data_for_window = self._adjust_frame_size(
                        processed_data, 
                        frame.frame_id, 
                        frame.timestamp  # ← Bu çok önemli!
                    )
                    
                    if data_for_window is not None:
                        window_ready = self.frame_window.add_frame(data_for_window, frame.timestamp)
                        if window_ready:
                            self.publish_window()
                    else:
                        self.logger.warning(f"Frame {frame.frame_id}: data_for_window is None after processing")
                    
                    self._frame_queue.task_done()
                except Exception as e:
                    self.logger.error(f"Error processing frame {frame.frame_id}: {e}", exc_info=True)
                    self._frame_queue.task_done()
            
            except queue.Empty:
                # Timeout oldu, devam et
                continue
            except Exception as e:
                self.logger.error(f"Error in processing loop: {e}", exc_info=True)
                time.sleep(0.1)
        
        self.logger.info("Processing loop finished.")

    def _adjust_frame_size(self, processed_data: np.ndarray, frame_id: int, timestamp=None) -> Optional[np.ndarray]:
        """Daha az kopyalama ile frame boyutunu ayarla"""
        if processed_data is None:
            return None
        
        # Get current preprocessing pipeline
        current_pipeline = self.config.get_preprocessing_pipeline()

        # ADC 16-bit yayınla (kopyalamadan - değiştirilmeyecekse)
        if timestamp is not None:
            self.event_bus.publish(Event(
                EventType.ADC_BEFORE,
                data={"frame_id": frame_id, "timestamp": timestamp, "data": processed_data},
                source="DataProcessor"
            ))
        
        # 16-bit ADC → 12-bit dönüşümü 
        is_signed = np.any(processed_data < 0)
        
        # Yerinde (in-place) dönüşüm yapmak için kopya oluştur
        # (sonraki aşamalarda verinin değiştirilmesi gerektiği için)
        processed_data_copy = processed_data.copy()  
        
        if is_signed:
            scale_factor = 2048 / 32768
            processed_data_copy = np.floor(processed_data_copy * scale_factor).astype(np.int16)
        else:
            scale_factor = 4095 / 65535
            processed_data_copy = np.floor(processed_data_copy * scale_factor).astype(np.uint16)
        
        # # ADC dönüşümü sonrası debug
        # if frame_id % 50 == 0:
        #     adc_mean = np.mean(processed_data_copy)
        #     adc_hash = hash(processed_data_copy.tobytes())
        #     self.logger.info(f"  After ADC - Hash: {adc_hash}, Mean: {adc_mean:.4f}")
        
        # ADC 12-bit yayınla (kopyalamadan)
        if timestamp is not None:
            self.event_bus.publish(Event(
                EventType.ADC_AFTER,
                data={"frame_id": frame_id, "timestamp": timestamp, "data": processed_data_copy},
                source="DataProcessor"
            ))
        
        # Boyut uyumlu hale getirme
        current_length = len(processed_data_copy)
        target_length = self.target_length
        
        if current_length != target_length:
            if current_length > target_length:
                # Fazla elemanları kırp
                data_for_window = processed_data_copy[:target_length]
            else: # current_length < target_length
                # Eksik elemanları doldur (son elemanla) - bu durum artık olmamalı
                self.logger.warning(f"Frame {frame_id}: Unexpected frame size {current_length}, expected {target_length}")
                padding_needed = target_length - current_length
                if current_length > 0:
                    padding_value = processed_data_copy[-1]
                else:
                    padding_value = 0 
                padding = np.full(padding_needed, padding_value, dtype=processed_data_copy.dtype)
                data_for_window = np.concatenate((processed_data_copy, padding))
        else:
            data_for_window = processed_data_copy
        
        # # Resize sonrası debug
        # if frame_id % 50 == 0:
        #     resize_hash = hash(data_for_window.tobytes())
        #     resize_mean = np.mean(data_for_window)
        #     self.logger.info(f"  After Resize - Hash: {resize_hash}, Mean: {resize_mean:.4f}, Shape: {data_for_window.shape}")
        
        # Resized veriyi yayınla
        if timestamp is not None:
            self.event_bus.publish(Event(
                EventType.FRAME_RESIZED,
                data={"frame_id": frame_id, "timestamp": timestamp, "data": data_for_window},
                source="DataProcessor"
            ))
        
        # Float32 dönüşümü
        data_for_window_float = data_for_window.astype(np.float32)
        
        # # Float32 dönüşüm sonrası debug
        # if frame_id % 50 == 0:
        #     float_hash = hash(data_for_window_float.tobytes())
        #     float_mean = np.mean(data_for_window_float)
        #     self.logger.info(f"  After Float32 - Hash: {float_hash}, Mean: {float_mean:.4f}")
        
        # FRAME_PROCESSED olayını yayınla (Float32'ye dönüştürülmüş veri ile)
        if timestamp is not None:
            self.event_bus.publish(Event(
                EventType.FRAME_PROCESSED,
                data={"frame_id": frame_id, "timestamp": timestamp, "processed_data": data_for_window_float},
                source="DataProcessor"
            ))
        # ===========================================
        # PREPROCESSING PIPELINE SELECTION
        # ===========================================
        
        # Apply preprocessing based on current model pipeline
        final_data = self._apply_preprocessing_pipeline(
            data_for_window_float, 
            current_pipeline, 
            frame_id, 
            timestamp
        )
        
        # # Debug final data
        # if frame_id % 50 == 0:
        #     final_hash = hash(final_data.tobytes())
        #     final_mean = np.mean(final_data)
        #     final_std = np.std(final_data)
        #     self.logger.info(f"  Final Data - Hash: {final_hash}, Mean: {final_mean:.4f}, Std: {final_std:.4f}")
        #     self.logger.info(f"  Final [0:10]: {final_data[:10]}")
        #     self.logger.info("-" * 40)

        # MODEL_INPUT_DATA olayını burada yayınla (pre-bandpass verisi)
        # DataStorage için ham işlenmiş veri (filtrelemeden önce)
        if timestamp is not None:
            # self.logger.info(f"DEBUG: Publishing MODEL_INPUT_DATA event for frame {frame_id}")  # DEBUG LOG

            # self.logger.debug(f"Publishing MODEL_INPUT_DATA event for frame {frame_id} with {len(data_for_window_float)} features")
            self.event_bus.publish(Event(
                EventType.MODEL_INPUT_DATA,
                data={
                    "frame_number": frame_id, 
                    "processed_data": final_data # Bu, 1D numpy dizisidir (örneğin 688 özellik)
                },
                source="DataProcessor"
            ))
            # self.logger.info(f"DEBUG: MODEL_INPUT_DATA event published successfully for frame {frame_id}")  # DEBUG

        else:
            self.logger.warning(f"DEBUG: timestamp is None for frame {frame_id}, MODEL_INPUT_DATA not published")  # DEBUG LOG
        
        return final_data

    def _apply_preprocessing_pipeline(self, data: np.ndarray, pipeline: PreprocessingPipeline, 
                                    frame_id: int, timestamp=None) -> np.ndarray:
        """Apply preprocessing pipeline based on model requirements"""
        
        # Publish pre-processing data
        if timestamp is not None:
            self.event_bus.publish(Event(
                EventType.PRE_BANDPASS,
                data={"frame_id": frame_id, "timestamp": timestamp, "data": data},
                source="DataProcessor"
            ))
        
        if pipeline == PreprocessingPipeline.RAW:
            # No processing - return data as is
            final_data = data
            
        elif pipeline == PreprocessingPipeline.RAW_NORMALIZE:
            # Only normalization
            final_data = self._normalize_frame(data.reshape(1, -1))[0]
            
        elif pipeline == PreprocessingPipeline.BANDPASS:
            # Only bandpass filter
            try:
                single_frame_2d = data.reshape(1, -1)
                filtered_data = self.apply_bandpass_filter(single_frame_2d)
                final_data = filtered_data[0]
            except Exception as e:
                self.logger.error(f"Frame {frame_id}: Bandpass filter error: {e}", exc_info=True)
                final_data = data
                
        elif pipeline == PreprocessingPipeline.BANDPASS_NORMALIZE:
            # Bandpass filter + normalization
            try:
                single_frame_2d = data.reshape(1, -1)
                filtered_data = self.apply_bandpass_filter(single_frame_2d)
                normalized_data = self._normalize_frame(filtered_data)
                final_data = normalized_data[0]
            except Exception as e:
                self.logger.error(f"Frame {frame_id}: Bandpass+normalize error: {e}", exc_info=True)
                final_data = data
        else:
            self.logger.warning(f"Unknown preprocessing pipeline: {pipeline}, using raw data")
            final_data = data
        
        # Publish post-processing data
        if timestamp is not None:
            self.event_bus.publish(Event(
                EventType.POST_BANDPASS,
                data={"frame_id": frame_id, "timestamp": timestamp, "data": final_data},
                source="DataProcessor"
            ))
        
        return final_data

    def apply_bandpass_filter(self, signal_2d, nfft=None, fc=None, bw=None, fs=None):
        """Çalışan bandpass filter implementasyonu (predict-test.py'den)"""
        # Konfigürasyon değerleri
        nfft = nfft if nfft is not None else self.config.get('processing', 'filter_nfft', 1024)
        fc = fc if fc is not None else self.config.get('processing', 'filter_center_freq', 8e9)
        bw = bw if bw is not None else self.config.get('processing', 'filter_bandwidth', 1.9e9)
        fs = fs if fs is not None else self.config.get('processing', 'filter_sampling_rate', 10000)
        
        # predict-test.py'deki çalışan implementasyon
        freq_ind = np.linspace(-fs/2, fs/2, nfft)

        f_low = int(np.ceil((fc - bw/2) / (2048 * 1000)))
        f_high = int(np.ceil((fc + bw/2) / (2048 * 1000)))

        win_1 = ((freq_ind[512:1023] >= f_low) & (freq_ind[512:1023] <= f_high)).astype(float)
        win_2 = ((freq_ind[0:511] <= -f_low) & (freq_ind[0:511] >= -f_high)).astype(float)
        rectangular_window = np.concatenate([[0], win_2, win_1, [0]])

        # pyfftw kullanabiliyorsak
        if self._use_pyfftw:
            import pyfftw.interfaces.numpy_fft as fft
            
            fft_signal = fft.fft(signal_2d, n=nfft, axis=1)
            fft_signal_shifted = fft.fftshift(fft_signal, axes=1)
            filtered_freq = fft_signal_shifted * rectangular_window[None, :]
            filtered_time = fft.ifft(fft.ifftshift(filtered_freq, axes=1), n=nfft, axis=1)
        else:
            # Standart numpy FFT
            fft_signal = np.fft.fft(signal_2d, n=nfft, axis=1)
            fft_signal_shifted = np.fft.fftshift(fft_signal, axes=1)
            filtered_freq = fft_signal_shifted * rectangular_window[None, :]
            filtered_time = np.fft.ifft(np.fft.ifftshift(filtered_freq, axes=1), n=nfft, axis=1)

        return np.real(filtered_time[:, :signal_2d.shape[1]]).astype(np.float32)

    def _normalize_frame(self, signal_2d) -> np.ndarray:
        """
        Normalize signal per frame to target range
        signal_2d: shape (H, W) = (num_frames, num_bins)
        Scales each frame individually to [target_min, target_max]
        """
        signal_2d = signal_2d.astype(np.float32)
        normalized = np.zeros_like(signal_2d)

        for i, frame in enumerate(signal_2d):
            min_val = frame.min()
            max_val = frame.max()

            if max_val - min_val < 1e-6:  # Avoid divide by zero
                normalized[i] = frame  # Or set to zeros if you prefer
            else:
                normalized[i] = (frame - min_val) / (max_val - min_val)  # Scale to [0, 1]
                normalized[i] = normalized[i] * (self.normalize_target_max - self.normalize_target_min) + self.normalize_target_min  # Scale to target range

        return normalized
    
    def publish_window(self):
        """Hazır pencereyi modele gönderir - doğrudan veya EventBus üzerinden"""
        window_data = self.frame_window.get_window_data()
        time_range = self.frame_window.get_time_range()
        
        if window_data is not None and time_range is not None:
            # Modelin beklediği özellik sayısını kontrol et (target_length)
            if window_data.shape[1] != self.target_length:
                self.logger.error(f"Window shape error: {window_data.shape}, expected (n, {self.target_length})")
                return
            
            # ===========================================
            #  DEBUG: WINDOW DATA ANALYSIS
            # ===========================================
            
            # window_mean = np.mean(window_data)
            # window_std = np.std(window_data)
            # window_hash = hash(window_data.tobytes())
            
            # # Check inter-frame variation
            # frame_means = np.mean(window_data, axis=1)
            # frame_variance = np.var(frame_means)
            
            # # Compare first and last frame
            # first_frame = window_data[0]
            # last_frame = window_data[-1]
            # frame_diff = np.sum(np.abs(first_frame - last_frame))
            
            # # Debugging (every 10 windows)
            # total_windows = getattr(self, '_debug_window_count', 0) + 1
            # self._debug_window_count = total_windows
            
            # if total_windows % 10 == 0:
            #     current_pipeline = self.config.get_preprocessing_pipeline()
            #     self.logger.info(" WINDOW DEBUG:")
            #     self.logger.info(f"  Window #{total_windows} - Pipeline: {current_pipeline.value}")
            #     self.logger.info(f"  Window Shape: {window_data.shape}")
            #     self.logger.info(f"  Window Hash: {window_hash}")
            #     self.logger.info(f"  Window Mean: {window_mean:.4f}, Std: {window_std:.4f}")
            #     self.logger.info(f"  Frame Variance: {frame_variance:.6f}")
            #     self.logger.info(f"  First vs Last Frame Diff: {frame_diff:.4f}")
            #     self.logger.info(f"  Frame Means: {frame_means[:5]}...{frame_means[-5:]}")
                
            #     # Time range check
            #     if time_range:
            #         duration = (time_range[1] - time_range[0]).total_seconds()  
            #         self.logger.info(f"  Time Range: {time_range[0].strftime('%H:%M:%S.%f')[:-3]} - {time_range[1].strftime('%H:%M:%S.%f')[:-3]}")
            #         self.logger.info(f"  Duration: {duration:.3f} seconds")
                
            #     # First frame's first 10 values
            #     self.logger.info(f"  First Frame [0:10]: {first_frame[:10]}")
            #     self.logger.info(f"  Last Frame [0:10]: {last_frame[:10]}")
            #     self.logger.info("-" * 50)
            
            # Doğrudan ModelPredictor'a gönder
            if self._model_predictor and self._direct_prediction_queue:
                try:
                    self._direct_prediction_queue.put_nowait({
                        "window_data": window_data,
                        "time_range": time_range
                    })
                    # self.logger.debug(f"Window data sent directly to ModelPredictor. Shape: {window_data.shape}")
                    return # Başarıyla gönderildi, EventBus'a düşme
                except queue.Full:
                    self.logger.warning("Direct prediction queue full, falling back to EventBus for window data.")
            
            # EventBus üzerinden gönder (doğrudan gönderim başarısızsa veya konfigüre edilmemişse)
            self.event_bus.publish(Event(
                EventType.WINDOW_READY_FOR_PREDICTION,
                data={"window_data": window_data, "time_range": time_range},
                source="DataProcessor"
            ))
            
            self.logger.debug(f"Window data published via EventBus for prediction. Shape: {window_data.shape}")
        else:
            self.logger.warning("Attempted to publish window, but data or time range was None. Window full status: %s", 
                                self.frame_window.is_full() if self.frame_window else "N/A")

    def update_model_configuration(self):
        """Update preprocessing pipeline when model changes"""
        current_pipeline = self.config.get_preprocessing_pipeline()
        current_model = self.config.get_current_model()
        
        self.logger.info(f"DataProcessor updated for model: {current_model.name}")
        self.logger.info(f"Preprocessing pipeline: {current_pipeline.value}")

    def stop(self):
        """DataProcessor'ı durdurur (temizlik veya state sıfırlama)."""
        self.logger.info("Stopping DataProcessor.")
        self._processing_enabled = False # Bu, handle_frame_received ve _processing_loop'u etkiler.
        
        # Doğrudan mod açıksa, processing thread'i durdurmak için kuyruğa None gönder
        if self._direct_mode and self._processing_thread and self._processing_thread.is_alive():
            try:
                # put_nowait yerine timeout'lu put: queue dolu olsa bile sentinel ulaşır
                # (_processing_enabled=False olduğu için thread frame'leri hemen atlıyor,
                #  queue kısa sürede yer açar)
                self._frame_queue.put(None, timeout=3.0)
            except queue.Full:
                self.logger.warning("Could not send stop signal to DataProcessor's frame_queue (still full after 3s).")

            self._processing_thread.join(timeout=2.0)
            if self._processing_thread.is_alive():
                self.logger.warning("Processing thread did not exit gracefully")
        self._processing_thread = None  # Thread referansını temizle