import serial
import time
import queue
import threading
import re
import numpy as np
from typing import Optional, Dict

from core.config import Configuration
from core.logger import Logger
from core.event_bus import EventBus, Event
from core.enums import EventType, SystemStatus
from data.models import Frame


class SerialInterface:
    """STM32 ile seri iletişim için arayüz"""
    def __init__(self):
        self.config = Configuration()
        self.logger = self.config.get_logger()
        self.event_bus = EventBus()

        # Yapılandırma ve protokol
        serial_config = self.config.get('serial')
        self.port = serial_config.get('port')
        self.baudrate = serial_config.get('baudrate')
        self.timeout = serial_config.get('timeout')
        self.read_chunk_size = serial_config.get('read_chunk_size', 512)   # reduced for stability

        self.protocol = self.config.get('protocol')
        self.start_marker = self.protocol['start_marker']
        self.data_marker = self.protocol['data_marker']
        self.stop_marker = self.protocol['stop_marker']
        self.ack_marker = self.protocol.get('ack_marker')
        self.error_marker = self.protocol.get('error_marker')

        # Durum ve yapılar
        self.ser: Optional[serial.Serial] = None
        self.data_buffer = bytearray()
        self.max_buffer_size = 15 * 1024 * 1024  # Buffer boyutunu artır (15MB)
        
        self._running = False
        self._receiver_thread: Optional[threading.Thread] = None
        self._command_queue = queue.Queue()
        self._command_thread: Optional[threading.Thread] = None
        self._lock = threading.Lock()
        self._streaming_active = False
        self._paused = False # Flag to pause receiver thread for external data handling
        self._frame_counter = 0
        self._system_state = SystemStatus.IDLE  # Sistem durumunu takip et

        # Beklenen frame özellik sayısı
        self.expected_frame_features = self.config.get('processing', 'frame_feature_count')
        
        # Komut yanıtlarını beklemek için
        self._response_event = threading.Event()
        self._last_response = None
        
        # DataProcessor ile doğrudan bağlantı - EventBus'ı bypass etmek için
        self.data_processor = None  # DataProcessor instance
        self._direct_queue = None  # Doğrudan DataProcessor'a göndermek için Queue
        self._direct_mode = False  # Doğrudan mod aktif mi?
        
        # Geri basınç (backpressure) parametreleri
        self._backpressure_threshold = 0.8  # %80 doluluk kuyruğu yavaşlatır
        self._reading_throttle = False  # Okuma hızını yavaşlatma
        self._dropped_frame_count = 0  # Direct queue dolu olduğunda düşürülen frame sayısı

        # AI sınıf etiketleri (activity classification)
        self.class_labels = [
            "FALL",
            "GETUP",
            "GETUPGR",
            "SIT",
            "STILLMOVE",
            "STILLNOACT",
            "WALK"
        ]

    def set_direct_mode(self, data_processor, direct_queue):
        """DataProcessor ile doğrudan bağlantı kurma"""
        self.data_processor = data_processor
        self._direct_queue = direct_queue
        self._direct_mode = True
        self.logger.info("Direct queue mode activated with DataProcessor")
    
    def activate_streaming(self):
        """Streaming modunu aktif hale getirir (harici START komutu sonrası)."""
        self._streaming_active = True
        self._system_state = SystemStatus.STREAMING
        self.logger.info("Streaming activated")

    def connect(self) -> bool:
        if self.ser and self.ser.is_open:
            self.logger.warning(f"Already connected to {self.port}")
            return True
        try:
            # Bağlantı sırasında timeout biraz daha uzun olabilir
            connect_timeout = max(self.timeout * 5, 1.0) if self.timeout else 1.0
            self.ser = serial.Serial(self.port, self.baudrate, timeout=connect_timeout)
            if self.ser.is_open:
                time.sleep(0.1)  # Cihazın hazır olması için kısa bekleme
                self.ser.reset_input_buffer()
                self.ser.reset_output_buffer()
                self.logger.info(f"Successfully connected to {self.port} at {self.baudrate} baud")
                return True
            else:
                self.logger.error(f"Serial port {self.port} opened but status is not 'open'.")
                self.ser = None
                return False
        except serial.SerialException as e:
            self.logger.error(f"Connection failed to {self.port}: {e}")
            self.ser = None
            return False
        except Exception as e:
            self.logger.error(f"An unexpected error occurred during connection: {e}", exc_info=True)
            self.ser = None
            return False

    def disconnect(self):
        if self.ser and self.ser.is_open:
            try:
                self.ser.close()
                self.logger.info(f"Disconnected from {self.port}")
            except Exception as e:
                self.logger.error(f"Error closing serial port {self.port}: {e}", exc_info=True)
        self.ser = None
        self._streaming_active = False

    def start(self) -> bool:
        if self._running:
            self.logger.warning("Serial interface is already running.")
            return True

        if not self.ser:
            if not self.connect():
                self.logger.error("Cannot start serial interface: connection failed.")
                return False

        self._running = True
        self._streaming_active = False
        self.data_buffer.clear()
        self._frame_counter = 0

        self._command_thread = threading.Thread(target=self._command_loop, name="SerialCommandThread")
        self._command_thread.daemon = True
        self._command_thread.start()

        self._receiver_thread = threading.Thread(target=self._receiver_loop, name="SerialReceiverThread")
        self._receiver_thread.daemon = True
        self._receiver_thread.start()

        self.logger.info("Serial interface started (threads launched).")
        return True

    def stop(self):
        if not self._running:
            return

        self.logger.info("Stopping serial interface...")
        self._running = False

        # Komut thread'ine durma sinyali gönder
        if self._command_thread and self._command_thread.is_alive():
            self._command_queue.put(None)
            self._command_thread.join(timeout=1.0)

        # Alıcı thread seri portta bekleyebilir, timeout önemli
        if self._receiver_thread and self._receiver_thread.is_alive():
            receiver_timeout = max(1.0, self.timeout * 2 if self.timeout else 1.0)
            self._receiver_thread.join(timeout=receiver_timeout)
            if self._receiver_thread.is_alive():
                self.logger.warning("Receiver thread did not exit gracefully.")

        self.disconnect()

        self._command_thread = None
        self._receiver_thread = None
        self.logger.info("Serial interface stopped.")

    def pause_reading(self):
        """Pause receiver thread to allow external reading"""
        self._paused = True
        self.logger.info("Serial receiver thread paused for external access.")

    def resume_reading(self):
        """Resume receiver thread"""
        self._paused = False
        self.logger.info("Serial receiver thread resumed.")

    def send_command(self, command: str) -> bool:
        """Komut gönderir ve başarılı olup olmadığını döndürür"""
        if not self._running:
            self.logger.error("Cannot send command: Serial interface not running")
            return False
        if command is None:
            self.logger.warning("send_command(None) called — ignored. Use stop() to stop the command loop.")
            return False

        self.logger.debug(f"Queuing command: {command}")
        self._command_queue.put(command)
        return True

    def send_command_wait_response(self, command: str, timeout: float = 5.0) -> Optional[str]:
        """Komut gönderir ve cevap bekler"""
        if not self._running:
            self.logger.error("Cannot send command: Serial interface not running")
            return None
            
        # Önceki yanıt verisini temizle
        self._response_event.clear()
        self._last_response = None
        
        # Komutu kuyruğa ekle
        self.logger.debug(f"Queuing command with wait: {command}")
        self._command_queue.put(command)
        
        # Yanıt bekle
        if self._response_event.wait(timeout):
            return self._last_response
        else:
            self.logger.warning(f"Timeout waiting for response to command: {command}")
            return None

    # Embedded sisteme özel komutlar
    def send_init_command(self) -> bool:
        """INIT komutunu gönderir"""
        self.logger.info("Sending INIT command...")
        return self.send_command("INIT")
        
    def send_ready_command(self) -> bool:
        """READY komutunu gönderir"""
        self.logger.info("Sending READY command...")
        return self.send_command("READY")
        
    def send_calibrate_command(self) -> bool:
        """CALIBRATE komutunu gönderir"""
        self.logger.info("Sending CALIBRATE command...")
        return self.send_command("CALIBRATE")

    def send_ai_init_command(self) -> bool:
        """AI_INIT komutunu gönderir"""
        self.logger.info("Sending AI_INIT command...")
        return self.send_command("AI_INIT")

    def send_ai_inference_command(self) -> bool:
        """AI_INFERENCE komutunu gönderir"""
        self.logger.info("Sending AI_INFERENCE command...")
        return self.send_command("AI_INFERENCE")
    
    def send_data(self, data: bytes, length: int) -> bool:
        """Send raw binary data - primary method"""
        if data is None or length == 0:
            return False  # Invalid data or length

        if not self.ser or not self.ser.is_open:
            return False

        try:
            with self._lock:
                bytes_sent = self.ser.write(data)
            
            if bytes_sent == len(data):
                return True
            else:
                self.logger.error(f"Partial send: {bytes_sent}/{len(data)} bytes")
                return False
        except Exception as e:
            self.logger.error(f"Error sending binary data: {e}")
            return False
    
    def send_register_configuration(self, register_values: Dict[int, int]) -> bool:
        """Radar register değerlerini gönderir
        
        Args:
            register_values: {register_numarası: değer} formatında sözlük
        """
        if not register_values:
            self.logger.error("Cannot send empty register configuration")
            return False
            
        # Register yapılandırma mesajını oluştur
        # Doğrudan önceden formatlanmış string göndermek yerine sözlükten oluştur
        command = "<STR>"
        for reg_num, value in register_values.items():
            if 0 <= reg_num <= 58:  # R00-R58 geçerli register aralığı
                command += f"<R{reg_num:02d}>0x{value:02X}"
            else:
                self.logger.warning(f"Invalid register number: {reg_num}, skipping")
                
        command += ">"
        
        self.logger.info(f"Sending register configuration with {len(register_values)} values...")
        self.logger.debug(f"Register command: {command}")
        return self.send_command(command)

    def send_set_reg34(self, val_hex_or_int) -> bool:
        """Radarın reg34 (0x22 / Step Bin) değerini tek komutla doğrudan donanıma yazar"""
        if isinstance(val_hex_or_int, int):
            val_str = f"0x{val_hex_or_int:02X}"
        else:
            val_str = str(val_hex_or_int)
        command = f"SET_REG34:{val_str}"
        self.logger.info(f"Sending direct reg34 command: {command}")
        return self.send_command(command)

    def send_read_reg34(self) -> bool:
        """Radarın reg34 donanım değerini geri okur"""
        return self.send_command("READ_REG34")

    def _command_loop(self):
        self.logger.info("Command loop started.")
        while self._running:
            try:
                command = self._command_queue.get(timeout=0.5)
                if command is None:
                    break
                self._execute_command(command)
                self._command_queue.task_done()
            except queue.Empty:
                continue
            except Exception as e:
                self.logger.error(f"Error in command loop: {e}", exc_info=True)
                time.sleep(0.1)
        self.logger.info("Command loop finished.")

    def _execute_command(self, command: str):
        if self.ser and self.ser.is_open:
            try:
                cmd_bytes = (command + '\r\n').encode('utf-8')
                bytes_sent = 0
                with self._lock:
                    bytes_sent = self.ser.write(cmd_bytes)
                if bytes_sent == len(cmd_bytes):
                    self.logger.info(f"Sent command: {command}")
                    self.event_bus.publish(Event(EventType.COMMAND_SENT, data=command, source="SerialInterface"))
                else:
                    self.logger.error(f"Failed to send full command '{command}'. Sent {bytes_sent}/{len(cmd_bytes)} bytes.")

            except serial.SerialTimeoutException:
                self.logger.error(f"Timeout sending command: {command}")
                self._check_connection_health()
            except serial.SerialException as e:
                self.logger.error(f"Serial error sending command '{command}': {e}", exc_info=True)
                self._check_connection_health()
            except Exception as e:
                self.logger.error(f"Unexpected error sending command '{command}': {e}", exc_info=True)
                self._check_connection_health()
        else:
            self.logger.error(f"Cannot send command '{command}': Serial port not open or available.")

    def _receiver_loop(self):
        self.logger.info("Receiver loop started.")
        while self._running:
            if self._paused:
                time.sleep(0.1)
                continue

            if not self.ser or not self.ser.is_open:
                time.sleep(1.0)
                continue

            try:
                # Geri basınç kontrolü - kuyruk doluysa okumayı yavaşlat
                if self._direct_mode and self._direct_queue and self._direct_queue.qsize() / self._direct_queue.maxsize > self._backpressure_threshold:
                    self._reading_throttle = True
                    time.sleep(0.005)  # Daha uzun bekleme
                else:
                    self._reading_throttle = False
                    time.sleep(0.0005)  # OPTIMIZED: Even faster (0.001 → 0.0005)
                
                # Non-blocking okuma için in_waiting kontrolü önemli
                bytes_available = 0
                with self._lock:
                    if self.ser and self.ser.is_open:
                        bytes_available = self.ser.in_waiting

                if bytes_available > 0:
                    read_size = min(bytes_available, self.read_chunk_size)
                    data = b''
                    with self._lock:
                        if self.ser and self.ser.is_open:
                            data = self.ser.read(read_size)

                    if data:
                        self.data_buffer.extend(data)

                        if len(self.data_buffer) > self.max_buffer_size:
                            overload = len(self.data_buffer) - self.max_buffer_size
                            self.logger.warning(f"Buffer overload ({len(self.data_buffer)} > {self.max_buffer_size}). Discarding {overload} oldest bytes.")
                            self.data_buffer = self.data_buffer[overload:]

                        self._process_buffer()
                else:
                    # Veri yoksa CPU'yu yormamak için kısa süre uyu (OPTIMIZED)
                    time.sleep(0.0005)  # OPTIMIZED: Even faster (0.001 → 0.0005)

            except serial.SerialException as e:
                self.logger.error(f"Serial error in receiver loop: {e}", exc_info=True)
                self.event_bus.publish(Event(EventType.ERROR_OCCURRED, data={"error": e, "source": "SerialReceiver"}, source="SerialInterface"))
                self._streaming_active = False
                self.disconnect()
                time.sleep(2.0)
            except Exception as e:
                self.logger.error(f"Unexpected error in receiver loop: {e}", exc_info=True)
                self.event_bus.publish(Event(EventType.ERROR_OCCURRED, data={"error": e, "source": "SerialReceiver"}, source="SerialInterface"))
                time.sleep(0.1)

        self.logger.info("Receiver loop finished.")


    def _process_buffer(self):
        """Tamponu işleyerek marker'ları bulur ve olayları yayınlar."""
        while True:
            processed_something = False

            buffer_len = len(self.data_buffer)
            if buffer_len == 0:
                break

            # Streaming aktif değilse: sadece komut yanıtlarını işle
            if not self._streaming_active:
                ack_pos = self.data_buffer.find(self.ack_marker) if self.ack_marker else -1
                err_pos = self.data_buffer.find(self.error_marker) if self.error_marker else -1

                if ack_pos != -1:
                    self.logger.info("ACK marker detected.")
                    self.event_bus.publish(Event(EventType.COMMAND_RESPONSE, data="ACK", source="SerialInterface"))
                    self.data_buffer = self.data_buffer[ack_pos + len(self.ack_marker):]
                    processed_something = True
                elif err_pos != -1:
                    processed_bytes = self._handle_error_marker(offset=err_pos)
                    self.data_buffer = self.data_buffer[processed_bytes:]
                    processed_something = True
                else:
                    # [STATE] gibi durum yanıtlarını ara
                    state_pos = self.data_buffer.find(b"[STATE]")
                    if state_pos != -1:
                        line_end = self.data_buffer.find(b"\n", state_pos)
                        if line_end != -1:
                            state_line = self.data_buffer[state_pos:line_end].decode('utf-8', errors='ignore')
                            self.logger.info(f"State change detected: {state_line}")
                            self._last_response = state_line
                            self._response_event.set()
                            
                            # Durum değişikliği işle
                            new_state = None
                            if "IDLE" in state_line:
                                new_state = SystemStatus.IDLE
                            elif "INIT" in state_line:
                                new_state = SystemStatus.INITIALIZING
                            elif "READY" in state_line:
                                new_state = SystemStatus.READY
                            
                            if new_state and new_state != self._system_state:
                                self._system_state = new_state
                                self.logger.info(f"System state updated to: {new_state.name}")
                                self.event_bus.publish(Event(EventType.STATUS_CHANGED, data=new_state, source="SerialInterface"))
                                
                            self.data_buffer = self.data_buffer[line_end + 1:]
                            processed_something = True
                    else:
                        # Diğer line-by-line yanıtları işle
                        line_end = self.data_buffer.find(b"\n")
                        if line_end != -1:
                            raw_line_bytes = self.data_buffer[:line_end]
                            line = raw_line_bytes.decode('utf-8', errors='ignore').strip()

                            # -------------------------------------------------------------------
                            # BINARY GARBAGE FILTER:
                            # <DTA>/<END> marker'ları veya yüksek oranda non-printable byte
                            # içeren satırlar binary frame data'dır. Sessizce at.
                            # Neden olur: collect/stream bittikten sonra _streaming_active=False
                            # yapılır ama buffer'da hala <DTA> paket byte'ları kalabilir.
                            # -------------------------------------------------------------------
                            is_binary_garbage = False
                            if b'<DTA' in raw_line_bytes or b'<END>' in raw_line_bytes:
                                is_binary_garbage = True
                            elif len(raw_line_bytes) > 0:
                                non_printable = sum(
                                    1 for b in raw_line_bytes
                                    if b < 0x20 and b not in (0x09, 0x0A, 0x0D)  # tab, lf, cr
                                )
                                ratio = non_printable / len(raw_line_bytes)
                                if ratio > 0.15:  # >%15 binary → garbage
                                    is_binary_garbage = True
                                # Kısa satırlarda herhangi bir non-printable varsa garbage
                                elif len(raw_line_bytes) <= 10 and non_printable > 0:
                                    is_binary_garbage = True

                            if is_binary_garbage:
                                # Binary frame verisi non-streaming path'e düştü → at
                                self.data_buffer = self.data_buffer[line_end + 1:]
                                processed_something = True
                                continue

                            if line:
                                # [REAL] mesajları için sade format - sadece bir satır göster
                                if line.startswith('[REAL]') or line.startswith('[REAL_STATS]'):
                                    import datetime
                                    now = datetime.datetime.now().strftime("%M:%S.%f")[:-3]
                                    print(f"  {now}  {line}")
                                    # Logger'a DEBUG olarak yazma - sadece print
                                else:
                                    self.logger.debug(f"Response received: {line}")

                                # PRINT RAW LOGITS WITH CLASS PREDICTION
                                # Format: [float, float, float, float, float, float, float]
                                if line.startswith('[') and line.endswith(']'):
                                    # Additional validation: check if line looks like valid logits (7 floats)
                                    if ','.join(line.split(',')[:2]).count('.') > 1:
                                        from processing.model_predictor import ModelPredictor
                                        prediction = ModelPredictor.parse_logits_string(line, self.class_labels)
                                        if prediction:
                                            predicted_class, confidence, probs = prediction
                                            print(f"{line} -> {predicted_class} ({confidence:.1f}%)")
                                        else:
                                            print(line)
                                    else:
                                        # Corrupted message - log but don't print
                                        self.logger.warning(f"Corrupted logits message: {line}")

                                    self._last_response = line
                                    self._response_event.set()

                                    if "READY" in line and "state" in line.lower():
                                        if self._system_state != SystemStatus.READY:
                                            self._system_state = SystemStatus.READY
                                            self.logger.info(f"System state updated to READY (from response)")
                                            self.event_bus.publish(Event(EventType.STATUS_CHANGED, data=SystemStatus.READY, source="SerialInterface"))

                            self.data_buffer = self.data_buffer[line_end + 1:]
                            processed_something = True

            # Streaming aktifse: DATA ve STOP marker'larını ara
            if self._streaming_active:
                # self.logger.debug(f"DEBUG: Processing buffer in streaming mode, buffer size: {len(self.data_buffer)}")  # Bu satırı ekleyin
                    # Bu kısmı buraya ekleyin:
                if len(self.data_buffer) > 1000:  # Büyük buffer varsa
                    hex_dump = self.data_buffer[:200].hex()  # İlk 200 byte'ı hex olarak
                    # self.logger.info(f"DEBUG: Buffer HEX dump: {hex_dump}")
                    
                    # <DTA> hex karşılığını ara
                    dta_hex = bytes.fromhex('3C4454413E')  # <DTA>
                    dta_pos = self.data_buffer.find(dta_hex)
                    # self.logger.info(f"DEBUG: <DTA> hex search result: {dta_pos}")
                    
                data_pos = self.data_buffer.find(self.data_marker)  # <DTA>
                stop_pos = self.data_buffer.find(self.stop_marker)  # <STP>
                # self.logger.debug(f"DEBUG: DTA pos: {data_pos}, STP pos: {stop_pos}")  # Bu satırı ekleyin



                # STOP marker'ı öncelikli (streaming'i durdurur)
                if stop_pos != -1 and (data_pos == -1 or stop_pos < data_pos):
                    self.logger.info("Stream stop marker detected!")
                    if stop_pos > 0:
                        self.logger.debug(f"Discarding {stop_pos} bytes before STOP marker.")
                    self._streaming_active = False
                    self.data_buffer = self.data_buffer[stop_pos + len(self.stop_marker):]
                    self._system_state = SystemStatus.READY
                    self.event_bus.publish(Event(EventType.STATUS_CHANGED, data=SystemStatus.READY, source="SerialInterface"))
                    processed_something = True

                # DATA marker'ı işle
                elif data_pos != -1:
                    # self.logger.info(f"DEBUG: Found DTA marker at position {data_pos}")  # Bu satırı ekleyin

                    # DTA marker'ından önceki veriyi atla
                    if data_pos > 0:
                        self.logger.debug(f"Discarding {data_pos} bytes before DATA marker while streaming.")
                        self.data_buffer = self.data_buffer[data_pos:]

                    # <END> işaretini ara
                    end_pos = self.data_buffer.find(b'<END>', len(self.data_marker))
                    # self.logger.info(f"DEBUG: Looking for END marker, found at position: {end_pos}")  # Bu satırı ekleyin

                    
                    if end_pos != -1:  # <END> bulundu - tam paket var
                        try:
                            # Etiketlerden arındırılmış veri: sadece meta + örnekler
                            clean_frame_data = self.data_buffer[4:end_pos]  # <DTA> 4 byte atla
                            
                            # Debug: clean_frame_data boyutunu logla
                            # self.logger.debug(f"Clean frame data size: {len(clean_frame_data)} bytes")
                            
                            if len(clean_frame_data) >= 16:  # Normal durum: tam meta veri var
                                import struct
                                
                                # Meta verileri çıkar (embedded format: IIIHH)
                                meta_data = struct.unpack('<IIIHH', clean_frame_data[:16])
                                frame_number, timestamp, duration, sample_count, crc = meta_data
                                
                                # Örnekleri al (meta verilerden sonraki kısım)
                                samples_data = clean_frame_data[16:]
                                
                                # CRC doğrulama
                                crc_data = clean_frame_data[:14] 
                                calculated_crc = self._calculate_crc16(crc_data)
                                crc_valid = (calculated_crc == crc)
                                
                                if not crc_valid:
                                    pass
                                    # self.logger.warning(f"CRC validation failed for frame #{frame_number}: expected {crc}, calculated {calculated_crc}")
                                
                                # Sample sayısı kontrolü
                                expected_sample_bytes = sample_count * 2
                                actual_sample_bytes = len(samples_data)
                                if expected_sample_bytes != actual_sample_bytes:
                                    self.logger.warning(f"Sample count mismatch for frame #{frame_number}: expected {expected_sample_bytes} bytes, got {actual_sample_bytes} bytes")
                                
                                # Frame counter'ı senkronize et
                                self._frame_counter = frame_number
                                
                                # === OLAYLARI YAYINLA ===
                                
                                # 1. META VERİLERİ KAYDET
                                self.event_bus.publish(Event(
                                    EventType.FRAME_METADATA,
                                    data={
                                        "frame_number": frame_number,
                                        "timestamp": timestamp,
                                        "duration": duration,
                                        "sample_count": sample_count,
                                        "crc": crc,
                                        "crc_valid": crc_valid
                                    },
                                    source="SerialInterface"
                                ))
                                
                                # 2. HAM FRAME VERİSİNİ KAYDET
                                self.event_bus.publish(Event(
                                    EventType.RAW_FRAME_DATA,
                                    data={
                                        "frame_number": frame_number,
                                        "timestamp": timestamp,
                                        "raw_data": clean_frame_data
                                    },
                                    source="SerialInterface"
                                ))
                                
                                # 3. NORMAL FRAME İŞLEME (model için)
                                frame = Frame(raw_data=samples_data, frame_id=frame_number)
                                
                                # Doğrudan mod aktifse
                                if self._direct_mode and self._direct_queue is not None:
                                    try:
                                        self._direct_queue.put_nowait(frame)
                                    except queue.Full:
                                        self._dropped_frame_count += 1
                                        if self._dropped_frame_count % 10 == 1:
                                            self.logger.warning(
                                                f"Direct queue full — frame {frame_number} dropped "
                                                f"(total dropped: {self._dropped_frame_count})"
                                            )
                                else:
                                    self.event_bus.publish(Event(EventType.FRAME_RECEIVED, data=frame, source="SerialInterface"))

                                # self.logger.info(f"Frame {frame_number} processed: {len(samples_data)} sample bytes, CRC {'valid' if crc_valid else 'invalid'}")
                                
                            elif len(clean_frame_data) >= 14:  # Kısmi meta veri - CRC yok
                                import struct
                                
                                self.logger.warning(f"Partial frame data: {len(clean_frame_data)} bytes (14-15 bytes, missing CRC)")
                                
                                # Sadece ilk 12 byte'ı oku (CRC olmadan)
                                meta_data = struct.unpack('<IIIH', clean_frame_data[:12])
                                frame_number, timestamp, duration, sample_count = meta_data
                                crc = 0  # CRC yok
                                crc_valid = False  # CRC kontrol edilemez
                                
                                # Örnekleri al
                                samples_data = clean_frame_data[12:]
                                
                                # Frame counter'ı senkronize et
                                self._frame_counter = frame_number
                                
                                # META VERİLERİ KAYDET (CRC olmadan)
                                self.event_bus.publish(Event(
                                    EventType.FRAME_METADATA,
                                    data={
                                        "frame_number": frame_number,
                                        "timestamp": timestamp,
                                        "duration": duration,
                                        "sample_count": sample_count,
                                        "crc": crc,
                                        "crc_valid": crc_valid
                                    },
                                    source="SerialInterface"
                                ))
                                
                                # HAM FRAME VERİSİNİ KAYDET
                                self.event_bus.publish(Event(
                                    EventType.RAW_FRAME_DATA,
                                    data={
                                        "frame_number": frame_number,
                                        "timestamp": timestamp,
                                        "raw_data": clean_frame_data
                                    },
                                    source="SerialInterface"
                                ))
                                
                                # NORMAL FRAME İŞLEME
                                frame = Frame(raw_data=samples_data, frame_id=frame_number)
                                
                                if self._direct_mode and self._direct_queue is not None:
                                    try:
                                        self._direct_queue.put_nowait(frame)
                                    except queue.Full:
                                        self._dropped_frame_count += 1
                                        if self._dropped_frame_count % 10 == 1:
                                            self.logger.warning(
                                                f"Direct queue full — frame {frame_number} dropped "
                                                f"(total dropped: {self._dropped_frame_count})"
                                            )
                                else:
                                    self.event_bus.publish(Event(EventType.FRAME_RECEIVED, data=frame, source="SerialInterface"))

                                self.logger.info(f"Frame {frame_number} processed (partial): {len(samples_data)} sample bytes, no CRC")
                                
                            else:
                                self.logger.warning(f"Frame skipped - data too small: {len(clean_frame_data)} bytes (minimum 14 required)")
                                
                        except struct.error as e:
                            self.logger.error(f"Error unpacking frame meta data: {e}")
                        except Exception as e:
                            self.logger.error(f"Error processing frame: {e}", exc_info=True)
                        
                        # İşlenen paketi buffer'dan çıkar
                        self.data_buffer = self.data_buffer[end_pos + 5:]  # <END> 5 byte
                        processed_something = True
                        
                    else:  # <END> bulunamadı - paket henüz tamamlanmamış
                        break  # Daha fazla veri bekle

            # Eğer bu turda bir şey işlendiyse, tamponu baştan kontrol et
            if processed_something:
                continue
            else:
                break

    def _calculate_crc16(self, data):
        """CRC-16-MODBUS hesaplama fonksiyonu - embedded ile aynı algoritma"""
        crc = 0xFFFF
        
        for byte in data:
            crc ^= byte
            for _ in range(8):
                if crc & 0x0001:
                    crc = (crc >> 1) ^ 0xA001  # CRC-16-MODBUS polinomu
                else:
                    crc >>= 1
                    
        return crc & 0xFFFF                 

    def _handle_error_marker(self, offset=0) -> int:
        """ERR marker'ını ve takip eden mesajı işler, işlenen byte sayısını döndürür."""
        marker_len = len(self.error_marker)
        err_pos = offset
        line_end_lf = self.data_buffer.find(b'\n', err_pos + marker_len)
        line_end_cr = self.data_buffer.find(b'\r', err_pos + marker_len)

        error_msg_end = -1
        consumed_bytes = marker_len

        if line_end_lf != -1 and line_end_cr != -1:
            error_msg_end = min(line_end_lf, line_end_cr)
            if self.data_buffer[error_msg_end:error_msg_end+2] == b'\r\n':
                consumed_bytes = (error_msg_end - err_pos) + 2
            else:
                consumed_bytes = (error_msg_end - err_pos) + 1
        elif line_end_lf != -1:
            error_msg_end = line_end_lf
            consumed_bytes = (error_msg_end - err_pos) + 1
        elif line_end_cr != -1:
            error_msg_end = line_end_cr
            consumed_bytes = (error_msg_end - err_pos) + 1

        error_msg = "Unknown Error (no newline)"
        if error_msg_end != -1:
            try:
                error_msg = self.data_buffer[err_pos + marker_len : error_msg_end].strip().decode('utf-8', errors='ignore')
            except Exception as decode_err:
                error_msg = f"Undecodable error message: {decode_err}"

        self.logger.error(f"ERR marker detected from device: {error_msg}")
        self.event_bus.publish(Event(EventType.COMMAND_RESPONSE, data=f"ERR: {error_msg}", source="SerialInterface"))
        self.event_bus.publish(Event(EventType.ERROR_OCCURRED, data={"error": error_msg, "source": "STM32"}, source="SerialInterface"))
        
        # Hata yanıtı olarak da ayarla (bekleyen bir komut varsa)
        self._last_response = f"ERROR: {error_msg}"
        self._response_event.set()

        return err_pos + consumed_bytes

    def _check_connection_health(self):
        """Bağlantı durumunu kontrol eder ve sorun varsa loglar/olay yayınlar."""
        is_healthy = False
        if self.ser:
            try:
                is_healthy = self.ser.is_open
            except Exception:  # Port aniden çekilirse exception verebilir
                is_healthy = False

        if not is_healthy:
            self.logger.error("Serial connection lost or unavailable.")
            self._streaming_active = False
            self._system_state = SystemStatus.ERROR
            if self.ser:
                self.disconnect()
            
            # Bağlantı hatası durumunda bekleyen yanıtları serbest bırak
            self._last_response = "ERROR: Serial connection lost"
            self._response_event.set()
            
            # Durum değişikliği olayını yayınla
            self.event_bus.publish(Event(EventType.STATUS_CHANGED, data=SystemStatus.ERROR, source="SerialInterface"))

    def is_streaming(self) -> bool:
        """Veri akışının aktif olup olmadığını döndürür."""
        return self._streaming_active
        
    def get_system_state(self) -> SystemStatus:
        """Sistemin şu anki durumunu döndürür."""
        return self._system_state
        
    def reset_state(self):
        """Durum bilgisini READY olarak sıfırlar (hata durumlarında debug için)"""
        self._system_state = SystemStatus.READY
        self._streaming_active = False
        self.logger.info("System state manually reset to READY")
        self.event_bus.publish(Event(EventType.STATUS_CHANGED, data=SystemStatus.READY, source="SerialInterface"))