import os
import csv
import queue
import threading
import time
import numpy as np
from datetime import datetime
from typing import Optional, Any

# Gerekli core ve data modüllerini import et
from core.config import Configuration
from core.logger import Logger
from core.event_bus import EventBus, Event
from core.enums import EventType


class DataStorage:
    """İşlenmiş veri ve metadata kaydını destekleyen DataStorage sınıfı"""
    
    def __init__(self):
        self.config = Configuration()
        self.logger = self.config.get_logger()
        self.event_bus = EventBus()

        # Yapılandırma
        self.output_dir = self.config.get('system', 'output_dir')
        self.save_processed_data = self.config.get('processing', 'save_processed_data', True)
        self.save_predictions = self.config.get('processing', 'save_predictions', True)
        
        # Dosya yolları
        self.metadata_file_path: Optional[str] = None
        self._metadata_file_handle: Optional[Any] = None
        self.frame_data_file_path: Optional[str] = None
        self._frame_data_file_handle: Optional[Any] = None
        self.predictions_file_path: Optional[str] = None
        self._predictions_file_handle: Optional[Any] = None
        
        # Asenkron kayıt için
        self._storage_queue = queue.Queue(maxsize=1000)
        self._storage_thread: Optional[threading.Thread] = None
        self._running = False
        
        # Batch kayıt ayarları
        self._batch_size = 10
        self._batch_time_limit = 0.5
        self._metadata_batch = []
        self._frame_data_batch = []
        self._predictions_batch = []
        self._metadata_count = 0
        self._frame_data_count = 0
        self._predictions_count = 0
        self._last_write_time = 0
        
        # Gerçek zamanlı mod - varsayılan olarak açık
        self.real_time_mode = True

        # Olay abonelikleri — DataStorage disk I/O yaptığı için low-priority subscriber
        self.event_bus.subscribe(EventType.FRAME_METADATA, self.handle_frame_metadata, high_priority=False)
        self.event_bus.subscribe(EventType.MODEL_INPUT_DATA, self.handle_model_input_data, high_priority=False)
        self.event_bus.subscribe(EventType.PREDICTION_READY, self.handle_prediction_ready, high_priority=False)

        self.logger.info("DataStorage initialized.")

    def start(self) -> bool:
        """DataStorage modülünü başlatır ve dosyaları açar."""
        if self._running:
            self.logger.warning("DataStorage is already running.")
            return True

        try:
            os.makedirs(self.output_dir, exist_ok=True)
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

            # METADATA DOSYASI
            self.metadata_file_path = os.path.join(self.output_dir, f"frame_metadata_{timestamp}.csv")
            self.logger.info(f"Opening metadata file: {self.metadata_file_path}")
            self._metadata_file_handle = open(self.metadata_file_path, 'w', newline='', encoding='utf-8')
            metadata_writer = csv.writer(self._metadata_file_handle)
            
            # Metadata header
            metadata_header = [
                'timestamp', 
                'frame_number', 
                'embedded_timestamp', 
                'duration', 
                'sample_count', 
                'crc', 
                'crc_valid'
            ]
            metadata_writer.writerow(metadata_header)
            self._metadata_file_handle.flush()
            
            # FRAME DATA DOSYASI (işlenmiş veri)
            if self.save_processed_data:
                self.frame_data_file_path = os.path.join(self.output_dir, f"frame_data_{timestamp}.csv")
                self.logger.info(f"Opening frame data file: {self.frame_data_file_path}")
                self._frame_data_file_handle = open(self.frame_data_file_path, 'w', newline='', encoding='utf-8')
                frame_data_writer = csv.writer(self._frame_data_file_handle)
                
                # Frame data header (timestamp, frame_number, ve feature_0 ila feature_N)
                frame_data_header = ['timestamp', 'frame_number']
                # 688 özellik için kolonlar ekle
                expected_features = self.config.get('processing', 'frame_feature_count', 688)
                frame_data_header.extend([f'feature_{i}' for i in range(expected_features)])
                frame_data_writer.writerow(frame_data_header)
                self._frame_data_file_handle.flush()
                self.logger.info(f"Frame data file initialized with {expected_features + 2} columns")
            
            # PREDICTIONS DOSYASI (tahmin sonuçları)
            if self.save_predictions:
                self.predictions_file_path = os.path.join(self.output_dir, f"predictions_{timestamp}.csv")
                self.logger.info(f"Opening predictions file: {self.predictions_file_path}")
                self._predictions_file_handle = open(self.predictions_file_path, 'w', newline='', encoding='utf-8')
                predictions_writer = csv.writer(self._predictions_file_handle)
                
                # Predictions header
                predictions_header = [
                    'prediction_timestamp', 
                    'window_start_time', 
                    'window_end_time',
                    'predicted_label',
                    'confidence',
                    'confidence_percent'
                ]
                predictions_writer.writerow(predictions_header)
                self._predictions_file_handle.flush()
                self.logger.info("Predictions file initialized")
            
            # Batch yönetimi başlat
            self._metadata_batch = []
            self._frame_data_batch = []
            self._predictions_batch = []
            self._metadata_count = 0
            self._frame_data_count = 0
            self._predictions_count = 0
            self._last_write_time = time.time()

            # Storage thread'i başlat
            self._running = True
            self._storage_thread = threading.Thread(target=self._storage_loop, name="StorageThread")
            self._storage_thread.daemon = True
            self._storage_thread.start()
            
            self.logger.info(f"DataStorage started with real_time_mode={self.real_time_mode}")
            return True

        except Exception as e:
            self.logger.error(f"Failed to start DataStorage: {e}", exc_info=True)
            self.close_files()
            return False

    def stop(self):
        """DataStorage modülünü durdurur ve dosyaları kapatır."""
        if not self._running:
            return

        self.logger.info("Stopping DataStorage...")
        self._running = False

        if self._storage_thread and self._storage_thread.is_alive():
            try:
                self._storage_queue.put(None, timeout=1.0)
            except queue.Full:
                self.logger.warning("Storage queue is full, cannot add stop signal.")
            
            self._storage_thread.join(timeout=3.0)
            if self._storage_thread.is_alive():
                self.logger.warning("Storage thread did not exit gracefully.")

        self.close_files()
        self._storage_thread = None
        self.logger.info("DataStorage stopped.")

    def handle_frame_metadata(self, event: Event):
        """Frame meta verilerini işler."""
        if not self._running:
            return
        if event.event_type == EventType.FRAME_METADATA:
            try:
                # Metadata olaylarının tipi "metadata" olarak işaretle
                event_data = event.data.copy()
                event_data['_type'] = 'metadata'
                self._storage_queue.put_nowait(event_data)
                
                # Debug amaçlı logla (çok fazla log oluşturmamak için)
                frame_num = event.data.get('frame_number', -1)
                if frame_num % 100 == 0:  # Her 100 frame'de bir logla
                    self.logger.debug(f"Queued metadata for frame {frame_num}")
            except queue.Full:
                self.logger.warning("Storage queue full! Discarding metadata.")

    def handle_model_input_data(self, event: Event):
        """Model giriş verilerini (işlenmiş frame) işler."""
        # DEBUG: Her zaman bu metoda girdiğinde logla
        frame_num = event.data.get('frame_number', -1) if event.data else -1
        # self.logger.info(f"DEBUG: handle_model_input_data called for frame {frame_num}")
        # self.logger.info(f"DEBUG: _running={self._running}, save_processed_data={self.save_processed_data}")
        
        if not self._running or not self.save_processed_data:
            self.logger.warning(f"DEBUG: MODEL_INPUT_DATA event IGNORED for frame {frame_num} - running={self._running}, save_processed_data={self.save_processed_data}")
            return
        
        if event.event_type == EventType.MODEL_INPUT_DATA:
            try:
                # Frame data olaylarının tipi "frame_data" olarak işaretle
                event_data = event.data.copy()
                event_data['_type'] = 'frame_data'
                self._storage_queue.put_nowait(event_data)
                
                # self.logger.info(f"DEBUG: Successfully queued frame data for frame {frame_num}")
                
                # Debug amaçlı logla (çok fazla log oluşturmamak için)
                # if frame_num % 10 == 0:  # Her 10 frame'de bir logla
                #     self.logger.info(f"DEBUG: Queued frame data for frame {frame_num} (every 10th frame log)")
            except queue.Full:
                self.logger.error(f"DEBUG: Storage queue FULL! Discarding frame data for frame {frame_num}")
        else:
            self.logger.warning(f"DEBUG: Wrong event type received: {event.event_type}")

    def handle_prediction_ready(self, event: Event):
        """Tahmin sonuçlarını işler."""
        if not self._running or not self.save_predictions:
            return
        
        if event.event_type == EventType.PREDICTION_READY:
            try:
                # Prediction olaylarının tipi "prediction" olarak işaretle
                prediction_data = {
                    '_type': 'prediction',
                    'prediction_result': event.data
                }
                self._storage_queue.put_nowait(prediction_data)
                
                # Debug log
                # self.logger.debug(f"Queued prediction: {event.data.label} ({event.data.confidence:.2f})")
            except queue.Full:
                self.logger.warning("Storage queue full! Discarding prediction.")

    def _storage_loop(self):
        """Kuyruktan verileri alıp dosyaya yazan döngü."""
        self.logger.info("Storage loop started.")
        items_processed = 0
        
        while self._running or not self._storage_queue.empty():
            try:
                # Timeout ile item al
                item = self._storage_queue.get(timeout=0.2)
                if item is None:  # Stop signal
                    break

                # İtem tipine göre işlem yap
                item_type = item.pop('_type', 'unknown')
                if item_type == 'metadata':
                    self._write_metadata(item)
                elif item_type == 'frame_data':
                    self._write_frame_data(item)
                elif item_type == 'prediction':
                    self._write_prediction(item['prediction_result'])
                else:
                    self.logger.warning(f"Unknown storage item type: {item_type}")
                
                self._storage_queue.task_done()
                items_processed += 1

                # Her 500 item'de bir bilgi ver
                if items_processed % 500 == 0:
                    self.logger.info(f"Storage processed {items_processed} items, queue size: {self._storage_queue.qsize()}")

            except queue.Empty:
                if not self._running:
                    break
                
                # Bekleyen batch'leri düzenli flush et
                current_time = time.time()
                if current_time - self._last_write_time >= self._batch_time_limit:
                    self._flush_batches()
                
                continue
                
            except Exception as e:
                self.logger.error(f"Error in storage loop: {e}", exc_info=True)
                time.sleep(0.1)

        # Çıkmadan önce batch'leri flush et
        self._flush_batches()
        
        self.logger.info(f"Storage loop finished. Processed {items_processed} items.")

    def _write_metadata(self, data):
        """Metadata'yı batch'e ekle"""
        try:
            current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
            
            # Metadata satırını oluştur
            metadata_row = [
                current_time,
                data.get('frame_number', -1),
                data.get('timestamp', 0),
                data.get('duration', 0),
                data.get('sample_count', 0),
                data.get('crc', 0),
                1 if data.get('crc_valid', False) else 0
            ]
            
            # Batch'e ekle
            self._metadata_batch.append(metadata_row)
            self._metadata_count += 1
            
            # Batch boyutu kontrolü
            if self._metadata_count >= self._batch_size:
                self._flush_metadata_batch()
                
        except Exception as e:
            self.logger.error(f"Error writing metadata: {e}", exc_info=True)

    def _write_frame_data(self, data):
        """İşlenmiş frame verisini batch'e ekle"""
        if not self._frame_data_file_handle or not self.save_processed_data:
            self.logger.debug("Not writing frame data: file handle missing or save_processed_data is False")
            return
            
        try:
            current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
            frame_number = data.get('frame_number', -1)
            processed_data = data.get('processed_data', None)
            
            if processed_data is None:
                self.logger.warning(f"Missing processed_data for frame {frame_number}")
                return
                
            # Log processed_data yapısını
            # self.logger.debug(f"Frame {frame_number} processed_data type: {type(processed_data)}, shape/len: {processed_data.shape if hasattr(processed_data, 'shape') else len(processed_data)}")
                
            # Veriyi düzleştir
            if isinstance(processed_data, np.ndarray):
                # Numpy dizisi ise liste dönüşümü yap
                processed_data = processed_data.tolist()
            
            # Frame data satırını oluştur
            frame_data_row = [current_time, frame_number]
            frame_data_row.extend(processed_data)
            
            # Batch'e ekle
            self._frame_data_batch.append(frame_data_row)
            self._frame_data_count += 1
            
            # Her 10 frame'de bir log
            # if frame_number % 10 == 0:
            #     self.logger.info(f"Added frame {frame_number} to batch, current batch size: {self._frame_data_count}")
            
            # Batch boyutu kontrolü
            if self._frame_data_count >= self._batch_size:
                self._flush_frame_data_batch()
                
        except Exception as e:
            self.logger.error(f"Error writing frame data: {e}", exc_info=True)

    def _write_prediction(self, prediction_result):
        """Tahmin sonucunu batch'e ekle"""
        if not self._predictions_file_handle or not self.save_predictions:
            return
            
        try:
            current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
            
            # Prediction satırını oluştur
            prediction_row = [
                current_time,
                prediction_result.start_time.strftime('%Y-%m-%d %H:%M:%S.%f')[:-3],
                prediction_result.end_time.strftime('%Y-%m-%d %H:%M:%S.%f')[:-3],
                prediction_result.label,
                f"{prediction_result.confidence:.4f}",
                f"{prediction_result.confidence * 100.0:.2f}%"
            ]
            
            # Batch'e ekle
            self._predictions_batch.append(prediction_row)
            self._predictions_count += 1
            
            # Batch boyutu kontrolü
            if self._predictions_count >= self._batch_size:
                self._flush_predictions_batch()
                
        except Exception as e:
            self.logger.error(f"Error writing prediction: {e}", exc_info=True)

    def _flush_batches(self):
        """Tüm bekleyen batch'leri temizle"""
        if self._metadata_count > 0:
            self._flush_metadata_batch()
        if self._frame_data_count > 0 and self.save_processed_data:
            self._flush_frame_data_batch()
        if self._predictions_count > 0 and self.save_predictions:
            self._flush_predictions_batch()
        
        # Son yazma zamanını güncelle
        self._last_write_time = time.time()

    def _flush_metadata_batch(self):
        """Metadata batch'ini diske yaz"""
        try:
            if (self._metadata_file_handle and 
                not self._metadata_file_handle.closed and 
                self._metadata_batch):
                
                csv.writer(self._metadata_file_handle).writerows(self._metadata_batch)
                self._metadata_file_handle.flush()
                
                # Batch'i temizle
                written_count = len(self._metadata_batch)
                self._metadata_batch = []
                self._metadata_count = 0
                
                # Debug log
                # if written_count > 0 and written_count % 100 == 0:
                #     self.logger.debug(f"Flushed {written_count} metadata records to disk")
                    
        except Exception as e:
            self.logger.error(f"Error flushing metadata batch: {e}")

    def _flush_frame_data_batch(self):
        """Frame data batch'ini diske yaz"""
        try:
            if (self._frame_data_file_handle and 
                not self._frame_data_file_handle.closed and 
                self._frame_data_batch):
                
                rows_to_write = len(self._frame_data_batch)
                # self.logger.debug(f"Flushing {rows_to_write} frame data records to disk")
                
                csv.writer(self._frame_data_file_handle).writerows(self._frame_data_batch)
                self._frame_data_file_handle.flush()
                
                # Batch'i temizle
                written_count = len(self._frame_data_batch)
                self._frame_data_batch = []
                self._frame_data_count = 0
                
                # Log
                # self.logger.info(f"Flushed {written_count} frame data records to disk")
                
        except Exception as e:
            self.logger.error(f"Error flushing frame data batch: {e}", exc_info=True)

    def _flush_predictions_batch(self):
        """Predictions batch'ini diske yaz"""
        try:
            if (self._predictions_file_handle and 
                not self._predictions_file_handle.closed and 
                self._predictions_batch):
                
                csv.writer(self._predictions_file_handle).writerows(self._predictions_batch)
                self._predictions_file_handle.flush()
                
                # Batch'i temizle
                written_count = len(self._predictions_batch)
                self._predictions_batch = []
                self._predictions_count = 0
                
                # Log
                # self.logger.info(f"Flushed {written_count} prediction records to disk")
                
        except Exception as e:
            self.logger.error(f"Error flushing predictions batch: {e}")

    def close_files(self):
        """Tüm dosyaları güvenli bir şekilde kapatır."""
        self._flush_batches()
        
        try:
            if self._metadata_file_handle and not self._metadata_file_handle.closed:
                self._metadata_file_handle.flush()
                self._metadata_file_handle.close()
                self.logger.info(f"Closed metadata file: {self.metadata_file_path}")
                
            if self._frame_data_file_handle and not self._frame_data_file_handle.closed:
                self._frame_data_file_handle.flush()
                self._frame_data_file_handle.close()
                self.logger.info(f"Closed frame data file: {self.frame_data_file_path}")
                
            if self._predictions_file_handle and not self._predictions_file_handle.closed:
                self._predictions_file_handle.flush()
                self._predictions_file_handle.close()
                self.logger.info(f"Closed predictions file: {self.predictions_file_path}")
                
        except Exception as e:
            self.logger.error(f"Error closing files: {e}", exc_info=True)
        finally:
            self._metadata_file_handle = None
            self._frame_data_file_handle = None
            self._predictions_file_handle = None

    def set_real_time_mode(self, enabled=True):
        """Gerçek zamanlı mod ayarını değiştirir (opsiyonel dosya kayıt optimizasyonları)"""
        self.real_time_mode = enabled
        if enabled:
            # Gerçek zamanlı modda daha sık flush et
            self._batch_time_limit = 0.2
            self._batch_size = 5
            self.logger.info("Real-time mode enabled - using smaller batches with more frequent flushes")
        else:
            # Tam analiz modunda daha büyük batch'ler kullan
            self._batch_time_limit = 1.0
            self._batch_size = 50
            self.logger.info("Full analysis mode - using larger batches for better throughput")