import time
import time as _time
import logging
import sys
import re
import os
import datetime
from tqdm import tqdm
import csv
import struct
import serial
import threading

from core.config import Configuration
from core.logger import Logger
from system.fallower_system import FallowerSystem
from core.enums import SystemStatus
from core.event_bus import EventBus, Event
from core.enums import EventType

# Optional GUI Modules
try:
    from collect_operator import launch_collector
except ImportError:
    try:
        from veritoplama_operator import launch_collector
    except ImportError:
        launch_collector = None

try:
    from collect_viewer import launch_viewer
except ImportError:
    try:
        from veritoplama_goruntuleyici import launch_viewer
    except ImportError:
        launch_viewer = None

# Global variables for debug and display control
prediction_counter = 0
prediction_display_enabled = True

# =============================================================================
# RADAR PERİYODU SABİTİ
# =============================================================================
RADAR_PERIOD_MS = 100
RADAR_PERIOD_S = RADAR_PERIOD_MS / 1000.0


# =============================================================================
# 1. AI MODEL TRANSFER (FIRE HOSE)
# =============================================================================
def transfer_model_direct_firehose(fallower_system, model_path: str) -> bool:
    try:
        print("🔥 Starting FIRE HOSE direct transfer...")

        CHUNK_SIZE_FOR_CRC = 128
        file_data_for_crc = bytearray()

        with open(model_path, 'rb') as f:
            while True:
                chunk = f.read(CHUNK_SIZE_FOR_CRC)
                if not chunk:
                    break
                if len(chunk) % 4 != 0:
                    padding = (4 - (len(chunk) % 4))
                    chunk += b'\x00' * padding
                if len(chunk) < CHUNK_SIZE_FOR_CRC:
                    chunk += b'\x00' * (CHUNK_SIZE_FOR_CRC - len(chunk))
                file_data_for_crc.extend(chunk)

        file_size = len(file_data_for_crc)

        def crc32_stm32_compatible(data):
            poly = 0x04C11DB7
            crc_table = []
            for i in range(256):
                c = i << 24
                for j in range(8):
                    c = (c << 1) ^ poly if (c & 0x80000000) else c << 1
                crc_table.append(c & 0xffffffff)
            crc = 0xffffffff
            length, k = len(data), 0
            while length >= 4:
                v = ((data[k] << 24) & 0xFF000000) | ((data[k+1] << 16) & 0xFF0000) | \
                    ((data[k+2] << 8) & 0xFF00) | (data[k+3] & 0xFF)
                crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ v)]
                crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 8))]
                crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 16))]
                crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 24))]
                k += 4; length -= 4
            if length > 0:
                v = 0
                for i in range(length):
                    v |= data[k+i] << (24 - 8*i)
                if length == 1: v &= 0xFF000000
                elif length == 2: v &= 0xFFFF0000
                elif length == 3: v &= 0xFFFFFF00
                crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ v)]
                crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 8))]
                crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 16))]
                crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 24))]
            return crc

        file_crc32 = crc32_stm32_compatible(file_data_for_crc)
        print(f"📊 File info: {file_size} bytes, CRC32 = 0x{file_crc32:08X}")

        serial_iface = fallower_system.serial_interface
        if not serial_iface:
            print("❌ Serial interface not available")
            return False

        print("📡 Step 1: Sending load_model_binary command...")
        if not serial_iface.send_command("load_model_binary"):
            print("❌ Failed to send load_model_binary command")
            return False

        time.sleep(0.2)

        print("📤 Step 3: Sending metadata...")
        metadata_command = f"metadata:{file_size},0x{file_crc32:08X}"
        print(f"🔍 Metadata: {metadata_command}")
        if not serial_iface.send_command(metadata_command):
            print("❌ Failed to send metadata command")
            return False

        print("⏳ Step 4: Waiting for SDRAM clear and fire hose activation...")
        time.sleep(3.0)
        print("✅ Step 4: SDRAM ready — starting data transfer...")

        CHUNK_SIZE = 128
        MAX_RETRIES = 1
        BACKPRESSURE_HIGH = 0.9
        BACKPRESSURE_SLEEP = 0.0005
        SDRAM_RECOVERY_WAIT = 0.05
        BURST_PROTECTION_LIMIT = 10
        FLOW_CONTROL_WINDOW = 6

        total_chunks = (file_size + CHUNK_SIZE - 1) // CHUNK_SIZE
        print(f"🔥 Fire hosing {total_chunks} chunks...")

        failed_chunks = 0
        total_retries = 0
        sdram_errors = 0
        flow_control_history = []
        burst_counter = 0
        chunk_idx = 0

        with open(model_path, 'rb') as f, tqdm(total=total_chunks, desc="🔥 Fire Hose", unit="chunk",
                 bar_format='{l_bar}{bar}| {n_fmt}/{total_fmt} [{elapsed}<{remaining}]') as pbar:
            while True:
                chunk = f.read(CHUNK_SIZE)
                if not chunk:
                    break

                if len(chunk) % 4 != 0:
                    chunk += b'\x00' * (4 - (len(chunk) % 4))
                if len(chunk) < CHUNK_SIZE:
                    chunk += b'\x00' * (CHUNK_SIZE - len(chunk))

                sent = False
                attempts = 0
                while not sent and attempts < MAX_RETRIES:
                    attempts += 1
                    try:
                        with fallower_system.serial_interface._lock:
                            buf_len = len(fallower_system.serial_interface.data_buffer)
                            max_buf = 1024 * 1024
                            buffer_str = fallower_system.serial_interface.data_buffer.decode('utf-8', errors='ignore')
                            if 'DATA_ERROR' in buffer_str:
                                error_count = buffer_str.count('DATA_ERROR')
                                print(f"\n🚨 REALTIME SDRAM ERROR! Count: {error_count}")
                                time.sleep(SDRAM_RECOVERY_WAIT * 3)
                                fallower_system.serial_interface.data_buffer = bytearray()
                                burst_counter = 0
                                continue
                    except Exception:
                        buf_len = 0; max_buf = 1024 * 1024

                    buffer_ratio = buf_len / max_buf if max_buf else 0
                    if buffer_ratio > BACKPRESSURE_HIGH:
                        time.sleep(BACKPRESSURE_SLEEP * 5)
                        continue
                    elif buffer_ratio > 0.5:
                        time.sleep(BACKPRESSURE_SLEEP)

                    try:
                        ok = fallower_system.serial_interface.send_data(chunk, len(chunk))
                    except Exception as e:
                        ok = False; print(f"⚠️ send_data exception: {e}")

                    if ok:
                        sent = True
                    else:
                        total_retries += 1
                        time.sleep(0.0005)

                if not sent:
                    failed_chunks += 1
                    if failed_chunks > 5:
                        print(f"❌ Too many failed chunks ({failed_chunks}). Aborting transfer.")
                        return False

                if sdram_errors > 5:
                    print(f"❌ Too many SDRAM write errors ({sdram_errors}). Transfer aborted.")
                    return False

                pbar.update(1); chunk_idx += 1

                burst_counter += 1
                if burst_counter >= BURST_PROTECTION_LIMIT:
                    time.sleep(SDRAM_RECOVERY_WAIT * 0.5)
                    burst_counter = 0

                success_rate = sum(flow_control_history) / max(len(flow_control_history), 1)
                if success_rate < 0.8 and len(flow_control_history) >= FLOW_CONTROL_WINDOW:
                    time.sleep(0.003)
                elif chunk_idx % 30 == 0:
                    time.sleep(0.001)

        print(f"\n🔥 FIRE HOSE COMPLETE!")
        print(f"   📊 Total chunks: {total_chunks}")
        print(f"   ✅ Successful: {total_chunks - failed_chunks}")
        print(f"   ❌ Failed: {failed_chunks}")
        print(f"   🔄 Total retries: {total_retries}")
        print(f"   ⚠️ SDRAM errors: {sdram_errors}")

        if sdram_errors == 0:
            time.sleep(0.8)
        elif sdram_errors <= 2:
            time.sleep(1.5)
        elif sdram_errors <= 5:
            time.sleep(2.5)
        else:
            time.sleep(4.0)

        print("✅ Transfer and verification completed!")
        return True

    except Exception as e:
        print(f"❌ Fire hose transfer failed: {e}")
        import traceback
        traceback.print_exc()
        return False


# =============================================================================
# 2. ISR TEST CAPTURE & MONITOR
# =============================================================================
def capture_isr_test_data_and_monitor(fallower_system):
    ser = fallower_system.serial_interface.ser
    if not ser:
        print("❌ Serial port not open!")
        return

    print("\n⏳ 1. WAITING FOR COLLECTION (~60 seconds)...")
    fallower_system.serial_interface.pause_reading()

    try:
        buffer = b""
        start_wait_timeout = 70.0
        start_time = time.time()

        while (time.time() - start_time) < start_wait_timeout:
            if ser.in_waiting:
                try:
                    chunk = ser.read(ser.in_waiting)
                    buffer += chunk
                    if b"[ISR_TEST] DATA TRANSFER START" in buffer:
                        print("✅ Data Transfer Detected!")
                        start_idx = buffer.find(b"[ISR_TEST] DATA TRANSFER START") + len(b"[ISR_TEST] DATA TRANSFER START") + 2
                        buffer = buffer[start_idx:]
                        break
                except Exception as e:
                    print(f"Error reading serial: {e}")
            time.sleep(0.01)
        else:
            print("❌ Timeout waiting for DATA TRANSFER START!")
            return

        print("\n📥 2. CAPTURING RAW DATA...")
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"analiz/raw_dump_{timestamp}.csv"
        os.makedirs("analiz", exist_ok=True)

        packets_received = 0
        total_expected = 900

        with open(filename, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            header = ["FrameID", "Timestamp", "Count"] + [f"S{i}" for i in range(688)]
            writer.writerow(header)

            pbar = tqdm(total=total_expected, unit="frame", desc="Downloading")

            while packets_received < 900:
                if ser.in_waiting > 0:
                    new_data = ser.read(ser.in_waiting)
                    buffer += new_data

                while True:
                    if len(buffer) < 4:
                        break

                    if buffer.startswith(b"<DTA"):
                        if len(buffer) < 20:
                            break
                        count = struct.unpack('<H', buffer[16:18])[0]
                        expected_len = 20 + (count * 2) + 5

                        if len(buffer) >= expected_len:
                            packet = buffer[:expected_len]
                            if packet[-5:] != b"<END>":
                                buffer = buffer[1:]
                                continue

                            buffer = buffer[expected_len:]
                            meta = struct.unpack('<I I I', packet[4:16])
                            frame_id, ts = meta[0], meta[1]
                            samples = struct.unpack(f'<{count}h', packet[20:20 + count*2])

                            row = [frame_id, ts, count] + list(samples)
                            writer.writerow(row)
                            packets_received += 1
                            pbar.update(1)
                        else:
                            break

                    elif b"[ISR_TEST] DATA TRANSFER COMPLETE" in buffer:
                        print("\n✅ Transfer Complete Signal Received!")
                        break
                    else:
                        if b"<DTA" in buffer:
                            start = buffer.find(b"<DTA")
                            buffer = buffer[start:]
                        elif b"\n" in buffer:
                            nl = buffer.find(b"\n")
                            buffer = buffer[nl+1:]
                        else:
                            break

                if b"[ISR_TEST] DATA TRANSFER COMPLETE" in buffer or b"TRANSFER COMPLETE" in buffer:
                    break
                time.sleep(0.001)

        pbar.close()
        print(f"\n💾 Saved {packets_received} frames to {filename}")

        print("\n📊 3. MONITORING MULTI-STRIDE TEST RESULTS (Ctrl+C to stop)...")
        print("="*60)
        try:
            while True:
                try:
                    if ser.in_waiting > 0:
                        line = ser.readline().decode('utf-8', errors='replace').strip()
                        if line:
                            if "TEST_S" in line:
                                print(f"\033[92m{line}\033[0m")
                            elif "TEST COMPLETE" in line:
                                print(f"\n✅ {line}")
                                break
                            else:
                                print(line)
                except (serial.SerialException, OSError) as e:
                    print(f"\n⚠️ Serial connection lost: {e}")
                    break
        except KeyboardInterrupt:
            pass
        print("="*60)

    finally:
        print("▶ Resuming background serial receiver...")
        fallower_system.serial_interface.resume_reading()


# =============================================================================
# 3. UTILITIES & HELPER COMMANDS
# =============================================================================
def parse_register_values(register_str):
    registers = {}
    try:
        if register_str.startswith("<STR>") and register_str.endswith(">"):
            cleaned = register_str.strip()
            pattern = r"<R(\d+)>0x([0-9A-Fa-f]+)"
            matches = re.findall(pattern, cleaned)
            for reg_num_str, value_str in matches:
                registers[int(reg_num_str)] = int(value_str, 16)
            registers["_raw_format"] = cleaned
            return registers
        elif "{<STR>" in register_str and register_str.endswith(">}"):
            cleaned = register_str.replace("{", "").replace("}", "").strip()
            pattern = r"<R(\d+)>0x([0-9A-Fa-f]+)"
            matches = re.findall(pattern, cleaned)
            for reg_num_str, value_str in matches:
                registers[int(reg_num_str)] = int(value_str, 16)
            registers["_raw_format"] = cleaned
            return registers
        elif register_str.startswith('{') and register_str.endswith('}') and not ("<STR>" in register_str):
            import json
            try:
                reg_dict = json.loads(register_str)
                for key, value in reg_dict.items():
                    if isinstance(value, str) and value.startswith('0x'):
                        registers[int(key)] = int(value, 16)
                    else:
                        registers[int(key)] = int(value)
            except json.JSONDecodeError:
                pass
        else:
            pairs = register_str.split(',')
            for pair in pairs:
                if ':' in pair:
                    reg_num, value = pair.split(':', 1)
                    reg_num = int(reg_num.strip())
                    value = value.strip()
                    if value.startswith('0x'):
                        reg_value = int(value, 16)
                    elif value.startswith('0X'):
                        reg_value = int(value, 16)
                    else:
                        reg_value = int(value)
                    registers[reg_num] = reg_value
        return registers
    except Exception as e:
        print(f"Hata: Register değerleri ayrıştırılamadı: {e}")
        return None


def add_monitor_command(fallower_system):
    if not fallower_system.serial_interface:
        print("Seri arayüz başlatılmamış.")
        return
    print("Seri port izleme modu başlatıldı. Çıkmak için Ctrl+C kullanın...")
    try:
        original_process_buffer = fallower_system.serial_interface._process_buffer
        def monitor_process_buffer():
            with fallower_system.serial_interface._lock:
                buffer_copy = fallower_system.serial_interface.data_buffer.copy()
            if buffer_copy:
                try:
                    text = buffer_copy.decode('utf-8', errors='replace')
                    if text.strip():
                        print(f"BUFFER: {text}")
                except Exception:
                    pass
            original_process_buffer()
        fallower_system.serial_interface._process_buffer = monitor_process_buffer
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\nSeri port izleme modu sonlandırıldı.")
    finally:
        fallower_system.serial_interface._process_buffer = original_process_buffer


def handle_prediction_result(event: Event):
    global prediction_counter, prediction_display_enabled
    if event.event_type == EventType.PREDICTION_READY:
        prediction = event.data
        if prediction:
            logger.info(f"PREDICTION #{prediction_counter}: {prediction.label} ({prediction.confidence*100:.2f}%)")


def setup_prediction_monitoring(event_bus):
    event_bus.subscribe(EventType.PREDICTION_READY, handle_prediction_result)
    print("✅ Tahmin izleme aktif")


def show_available_models(fallower_system):
    if not fallower_system.model_predictor:
        print("❌ Model predictor not initialized!")
        return
    available_models = fallower_system.model_predictor.get_available_models()
    current_model_info = fallower_system.model_predictor.get_current_model_info()
    print("\n🤖 Available AI Models:")
    print("=" * 60)
    for key, name in available_models.items():
        model_config = fallower_system.config.get_available_models()[key]
        status = "✅ CURRENT" if key == fallower_system.config.current_model_key else "  "
        print(f"{status} [{key}] {name}")
        print(f"      Pipeline: {model_config.pipeline.value}")
        print(f"      File: {model_config.filename}")
    print(f"\nCurrent: {current_model_info.get('name', 'Unknown')}")
    print(f"  Loaded: {'Yes' if current_model_info.get('loaded') else 'No'}")


def change_model_command(fallower_system, model_key):
    if not fallower_system.model_predictor:
        print("❌ Model predictor not initialized!")
        return False
    available_models = fallower_system.model_predictor.get_available_models()
    if model_key not in available_models:
        print(f"❌ Model '{model_key}' not found!")
        return False
    print(f"🔄 Changing to model: {available_models[model_key]}...")
    if fallower_system.model_predictor.change_model(model_key):
        if fallower_system.data_processor:
            fallower_system.data_processor.update_model_configuration()
        print(f"✅ Successfully changed to model: {model_key}")
        return True
    else:
        print(f"❌ Failed to change to model: {model_key}")
        return False


def print_command_help():
    """Kullanılabilir komutları modern ve kategorize biçimde gösterir."""
    print("\n" + "═" * 70)
    print(" ⚡ FALLOWER RADAR & AI GELİŞTİRİCİ KONTROL PANELİ")
    print("═" * 70)
    print("\n🖥️  STÜDYO & OPERATÖR ARAYÜZLERİ (GUI):")
    print("  operator / collector / 4 - Modern Radar Veri Toplama Stüdyosu (GUI)")
    print("  viewer / view            - Senkron Veri İnceleme & Etiketleme Arayüzü (GUI)")
    print("\n📡 VERİ TOPLAMA:")
    print("  collect <s>              - Ham radar verisi topla ve NPZ/CSV kaydet (örn: collect 30)")
    print("\n🤖 AI MODEL YÖNETİMİ & TRANSFER:")
    print("  models                   - Mevcut AI modellerini listele")
    print("  model <key>              - AI modelini değiştir (örn: model raw)")
    print("  model_info               - Aktif model parametrelerini ve durumunu göster")
    print("  load_model_binary        - AI modelini doğrudan SDRAM'e yükle (Firehose)")
    print("  load_model_flash         - Modeli SDRAM'den NAND flash'a kaydet")
    print("  read_model_nand          - Modeli NAND flash'tan SDRAM'e oku")
    print("\n⚡ EMBEDDED & GERÇEK ZAMANLI INFERENCE:")
    print("  init / 1                 - Embedded sistemi başlat (INIT)")
    print("  ready / 2                - Embedded sistemi hazır duruma getir (READY)")
    print("  calibrate / 3            - Radar sensörünü kalibre et (CALIBRATE)")
    print("  ai_init                  - Embedded AI altyapısını başlat (AI_INIT)")
    print("  ai_inference             - AI inference sistemini aktive et (AI_INFERENCE)")
    print("  run                      - AI modelini test verisi ile çalıştır")
    print("  run_inference_isr_real   - Real-time Stride-5 Modu")
    print("  stop_inference           - Real-time AI inference'ı durdur")
    print("  inference_status         - Real-time inference durumunu sorgula")
    print("  registers <STR>          - Radar register değerlerini yapılandır")
    print("\n▶️  AKIŞ KONTROLÜ:")
    print("  start                    - Veri akışını başlat (Normal mod)")
    print("  start_debug              - Veri akışını başlat + 30s otomatik debug")
    print("  start_quiet              - Veri akışını başlat (Sessiz mod)")
    print("  start_normal             - Normal tahmin görüntüleme moduna geç")
    print("  stop                     - Veri akışını durdur (STOP)")
    print("  monitor                  - Seri porttan gelen ham verileri izle")
    print("  status                   - Sistem durumunu göster")
    print("\n🔍 DEBUG & TEŞHİS:")
    print("  debug_serial             - Seri port tampon ve durum kontrolü")
    print("  debug_eventbus           - EventBus kuyruk ve abone durumu")
    print("  debug_storage            - Veri depolama alt sistemi kontrolü")
    print("  debug_flow               - Uçtan uca sistem veri akışı analizi")
    print("  debug_model_data         - 30 saniye model veri akış analizi")
    print("  debug_quick              - Son 5 frame ve pencere durumu")
    print("  debug_file               - Dosya sistemi kontrolü")
    print("  exit                     - Güvenli kapat ve çık")
    print("  help                     - Bu yardım menüsünü göster")
    print("═" * 70 + "\n")


# =============================================================================
# 4. COLLECT RAW DATA  —  RADAR & KAMERA SENKRON TOPLAMA
# =============================================================================
def collect_raw_data(fallower_system, duration_seconds: int):
    import struct
    import numpy as np
    from datetime import datetime
    import cv2
    import queue as cam_queue_module

    if not fallower_system.serial_interface:
        print("Serial interface başlatılmamış!")
        return

    output_dir = fallower_system.config.get('system', 'output_dir', 'output_data')
    os.makedirs(output_dir, exist_ok=True)

    timestamp_str = datetime.now().strftime('%Y%m%d_%H%M%S')
    output_file = os.path.join(output_dir, f"raw_collect_{timestamp_str}.npz")
    csv_file = os.path.join(output_dir, f"raw_collect_{timestamp_str}.csv")

    collected_frames = []
    collected_timestamps = []
    collected_frame_ids = []
    collected_stm32_ts = []
    collected_durations = []
    collected_sample_counts = []
    camera_frames_log = []
    camera_timestamps_log = []

    frame_count = [0]
    collection_active = [True]
    last_record_tick = [0.0]

    camera_queue = cam_queue_module.Queue(maxsize=5)
    camera_running = threading.Event()
    camera_running.set()

    def camera_capture_thread():
        cap = cv2.VideoCapture(0)
        if not cap.isOpened():
            print("⚠️ Kamera açılamadı!")
            return
        cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
        while camera_running.is_set():
            ret, frame = cap.read()
            if ret:
                cam_timestamp = time.time()
                if camera_queue.full():
                    try: camera_queue.get_nowait()
                    except: pass
                camera_queue.put_nowait((cam_timestamp, frame))
            time.sleep(0.001)
        cap.release()

    cam_thread = threading.Thread(target=camera_capture_thread, daemon=True)
    cam_thread.start()

    def on_raw_frame(event):
        if not collection_active[0]:
            return
        try:
            data = event.data
            raw_data = data.get("raw_data", b"")
            frame_number = data.get("frame_number", 0)
            stm32_timestamp = data.get("timestamp", 0)
            radar_pc_timestamp = time.time()

            if radar_pc_timestamp - last_record_tick[0] < RADAR_PERIOD_S:
                return
            last_record_tick[0] = radar_pc_timestamp

            cam_frame = None
            cam_ts = None
            if not camera_queue.empty():
                while not camera_queue.empty():
                    try:
                        cam_data = camera_queue.get_nowait()
                        cam_ts, cam_frame = cam_data if isinstance(cam_data, tuple) else (cam_data, None)
                    except:
                        break

            if cam_ts is not None:
                delta_ms = (radar_pc_timestamp - cam_ts) * 1000.0
                print(f"\r[SENK] STM32:{stm32_timestamp:>8}ms | "
                      f"RadarPC:{radar_pc_timestamp:.6f} | "
                      f"KameraPC:{cam_ts:.6f} | "
                      f"Delta:{delta_ms:+.2f}ms  |  Frame#{frame_count[0]}     ", end="", flush=True)
                camera_timestamps_log.append(cam_ts)
                if cam_frame is not None:
                    camera_frames_log.append(cam_frame)
            else:
                print(f"\r[SENK] STM32:{stm32_timestamp:>8}ms | "
                      f"RadarPC:{radar_pc_timestamp:.6f} | "
                      f"Kamera: BEKLENIYOR...  |  Frame#{frame_count[0]}     ", end="", flush=True)

            if len(raw_data) >= 16:
                meta = struct.unpack('<IIIHH', raw_data[:16])
                _, _, duration, sample_count, _ = meta
                samples_data = raw_data[16:]
            elif len(raw_data) >= 14:
                meta = struct.unpack('<IIIH', raw_data[:12])
                _, _, duration, sample_count = meta
                samples_data = raw_data[12:]
            else:
                return

            num_samples = len(samples_data) // 2
            if num_samples == 0:
                return

            adc_values = np.frombuffer(samples_data, dtype=np.int16).copy()

            collected_frames.append(adc_values)
            collected_timestamps.append(radar_pc_timestamp)
            collected_frame_ids.append(frame_number)
            collected_stm32_ts.append(stm32_timestamp)
            collected_durations.append(duration)
            collected_sample_counts.append(sample_count)
            frame_count[0] += 1
        except Exception:
            pass

    event_bus = fallower_system.event_bus
    event_bus.subscribe(EventType.RAW_FRAME_DATA, on_raw_frame, high_priority=False)

    print(f"📡 Ham veri toplama başlıyor ({duration_seconds} saniye, periyot: {RADAR_PERIOD_MS}ms)...")
    print(f"📁 Çıktı dosyası: {output_file}")
    print("-" * 60)

    if not fallower_system.serial_interface.send_command("COLLECT"):
        print("COLLECT komutu gönderilemedi!")
        event_bus.unsubscribe(EventType.RAW_FRAME_DATA, on_raw_frame)
        camera_running.clear()
        return

    fallower_system.serial_interface._streaming_active = True
    fallower_system.serial_interface._system_state = SystemStatus.STREAMING

    start_time = time.time()
    try:
        while time.time() - start_time < duration_seconds:
            elapsed = time.time() - start_time
            remaining = duration_seconds - elapsed
            fps = frame_count[0] / elapsed if elapsed > 0 else 0
            print(f"\r  Süre: {elapsed:.1f}/{duration_seconds}s | Frame: {frame_count[0]} | FPS: {fps:.1f} | Kalan: {remaining:.1f}s  ", end="", flush=True)
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\n\nKullanıcı tarafından durduruldu (Ctrl+C)")

    collection_active[0] = False
    print()

    event_bus.unsubscribe(EventType.RAW_FRAME_DATA, on_raw_frame)
    si = fallower_system.serial_interface

    si._streaming_active = False
    si._system_state = SystemStatus.READY
    si.send_command("STOP")
    time.sleep(1.0)

    si.pause_reading()
    time.sleep(0.1)
    si.data_buffer.clear()
    with si._lock:
        if si.ser and si.ser.is_open:
            si.ser.reset_input_buffer()
            try:
                while si.ser.in_waiting > 0:
                    si.ser.read(si.ser.in_waiting)
            except Exception:
                pass
    si.data_buffer.clear()
    si.resume_reading()

    # Kamera thread durdur
    camera_running.clear()
    cam_thread.join(timeout=1.0)
    print("\n⏹️ Kamera thread durduruldu.")

    total_frames = len(collected_frames)
    if total_frames == 0:
        print("Hiç frame toplanamadı!")
        return

    print(f"\n💾 {total_frames} frame kaydediliyor...")

    max_len = max(len(f) for f in collected_frames)
    min_len = min(len(f) for f in collected_frames)

    if max_len == min_len:
        frames_array = np.array(collected_frames, dtype=np.int16)
    else:
        frames_array = np.zeros((total_frames, max_len), dtype=np.int16)
        for i, f in enumerate(collected_frames):
            frames_array[i, :len(f)] = f

    video_file = os.path.join(output_dir, f"raw_collect_{timestamp_str}.avi")
    if camera_frames_log:
        h, w = camera_frames_log[0].shape[:2]
        fourcc = cv2.VideoWriter_fourcc(*'MJPG')
        vw = cv2.VideoWriter(video_file, fourcc, 1000.0/RADAR_PERIOD_MS, (w, h))
        for frm in camera_frames_log[:total_frames]:
            vw.write(frm)
        vw.release()
        print(f"🎬 Video kaydedildi: {video_file} ({len(camera_frames_log[:total_frames])} frame)")

    np.savez_compressed(
        output_file,
        frames=frames_array,
        timestamps=np.array(collected_timestamps, dtype=np.float64),
        frame_ids=np.array(collected_frame_ids, dtype=np.uint32),
        stm32_timestamps=np.array(collected_stm32_ts, dtype=np.uint32),
        durations=np.array(collected_durations, dtype=np.uint32),
        sample_counts=np.array(collected_sample_counts, dtype=np.uint16),
        frame_sample_lengths=np.array([len(f) for f in collected_frames], dtype=np.uint16),
        camera_timestamps=np.array(camera_timestamps_log[:total_frames], dtype=np.float64) if camera_timestamps_log else np.array([]),
        radar_period_ms=RADAR_PERIOD_MS
    )

    with open(csv_file, 'w') as f:
        f.write("frame_idx,frame_id,stm32_timestamp_ms,duration_us,sample_count,pc_timestamp\n")
        for i in range(total_frames):
            pc_ts = datetime.fromtimestamp(collected_timestamps[i]).strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
            f.write(f"{i},{collected_frame_ids[i]},{collected_stm32_ts[i]},{collected_durations[i]},{collected_sample_counts[i]},{pc_ts}\n")

    elapsed_total = collected_timestamps[-1] - collected_timestamps[0] if total_frames > 1 else 0
    avg_fps = (total_frames - 1) / elapsed_total if elapsed_total > 0 else 0
    file_size_kb = os.path.getsize(output_file) / 1024

    print("=" * 60)
    print(f"  Toplanan frame sayısı : {total_frames}")
    print(f"  Frame boyutu (sample) : {min_len}" + (f" - {max_len}" if max_len != min_len else ""))
    print(f"  Gerçek süre           : {elapsed_total:.2f} saniye")
    print(f"  Ortalama FPS          : {avg_fps:.1f}")
    print(f"  Periyot               : {RADAR_PERIOD_MS}ms")
    print(f"  Binary dosya (.npz)   : {output_file} ({file_size_kb:.1f} KB)")
    print(f"  Metadata dosya (.csv) : {csv_file}")
    if camera_frames_log:
        print(f"  Video dosyası (.avi)  : {video_file}")
    print("=" * 60)
    print("💡 Veriyi yüklemek için: data = np.load('dosya.npz'); frames = data['frames']")


# =============================================================================
# 5. MAIN ENTRY POINT & CLI
# =============================================================================
if __name__ == "__main__":
    config = Configuration()
    logger = config.get_logger()

    logger.info("========================================")
    logger.info(" Fallower Real-Time Fall Detection System")
    logger.info(f" Radar Periyot: {RADAR_PERIOD_MS}ms")
    logger.info("========================================")
    
    current_model = config.get_current_model()
    logger.info(f"Current model: {current_model.name}")
    logger.info(f"Pipeline: {current_model.pipeline.value}")
    logger.info(f"Serial port: {config.get('serial','port')}")

    fallower_system = FallowerSystem()
    exit_code = 0

    try:
        logger.info("Initializing system...")
        if not fallower_system.initialize():
            logger.critical("System initialization failed. Exiting.")
            sys.exit(1)

        logger.info("System is READY.")
        setup_prediction_monitoring(fallower_system.event_bus)
        
        
        
        launch_collector(fallower_system)

        if fallower_system.status == SystemStatus.ERROR:
            logger.error("System entered ERROR state.")
            exit_code = 1

    except KeyboardInterrupt:
        logger.info("Shutdown by user.")
    except Exception as e:
        logger.critical(f"Critical error: {e}", exc_info=True)
        exit_code = 1
        if fallower_system and fallower_system.status != SystemStatus.SHUTDOWN:
            fallower_system.shutdown(graceful=False)
    finally:
        if fallower_system and fallower_system.status != SystemStatus.SHUTDOWN:
            fallower_system.shutdown(graceful=True)
        logger.info(f"Application finished with exit code {exit_code}.")
        sys.exit(exit_code)
