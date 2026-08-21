import threading
import time
import logging
import serial
import onnxruntime as ort
import queue
from onnxruntime.capi import onnxruntime_pybind11_state as ort_capi
from typing import Optional, Dict

from core.config import Configuration
from core.logger import Logger
from core.event_bus import EventBus, Event
from core.enums import EventType, SystemStatus
from interfaces.serial_interface import SerialInterface
from processing.data_processor import DataProcessor
from processing.model_predictor import ModelPredictor
from storage.data_storage import DataStorage


class FallowerSystem:
    """Tüm Fallower sistemini yönetir."""
    def __init__(self):
        self.config = Configuration()
        self.logger = self.config.get_logger()
        self.event_bus = EventBus()

        # Sistem Durumu
        self._status = SystemStatus.IDLE
        self._status_lock = threading.Lock()

        # Modüller (başlangıçta None)
        self.serial_interface: Optional[SerialInterface] = None
        self.data_processor: Optional[DataProcessor] = None
        self.model_predictor: Optional[ModelPredictor] = None
        self.data_storage: Optional[DataStorage] = None

        # Olay abonelikleri
        self.event_bus.subscribe(EventType.STATUS_CHANGED, self._handle_status_change)
        self.event_bus.subscribe(EventType.ERROR_OCCURRED, self._handle_error)

    @property
    def status(self) -> SystemStatus:
        with self._status_lock:
            return self._status

    def _update_status(self, new_status: SystemStatus, source: str = "System"):
        with self._status_lock:
            if self._status == new_status:
                return
            old_status = self._status
            # SHUTDOWN durumundan hiçbir yere çıkış yapma
            if old_status == SystemStatus.SHUTDOWN:
                self.logger.warning(f"Attempted to change status from {old_status.name} to {new_status.name}. Ignored.")
                return
            # ERROR durumundan sadece STOPPING ve SHUTDOWN'a geçişe izin ver
            if old_status == SystemStatus.ERROR and new_status not in [SystemStatus.ERROR, SystemStatus.STOPPING, SystemStatus.SHUTDOWN]:
                self.logger.warning(f"Attempted to change status from {old_status.name} to {new_status.name}. Ignored.")
                return

            self._status = new_status
            self.logger.info(f"System status changed: {old_status.name} -> {new_status.name} (Source: {source})")
            # Olayı yayınla
            self.event_bus.publish(Event(EventType.STATUS_CHANGED, data=new_status, source=source))

    def _require_status(self, *allowed: SystemStatus, command: str = "Command") -> bool:
        """
        Mevcut sistem durumunun izin verilen durumlardan biri olup olmadığını kontrol eder.
        Değilse uyarı log'u basar ve False döndürür.
        """
        current = self.status
        if current not in allowed:
            allowed_names = ", ".join(s.name for s in allowed)
            self.logger.warning(
                f"{command} için geçersiz durum: {current.name}. "
                f"Beklenen: {allowed_names}. Komut gönderilemedi."
            )
            return False
        return True

    def _handle_status_change(self, event: Event):
        """Diğer modüllerden gelen durum değişikliklerine göre sistem durumunu ayarlar."""
        new_module_status = event.data
        source_module = event.source

        if source_module == "SerialInterface":
            # SerialInterface modülünden gelen durum değişikliklerini doğrudan uygula
            self._update_status(new_module_status, source=f"System ({source_module})")
        # Diğer modüllerin durum değişiklikleri burada ele alınabilir

    def _handle_error(self, event: Event):
        error_info = event.data or {}
        source = event.source or "Unknown"
        error_msg = error_info.get('error', 'Unknown error details')
        self.logger.error(f"Error reported from {source}: {error_msg}")

        # Kritik hatalarda sistemi ERROR durumuna al
        critical_sources = ["ModelLoader", "SerialReceiver", "SerialInterface", "System"]
        if source in critical_sources or isinstance(error_msg, (FileNotFoundError, serial.SerialException, ort_capi.RuntimeException)):
            if self.status != SystemStatus.ERROR:
                self.logger.critical(f"Critical error detected from {source}. Setting system status to ERROR.")
                self._update_status(SystemStatus.ERROR, source=f"ErrorHandler ({source})")

    def initialize(self) -> bool:
        if self.status != SystemStatus.IDLE:
            self.logger.warning(f"System initialization requested but status is {self.status.name}. Ignoring.")
            return self.status not in [SystemStatus.ERROR, SystemStatus.SHUTDOWN]

        self.logger.info("Initializing Fallower System with Multi-Model Support...")
        self._update_status(SystemStatus.INITIALIZING)

        initialization_successful = False
        try:
            # 1. EventBus'ı başlat
            self.event_bus.start()

            # 2. Modülleri oluştur
            self.data_storage = DataStorage()
            self.serial_interface = SerialInterface()
            self.data_processor = DataProcessor()
            self.model_predictor = ModelPredictor()

            # 3. Modülleri başlat (başlatma sırası önemli olabilir)
            if not self.data_storage.start():
                self.logger.warning("DataStorage failed to start, but initialization continues.")

            # Modelin yüklenmesi kritik, başarısız olursa devam etme
            if not self.model_predictor.start():
                self.logger.error("ModelPredictor failed to start. Initialization aborted.")
                raise RuntimeError("ModelPredictor failed to start.")

            # Seri arayüzün başlaması kritik
            if not self.serial_interface.start():
                self.logger.error("SerialInterface failed to start. Initialization aborted.")
                raise RuntimeError("SerialInterface failed to start.")
                
            # 4. Modüller arası doğrudan bağlantıları kur
            # Modüller oluşturulunca diğer modüller için referansları ayarla
            if self.data_processor and self.model_predictor:
                # ModelPredictor referansını DataProcessor'a ver
                prediction_queue = self.model_predictor.setup_direct_mode()
                self.data_processor.set_model_predictor(
                    self.model_predictor, prediction_queue)
            
            if self.serial_interface and self.data_processor:
                # DataProcessor referansını SerialInterface'e ver
                frame_queue = queue.Queue(maxsize=1000)
                self.serial_interface.set_direct_mode(
                    self.data_processor, frame_queue)
                self.data_processor.setup_direct_mode(frame_queue)
            
            # Gerçek zamanlı test modu
            if self.data_storage:
                # Varsayılan olarak gerçek zamanlı mod
                self.data_storage.set_real_time_mode(True)

            # Thread önceliklerini ayarla
            self._set_thread_priorities()

            # Her şey yolunda gittiyse
            self._update_status(SystemStatus.READY)
            
            # Log current model information
            current_model = self.config.get_current_model()
            self.logger.info("=" * 50)
            self.logger.info("SYSTEM READY - Multi-Model Fall Detection")
            self.logger.info(f"Active Model: {current_model.name}")
            self.logger.info(f"Pipeline: {current_model.pipeline.value}")
            self.logger.info(f"Model File: {current_model.filename}")
            self.logger.info("=" * 50)
            
            self.event_bus.publish(Event(EventType.SYSTEM_START, source="System"))
            initialization_successful = True
            return True

        except Exception as e:
            self.logger.critical(f"Critical error during system initialization: {e}", exc_info=True)
            self._update_status(SystemStatus.ERROR, source="Initialization")
            # Hata durumunda başlatılanları durdurmaya çalış
            self.shutdown(graceful=False)
            return False
        finally:
            # Eğer başarılı olmadıysa ve durum hala INITIALIZING ise ERROR yap
            if not initialization_successful and self.status == SystemStatus.INITIALIZING:
                self._update_status(SystemStatus.ERROR, source="Initialization Fallback")
                
    def _set_thread_priorities(self):
        """Thread önceliklerini ayarla"""
        try:
            import os
            import psutil
            
            process = psutil.Process(os.getpid())
            
            # İşlemci önceliğini artır
            if os.name == 'nt':  # Windows
                process.nice(psutil.HIGH_PRIORITY_CLASS)
            else:  # Linux
                process.nice(-10)  # -20 (en yüksek) ile 19 (en düşük) arasında
                
            self.logger.info("Process priority increased for better real-time performance")
        except (ImportError, PermissionError) as e:
            self.logger.warning(f"Could not set thread priorities: {e}")

    # ========== MODEL MANAGEMENT ==========
    
    def get_available_models(self) -> Dict[str, str]:
        """Get list of available AI models"""
        if not self.model_predictor:
            self.logger.error("ModelPredictor not initialized")
            return {}
        
        return self.model_predictor.get_available_models()

    def get_current_model_info(self) -> Dict:
        """Get current model information"""
        if not self.model_predictor:
            self.logger.error("ModelPredictor not initialized")
            return {}
        
        return self.model_predictor.get_current_model_info()

    def change_model(self, model_key: str) -> bool:
        """
        Change AI model during runtime
        
        Args:
            model_key: Key of the model to switch to (e.g., 'raw', 'bandpass', etc.)
        
        Returns:
            bool: Success status
        """
        if not self.model_predictor:
            self.logger.error("Cannot change model: ModelPredictor not initialized")
            return False

        if not self.data_processor:
            self.logger.error("Cannot change model: DataProcessor not initialized")
            return False

        self.logger.info(f"Changing model to: {model_key}")
        
        # Check if streaming is active
        was_streaming = False
        if self.serial_interface and self.serial_interface.is_streaming():
            was_streaming = True
            self.logger.info("Stopping streaming before model change...")
            self.stop_streaming()
            time.sleep(0.5)  # Wait for stream to stop

        try:
            # Change model in ModelPredictor
            if not self.model_predictor.change_model(model_key):
                self.logger.error(f"Failed to change model to: {model_key}")
                return False

            # Update DataProcessor configuration for new preprocessing pipeline
            self.data_processor.update_model_configuration()
            
            # Log successful change
            current_model = self.config.get_current_model()
            self.logger.info("=" * 50)
            self.logger.info("MODEL CHANGED SUCCESSFULLY")
            self.logger.info(f"New Model: {current_model.name}")
            self.logger.info(f"Pipeline: {current_model.pipeline.value}")
            self.logger.info(f"File: {current_model.filename}")
            self.logger.info("=" * 50)

            # Resume streaming if it was active
            if was_streaming:
                self.logger.info("Resuming streaming with new model...")
                time.sleep(0.5)  # Brief pause
                self.start_streaming()

            return True

        except Exception as e:
            self.logger.error(f"Error during model change: {e}", exc_info=True)
            return False

    def reload_current_model(self) -> bool:
        """Reload the current model (useful for debugging)"""
        if not self.model_predictor:
            self.logger.error("Cannot reload model: ModelPredictor not initialized")
            return False

        current_model_key = self.config.current_model_key
        self.logger.info(f"Reloading current model: {current_model_key}")
        
        return self.change_model(current_model_key)

    # Embedded sistem komut akışı metodları
    def send_init_command(self) -> bool:
        """
        Embedded sisteme INIT komutu gönderir.
        Sistem READY, CALIBRATING veya STREAMING durumunda olmalıdır.

        Returns:
            bool: Komutun başarıyla gönderilip gönderilmediği
        """
        if not self.serial_interface:
            self.logger.error("Cannot send INIT command: SerialInterface not initialized.")
            return False
        if not self._require_status(
            SystemStatus.READY, SystemStatus.CALIBRATING, SystemStatus.STREAMING,
            command="INIT"
        ):
            return False

        self.logger.info("Sending INIT command to initialize embedded system...")
        return self.serial_interface.send_init_command()

    def send_ready_command(self) -> bool:
        """
        Embedded sisteme READY komutu gönderir.
        Sistem READY veya CALIBRATING durumunda olmalıdır.

        Returns:
            bool: Komutun başarıyla gönderilip gönderilmediği
        """
        if not self.serial_interface:
            self.logger.error("Cannot send READY command: SerialInterface not initialized.")
            return False
        if not self._require_status(
            SystemStatus.INITIALIZING, SystemStatus.READY, SystemStatus.CALIBRATING,
            command="READY"
        ):
            return False

        self.logger.info("Sending READY command to prepare embedded system...")
        return self.serial_interface.send_ready_command()

    def send_register_configuration(self, register_values: Dict[int, int]) -> bool:
        """
        Radar register değerlerini embedded sisteme gönderir.
        Sistem READY veya CALIBRATING durumunda olmalıdır.

        Args:
            register_values: {register_numarası: değer} formatında sözlük

        Returns:
            bool: Komutun başarıyla gönderilip gönderilmediği
        """
        if not self.serial_interface:
            self.logger.error("Cannot send register configuration: SerialInterface not initialized.")
            return False
        if not self._require_status(
            SystemStatus.READY, SystemStatus.CALIBRATING,
            command="REGISTER_CONFIGURATION"
        ):
            return False

        self.logger.info(f"Sending register configuration with {len(register_values)} values...")
        return self.serial_interface.send_register_configuration(register_values)

    def send_set_reg34(self, val_hex_or_int) -> bool:
        """Radarın reg34 (0x22 / Step Bin) değerini doğrudan donanıma yazar (her durumda çalışır)"""
        if not self.serial_interface:
            self.logger.error("Cannot send SET_REG34: SerialInterface not initialized.")
            return False
        return self.serial_interface.send_set_reg34(val_hex_or_int)

    def send_read_reg34(self) -> bool:
        """Radarın reg34 donanım değerini geri okur"""
        if not self.serial_interface:
            return False
        return self.serial_interface.send_read_reg34()

    def send_calibrate_command(self) -> bool:
        """
        Embedded sisteme CALIBRATE komutu gönderir.
        Sistem READY durumunda olmalıdır.

        Returns:
            bool: Komutun başarıyla gönderilip gönderilmediği
        """
        if not self.serial_interface:
            self.logger.error("Cannot send CALIBRATE command: SerialInterface not initialized.")
            return False
        if not self._require_status(SystemStatus.READY, command="CALIBRATE"):
            return False

        self.logger.info("Sending CALIBRATE command...")
        success = self.serial_interface.send_calibrate_command()
        if success:
            self._update_status(SystemStatus.CALIBRATING, source="System (CALIBRATE)")
        return success

    def send_ai_init_command(self) -> bool:
        """
        Embedded sisteme AI_INIT komutu gönderir.
        Sistem CALIBRATING durumunda olmalıdır.

        Returns:
            bool: Komutun başarıyla gönderilip gönderilmediği
        """
        if not self.serial_interface:
            self.logger.error("Cannot send AI_INIT command: SerialInterface not initialized.")
            return False
        if not self._require_status(SystemStatus.CALIBRATING, command="AI_INIT"):
            return False

        self.logger.info("Sending AI_INIT command to initialize AI system...")
        return self.serial_interface.send_ai_init_command()

    def send_ai_inference_command(self) -> bool:
        """
        Embedded sisteme AI_INFERENCE komutu gönderir.
        Sistem CALIBRATING durumunda olmalıdır (AI_INIT sonrası).

        Returns:
            bool: Komutun başarıyla gönderilip gönderilmediği
        """
        if not self.serial_interface:
            self.logger.error("Cannot send AI_INFERENCE command: SerialInterface not initialized.")
            return False
        if not self._require_status(SystemStatus.CALIBRATING, command="AI_INFERENCE"):
            return False

        self.logger.info("Sending AI_INFERENCE command to start AI inference...")
        return self.serial_interface.send_ai_inference_command()

    def start_streaming(self):
        if not self.serial_interface:
            self.logger.error("Cannot start streaming: SerialInterface not initialized.")
            return False
        if not self._require_status(
            SystemStatus.READY, SystemStatus.CALIBRATING,
            command="START_STREAMING"
        ):
            return False

        # Log current model info before starting
        current_model = self.config.get_current_model()
        self.logger.info("=" * 50)
        self.logger.info("STARTING DATA STREAM")
        self.logger.info(f"Model: {current_model.name}")
        self.logger.info(f"Pipeline: {current_model.pipeline.value}")
        self.logger.info(f"Expected preprocessing:")
        
        # Log preprocessing steps
        pipeline = current_model.pipeline.value
        if pipeline == "raw":
            self.logger.info("  - No preprocessing (raw data)")
        elif pipeline == "raw_normalize":
            self.logger.info("  - Normalization only")
        elif pipeline == "bandpass":
            self.logger.info("  - Bandpass filter only")
        elif pipeline == "bandpass_normalize":
            self.logger.info("  - Bandpass filter + Normalization")
        
        self.logger.info("=" * 50)
                
        start_command = self.config.get('protocol', 'command_start', 'START')
        self.logger.info(f"Sending '{start_command}' command to start streaming...")
        
        # START komutunu gönder
        success = self.serial_interface.send_command(start_command)
        
        if success:
            # Bu protokolde <STR> marker'ı olmadığı için direk streaming'i aktifleştir
            self.serial_interface.activate_streaming()
            self._update_status(SystemStatus.STREAMING, source="System (START)")
        
        return success

    def stop_streaming(self):
        """
        Veri akışını durdurur (STOP komutu gönderir).
        Başarılı olursa STREAMING state'inden READY'e geçer.

        Returns:
            bool: İşlemin başarılı olup olmadığı
        """
        if not self.serial_interface:
            self.logger.error("Cannot stop streaming: SerialInterface not initialized.")
            return False

        stop_command = self.config.get('protocol', 'command_stop', 'STOP')
        self.logger.info(f"Sending '{stop_command}' command to stop streaming...")
        success = self.serial_interface.send_command(stop_command)
        # Sadece STREAMING state'indeyken READY'e geç (shutdown akışını bozma)
        if success and self.status == SystemStatus.STREAMING:
            self._update_status(SystemStatus.READY, source="System (STOP)")
        return success

    def send_run_command(self) -> bool:
        """
        Embedded sisteme RUN komutu gönderir.
        AI modelini dummy veriyle test eder. CALIBRATING durumunda olunmalıdır.

        Returns:
            bool: Komutun başarıyla gönderilip gönderilmediği
        """
        if not self.serial_interface:
            self.logger.error("Cannot send RUN command: SerialInterface not initialized.")
            return False
        if not self._require_status(SystemStatus.CALIBRATING, command="RUN"):
            return False

        self.logger.info("Sending RUN command for AI inference test...")
        return self.serial_interface.send_command("run")

    def send_run_radar_command(self) -> bool:
        """
        Embedded sisteme RUN_RADAR komutu gönderir.
        Radar verisi toplayıp AI modeline besler. CALIBRATING durumunda olunmalıdır.

        Returns:
            bool: Komutun başarıyla gönderilip gönderilmediği
        """
        if not self.serial_interface:
            self.logger.error("Cannot send RUN_RADAR command: SerialInterface not initialized.")
            return False
        if not self._require_status(SystemStatus.CALIBRATING, command="RUN_RADAR"):
            return False

        self.logger.info("Sending RUN_RADAR command for radar data collection and AI inference...")
        return self.serial_interface.send_command("run_radar")

    def sync_status_with_serial_interface(self):
        """
        SerialInterface'in durum bilgisini sistemin durumuyla senkronize eder.
        Bu metod, durum uyumsuzluğu olduğunda çağrılabilir.
        """
        if not self.serial_interface:
            self.logger.error("Cannot sync status: SerialInterface not initialized.")
            return False
            
        serial_state = self.serial_interface.get_system_state()
        if serial_state != self.status:
            self.logger.info(f"Syncing system status from serial interface: {self.status.name} -> {serial_state.name}")
            self._update_status(serial_state, source="System (StatusSync)")
            return True
        return False

    def shutdown(self, graceful=True):
        """
        Tüm sistemi kapatır ve kaynakları serbest bırakır.
        
        Args:
            graceful: True ise önce veri akışını durdurmayı dener, False ise doğrudan kapatır
        """
        current_status = self.status
        if current_status in [SystemStatus.SHUTDOWN, SystemStatus.STOPPING]:
            self.logger.warning(f"System shutdown already in progress or completed ({current_status.name}).")
            return

        self.logger.info(f"Shutting down Fallower System ({'graceful' if graceful else 'forced'})...")
        self._update_status(SystemStatus.STOPPING)

        # Graceful kapatma için akışı durdurmayı dene
        if graceful and self.serial_interface and self.serial_interface.is_streaming():
            self.logger.info("Attempting to stop stream gracefully...")
            self.stop_streaming()
            # Durdurma komutunun işlenmesi ve <STP> yanıtı için biraz bekle
            time.sleep(0.5 + (self.config.get('serial','timeout', 0.1) * 2))

        # Modülleri ters bağımlılık sırasına göre durdur
        self.logger.debug("Stopping ModelPredictor...")
        if self.model_predictor:
            self.model_predictor.stop()

        # DataProcessor'ın özel stop'u yok, ama referansı temizleyebiliriz
        self.logger.debug("Stopping DataProcessor...")
        if self.data_processor:
            self.data_processor.stop()  # Bu genellikle sadece state değiştirir

        # SerialInterface kritik, diğerleri (özellikle storage) bitmeden kapatılabilir
        self.logger.debug("Stopping SerialInterface...")
        if self.serial_interface:
            self.serial_interface.stop()

        # DataStorage en sonlardan biri, kuyruğun boşalması için zaman tanınmalı
        self.logger.debug("Stopping DataStorage...")
        if self.data_storage:
            self.data_storage.stop()

        # En son EventBus
        self.logger.debug("Stopping EventBus...")
        if self.event_bus:
            self.event_bus.stop()

        self._update_status(SystemStatus.SHUTDOWN)
        self.logger.info("Fallower System has been shut down.")