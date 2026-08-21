"""
=============================================================================
FALLOWER — Radar Veri Toplama Operatörü (Collector Pro)
=============================================================================
- Modern Beyaz / Aydınlık Tema (Clean Modern Light Theme)
- ADC Master / Kamera Slave Yüksek Hassasiyetli Senkron Veri Kayıt Motoru
- Çift Modlu Canlı Sinyal Analizi: Zaman Bölgesi Dalga Formu & FFT Spektrumu
- Titremesiz (Flicker-Free) Sağ Üst Köşe İmleç Göstergesi
- 30-60 FPS Optimize GUI Döngüsü ve Düşük CPU Tüketimi
- Hatasız Metadata.json Üretimi & veritoplama_goruntuleyici ile %100 Uyumluluk
=============================================================================
"""

import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import cv2
import numpy as np
import os
import json
import threading
import time
from datetime import datetime
from PIL import Image, ImageTk
import queue
import struct
import re

# Windows High-DPI Desteği
try:
    import ctypes
    ctypes.windll.shcore.SetProcessDpiAwareness(1)
except Exception:
    try:
        import ctypes
        ctypes.windll.user32.SetProcessDPIAware()
    except Exception:
        pass

# ---------------------------------------------------------------------------
# EVENTBUS ENTEGRASYONU
# ---------------------------------------------------------------------------
try:
    from core.event_bus import EventBus, Event
    from core.enums import EventType, SystemStatus
    HAS_CORE = True
except ImportError:
    HAS_CORE = False
    print("[WARN] core modulleri bulunamadi. Sadece MOCK/MANUEL test modu aktif.")


# =============================================================================
# GLOBAL SABİTLER  —  RADAR PERİYODU (ADC MASTER)
# =============================================================================
DEFAULT_RADAR_FPS = 20.0
DEFAULT_VIDEO_FPS = 20.0
GUI_UPDATE_INTERVAL_MS = 30  # ~33 FPS Akıcı UI Render Döngüsü (CPU dostu)

# =============================================================================
# RADAR DONANIM REGISTER HARİTASI (59 Registers: 0x00 - 0x3A)
# =============================================================================
DEFAULT_RADAR_REGISTERS = {
    0: 0xFF, 1: 0xFD, 2: 0xFF, 3: 0xC0, 4: 0xC1,
    5: 0x07,  # reg05 (0x05) - gain_control_1
    6: 0x01,  # reg06 (0x06) - gain_control_2
    7: 0x0F,  # reg07 (0x07)
    8: 0x80, 9: 0x00,  # reg08, reg09
    10: 0x01, 11: 0x20, 12: 0x20, 13: 0xE7, 14: 0x00, 15: 0x01,  # R08-R15
    16: 0x00, 17: 0x01, 18: 0x8A, 19: 0x00, 20: 0x03, 21: 0x06, 22: 0x02, 23: 0x63,
    24: 0x63, 25: 0x00, 26: 0x00, 27: 0x00, 28: 0x63, 29: 0x00, 30: 0x63, 31: 0x03,
    32: 0x00,  # reg32 (0x20)
    33: 0x7A,  # reg33 (0x21) - start_bin (0 noktası: 0x7A = 122)
    34: 0x1A,  # reg34 (0x22) - step_bin (start + step = target, varsayılan 0x1A = 26 bin)
    35: 0xFF,  # reg35 (0x23) - integration
    36: 0x01,  # reg36 (0x24)
    37: 0x00,  # reg37 (0x25) - resolution
    38: 0x00, 39: 0x00,
    40: 0x00, 41: 0x00, 42: 0x00, 43: 0x00, 44: 0x00, 45: 0x00, 46: 0x00, 47: 0x40,
    48: 0x40, 49: 0x10, 50: 0x04, 51: 0x04, 52: 0x01, 53: 0x01, 54: 0x01, 55: 0x00,
    56: 0x00, 57: 0xC0, 58: 0x00
}

# Modern Aydınlık / Beyaz Renk Paleti (Clean White / Light Slate)
PALETTE = {
    "bg_main": "#f4f6f8",       # Ana Arka Plan (Soft Slate White)
    "bg_card": "#ffffff",       # Kart / Panel Arka Planı (Pure White)
    "bg_panel": "#f8f9fa",      # Alt Kart / Liste Arka Planı (Subtle Gray)
    "bg_canvas": "#ffffff",     # Çizim Alanı (Beyaz)
    "border": "#dee2e6",        # Kenarlık / Çizgi (Border Gray)
    "border_focus": "#1971c2",  # Odaklanmış Kenarlık
    "fg_text": "#212529",        # Ana Metin (Koyu Grafit)
    "fg_muted": "#495057",       # İkincil Metin (Orta Gri)
    "fg_dim": "#868e96",         # Soluk Metin (Açık Gri)
    "accent_blue": "#1971c2",    # Vurgu Mavi (Modern Blue)
    "accent_cyan": "#0c8599",    # Vurgu Camgöbeği (Teal)
    "accent_green": "#2b8a3e",   # Başarı / Başlangıç (Emerald Green)
    "accent_red": "#d6336c",     # Hata / Bitiş (Crimson / Rose)
    "accent_yellow": "#e67700",  # Uyarı / Zaman Çizelgesi (Amber)
    "accent_purple": "#7048e8",  # FFT / Vurgu (Violet)
    "btn_bg": "#e9ecef",         # Standart Buton
    "btn_hover": "#dee2e6",      # Buton Üzerine Gelince
}


# =============================================================================
# YARDIMCI WIDGET: Modern Hover Buton
# =============================================================================
class ModernButton(tk.Button):
    def __init__(self, master, text="", command=None, bg_color=None, fg_color=None,
                 hover_bg=None, font=("Segoe UI", 9, "bold"), padx=10, pady=4, **kwargs):
        self.normal_bg = bg_color or PALETTE["btn_bg"]
        self.normal_fg = fg_color or PALETTE["fg_text"]
        self.hover_bg = hover_bg or PALETTE["btn_hover"]

        super().__init__(
            master, text=text, command=command, bg=self.normal_bg, fg=self.normal_fg,
            activebackground=self.hover_bg, activeforeground=self.normal_fg,
            relief=tk.FLAT, bd=0, padx=padx, pady=pady, font=font,
            cursor="hand2", **kwargs
        )
        self.bind("<Enter>", self._on_enter)
        self.bind("<Leave>", self._on_leave)

    def _on_enter(self, e):
        if self["state"] != tk.DISABLED:
            self.configure(bg=self.hover_bg)

    def _on_leave(self, e):
        if self["state"] != tk.DISABLED:
            self.configure(bg=self.normal_bg)

    def set_color(self, bg, fg, hover_bg=None):
        self.normal_bg = bg
        self.normal_fg = fg
        self.hover_bg = hover_bg or bg
        self.configure(bg=self.normal_bg, fg=self.normal_fg,
                       activebackground=self.hover_bg, activeforeground=self.normal_fg)


# =============================================================================
# 1. VERİ SETİ YÖNETİMİ
# =============================================================================
class DatasetManager:
    def __init__(self, base_dir="FALLOWER DATASETS"):
        self.base_dir = base_dir
        os.makedirs(base_dir, exist_ok=True)

    def create_dataset(self, name, room, radar_position, notes=""):
        dataset_path = os.path.join(self.base_dir, name)
        if os.path.exists(dataset_path):
            return False, "Bu isimde bir veri seti zaten var!"

        os.makedirs(dataset_path, exist_ok=True)

        meta = {
            "name": name,
            "created_at": datetime.now().isoformat(),
            "room": room,
            "radar_position": radar_position,
            "notes": notes,
            "record_count": 0,
            "total_duration_seconds": 0.0,
            "default_radar_fps": DEFAULT_RADAR_FPS,
            "default_video_fps": DEFAULT_VIDEO_FPS
        }

        with open(os.path.join(dataset_path, "metadata.json"), "w", encoding="utf-8") as f:
            json.dump(meta, f, ensure_ascii=False, indent=2)

        return True, dataset_path

    def get_datasets(self):
        datasets = []
        if not os.path.exists(self.base_dir):
            return datasets
        for name in sorted(os.listdir(self.base_dir)):
            path = os.path.join(self.base_dir, name)
            if os.path.isdir(path):
                meta_path = os.path.join(path, "metadata.json")
                if os.path.exists(meta_path):
                    try:
                        with open(meta_path, "r", encoding="utf-8") as f:
                            ds_info = json.load(f)
                            ds_info["name"] = name
                            datasets.append(ds_info)
                    except Exception:
                        datasets.append({"name": name, "room": "?", "radar_position": "?", "notes": "", "record_count": 0})
                else:
                    datasets.append({"name": name, "room": "?", "radar_position": "?", "notes": "", "record_count": 0})
        return datasets

    def get_dataset_path(self, name):
        return os.path.join(self.base_dir, name)

    def get_next_record_index(self, dataset_name):
        ds_path = self.get_dataset_path(dataset_name)
        if not os.path.exists(ds_path):
            return 1
        records = [d for d in os.listdir(ds_path)
                   if os.path.isdir(os.path.join(ds_path, d)) and d != "labels"]
        max_idx = 0
        for r in records:
            parts = r.split("_", 1)
            if len(parts) > 0 and parts[0].isdigit():
                max_idx = max(max_idx, int(parts[0]))
        return max_idx + 1

    def sanitize_folder_name(self, name):
        name = name.replace(" ", "_")
        tr_map = {
            'ç': 'c', 'Ç': 'C', 'ğ': 'g', 'Ğ': 'G', 'ı': 'i', 'İ': 'I',
            'ö': 'o', 'Ö': 'O', 'ş': 's', 'Ş': 'S', 'ü': 'u', 'Ü': 'U'
        }
        for tr_char, eng_char in tr_map.items():
            name = name.replace(tr_char, eng_char)
        clean = re.sub(r'[^a-zA-Z0-9_\-]', '', name)
        return clean.strip("_") if clean else "kayit"


# =============================================================================
# 2. EVENTBUS'TAN ADC VERİ AKIŞI ALICI
# =============================================================================
class ADCEventSource:
    def __init__(self, event_bus):
        self.event_bus = event_bus
        self.frame_queue = queue.Queue(maxsize=100)
        self._callback = self._on_raw_frame
        self._subscribed = False
        self.frame_count = 0
        self.last_duration_us = 0
        self.live_fps = 0.0
        self._last_frame_time = None
        self._fps_history = []
        self._duration_history = []

    def start(self):
        if self.event_bus is None:
            return False
        self.event_bus.subscribe(EventType.RAW_FRAME_DATA, self._callback)
        self._subscribed = True
        print("[OK] ADCEventSource: EventBus'a abone olundu.")
        return True

    def stop(self):
        if self._subscribed and self.event_bus is not None:
            self.event_bus.unsubscribe(EventType.RAW_FRAME_DATA, self._callback)
            self._subscribed = False
            while not self.frame_queue.empty():
                try:
                    self.frame_queue.get_nowait()
                except queue.Empty:
                    break
            print("[OK] ADCEventSource: Abonelik kaldirildi.")

    def _on_raw_frame(self, event):
        try:
            data = event.data
            raw_data = data.get("raw_data", b"")
            if len(raw_data) == 0:
                return
            if len(raw_data) >= 16:
                meta = struct.unpack('<IIIHH', raw_data[:16])
                frame_number, stm32_timestamp, duration, sample_count, _ = meta
                samples_data = raw_data[16:]
            elif len(raw_data) >= 14:
                meta = struct.unpack('<IIIH', raw_data[:12])
                frame_number, stm32_timestamp, duration, sample_count = meta
                samples_data = raw_data[12:]
            else:
                return
            adc_values = np.frombuffer(samples_data, dtype=np.int16).copy()
            adc_values = adc_values - np.mean(adc_values)
            if len(adc_values) == 0:
                return

            now = time.time()
            # İki frame arasındaki gerçek süre (Frame Delta) üzerinden FPS hesabı
            if hasattr(self, "_last_stm32_ts") and self._last_stm32_ts is not None and stm32_timestamp > self._last_stm32_ts:
                dt_s = (stm32_timestamp - self._last_stm32_ts) / 1000.0
                inst_fps = 1.0 / dt_s if dt_s > 0.001 else 20.0
            elif self._last_frame_time is not None:
                dt_s = now - self._last_frame_time
                inst_fps = 1.0 / dt_s if dt_s > 0.001 else 20.0
            else:
                inst_fps = 20.0

            self._last_stm32_ts = stm32_timestamp
            self._last_frame_time = now
            self.last_duration_us = duration

            self._fps_history.append(inst_fps)
            if len(self._fps_history) > 30:
                self._fps_history.pop(0)
            self.live_fps = float(np.mean(self._fps_history))

            if self.frame_queue.full():
                try:
                    self.frame_queue.get_nowait()
                except queue.Empty:
                    pass
            self.frame_queue.put_nowait({
                "adc": adc_values,
                "stm32_ts": stm32_timestamp,
                "duration_us": duration,
                "pc_ts": now,
                "frame_num": frame_number,
                "live_fps": self.live_fps
            })
            self.frame_count += 1
        except Exception:
            pass

    def get_all_frames(self):
        frames = []
        while not self.frame_queue.empty():
            try:
                frames.append(self.frame_queue.get_nowait())
            except queue.Empty:
                break
        return frames

    def get_live_fps(self):
        return self.live_fps

    def get_avg_duration_us(self):
        if self._duration_history:
            return float(np.mean(self._duration_history))
        return 0.0


# =============================================================================
# 3. KAMERA YAKALAMA (DURATION & GERÇEK FPS HESAPLAYICILI)
# =============================================================================
class CameraCapture:
    def __init__(self, source=1):
        self.source = source
        self.cap = None
        self.running = False
        self.frame = None
        self.lock = threading.Lock()
        self.thread = None
        self.frame_counter = 0
        self.live_fps = 0.0
        self._last_frame_time = None
        self._fps_history = []

    def start(self):
        try:
            self.cap = cv2.VideoCapture(self.source)
            if not self.cap.isOpened():
                return False
            self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
            self.running = True
            self.thread = threading.Thread(target=self._capture, daemon=True)
            self.thread.start()
            return True
        except Exception:
            return False

    def stop(self):
        self.running = False
        if self.thread and self.thread.is_alive():
            self.thread.join(timeout=0.5)
        if self.cap:
            try:
                self.cap.release()
            except Exception:
                pass
            self.cap = None

    def _capture(self):
        while self.running:
            if self.cap and self.cap.isOpened():
                ret, frame = self.cap.read()
                if ret:
                    now = time.time()
                    if self._last_frame_time is not None:
                        dt = now - self._last_frame_time
                        if dt > 0:
                            inst_fps = 1.0 / dt
                            self._fps_history.append(inst_fps)
                            if len(self._fps_history) > 30:
                                self._fps_history.pop(0)
                            self.live_fps = float(np.mean(self._fps_history))
                    self._last_frame_time = now

                    with self.lock:
                        self.frame = frame
                        self.frame_counter += 1
            time.sleep(0.003)

    def get_frame(self):
        with self.lock:
            return self.frame.copy() if self.frame is not None else None

    def get_live_fps(self):
        return self.live_fps

    def get_size(self):
        if self.cap and self.cap.isOpened():
            w = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
            h = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
            if w > 0 and h > 0:
                return (w, h)
        return (640, 480)


# =============================================================================
# 4. ANA OPERATÖR UYGULAMASI (DATA COLLECTOR PRO)
# =============================================================================
class DataCollectorApp:
    def __init__(self, root, fallower_system=None):
        self.root = root
        self.video_fps = DEFAULT_VIDEO_FPS
        self.radar_fps = DEFAULT_RADAR_FPS

        self.root.title("⚡ FALLOWER Radar Veri Toplama Stüdyosu")
        self.root.geometry("1400x920")
        self.root.minsize(1100, 750)
        self.root.configure(bg=PALETTE["bg_main"])

        self.dataset_manager = DatasetManager()
        self.camera = CameraCapture()
        self.fallower_system = fallower_system

        self.adc_source = None
        if fallower_system is not None and hasattr(fallower_system, "event_bus"):
            self.adc_source = ADCEventSource(fallower_system.event_bus)
        else:
            print("[INFO] FallowerSystem bagli degil. ADC canli goruntu pasif (Mock Mode).")

        self.current_dataset = None
        self.recording = False
        self.camera_mock = False

        # Görselleştirme Modları
        self.view_mode = "waveform"     # "waveform" veya "mti"
        self.y_scale_mode = "auto"      # "auto", "fixed_25k", "fixed_10k"
        self._last_canvas_w = 0
        self._last_canvas_h = 0
        self._live_adc_history = []

        # Kayıt Buffer'ları
        self.adc_buffer = []
        self.adc_meta_buffer = []
        self.camera_ts_buffer = []
        self.video_writer = None
        self.current_record_dir = None
        self.record_start_time = None
        self.stm32_ref_ts = None
        self.record_index = 0
        self.record_desc = ""
        self.safe_desc = ""

        # Radar Konfigürasyon Durumu (15m = 128 dec)
        self._radar_configuring = False
        self.active_reg34 = 0x1A  # Aktif donanım reg34 (0x22) değeri (26 dec ≈ 3.05m)
        self.active_range_m = 3.05

        # Son gelen ADC frame'i
        self.last_adc_item = None

        self._build_ui()
        self._bind_shortcuts()
        self._start_camera()
        self._update_ui()

    def _build_ui(self):
        self.main_frame = tk.Frame(self.root, bg=PALETTE["bg_main"])
        self.main_frame.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)
        self._build_dataset_selector()
        self._build_collection_ui()
        self._show_dataset_selector()

    # --- 1. VERİ SETİ SEÇİCİ & YÖNETİMİ ---
    def _build_dataset_selector(self):
        self.selector_frame = tk.Frame(self.main_frame, bg=PALETTE["bg_card"],
                                       highlightthickness=1, highlightbackground=PALETTE["border"])

        header_frame = tk.Frame(self.selector_frame, bg=PALETTE["bg_card"])
        header_frame.pack(fill=tk.X, pady=(35, 15))

        tk.Label(header_frame, text="⚡ FALLOWER VERİ SETİ YÖNETİCİSİ",
                 font=("Segoe UI", 20, "bold"), bg=PALETTE["bg_card"], fg=PALETTE["accent_blue"]).pack()
        tk.Label(header_frame, text="Kayıt yapılacak hedef veri setini seçin veya yeni bir oturum klasörü oluşturun.",
                 font=("Segoe UI", 10), bg=PALETTE["bg_card"], fg=PALETTE["fg_muted"]).pack(pady=3)

        cards_container = tk.Frame(self.selector_frame, bg=PALETTE["bg_card"])
        cards_container.pack(fill=tk.BOTH, expand=True, padx=35, pady=15)

        # Sol Kart: Mevcut Veri Setleri
        card_list = tk.Frame(cards_container, bg=PALETTE["bg_panel"],
                             highlightthickness=1, highlightbackground=PALETTE["border"], padx=20, pady=20)
        card_list.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 10))

        tk.Label(card_list, text="📋  MEVCUT VERİ SETLERİ",
                 font=("Segoe UI", 12, "bold"), bg=PALETTE["bg_panel"], fg=PALETTE["accent_blue"]).pack(anchor=tk.W, pady=(0, 8))

        list_container = tk.Frame(card_list, bg=PALETTE["bg_panel"])
        list_container.pack(fill=tk.BOTH, expand=True, pady=(0, 10))

        scrollbar = tk.Scrollbar(list_container)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        self.dataset_listbox = tk.Listbox(
            list_container, font=("Consolas", 10), bg="#ffffff", fg=PALETTE["fg_text"],
            selectbackground=PALETTE["accent_blue"], selectforeground="#ffffff",
            highlightthickness=1, highlightbackground=PALETTE["border"], bd=0,
            yscrollcommand=scrollbar.set
        )
        self.dataset_listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.config(command=self.dataset_listbox.yview)

        btn_row_list = tk.Frame(card_list, bg=PALETTE["bg_panel"])
        btn_row_list.pack(fill=tk.X)

        ModernButton(btn_row_list, text="✅  Seç ve Kayıt Ekranına Geç", command=self._select_dataset,
                     bg_color=PALETTE["accent_green"], fg_color="#ffffff",
                     hover_bg="#237032", font=("Segoe UI", 10, "bold"), pady=6).pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 4))

        ModernButton(btn_row_list, text="🔄 Yenile", command=self._refresh_dataset_list,
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"],
                     hover_bg=PALETTE["btn_hover"], font=("Segoe UI", 9), pady=6).pack(side=tk.RIGHT, padx=(4, 0))

        # Sağ Kart: Yeni Veri Seti Oluştur
        card_new = tk.Frame(cards_container, bg=PALETTE["bg_panel"],
                            highlightthickness=1, highlightbackground=PALETTE["border"], padx=20, pady=16)
        card_new.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(10, 0))

        tk.Label(card_new, text="➕  YENİ VERİ SETİ OLUŞTUR",
                 font=("Segoe UI", 12, "bold"), bg=PALETTE["bg_panel"], fg=PALETTE["accent_yellow"]).pack(anchor=tk.W, pady=(0, 6))

        form_frame = tk.Frame(card_new, bg=PALETTE["bg_panel"])
        form_frame.pack(fill=tk.X, expand=False)

        labels = [("Veri Seti Adı:", "entry_name"),
                  ("Oda / Ortam (Room):", "entry_room"),
                  ("Radar Konumu (Position):", "entry_radar")]

        for i, (lbl_txt, attr_name) in enumerate(labels):
            tk.Label(form_frame, text=lbl_txt, bg=PALETTE["bg_panel"], fg=PALETTE["fg_text"],
                     font=("Segoe UI", 9, "bold")).grid(row=i*2, column=0, sticky=tk.W, pady=(2, 1))
            entry = tk.Entry(form_frame, bg="#ffffff", fg=PALETTE["fg_text"],
                             insertbackground=PALETTE["fg_text"], relief=tk.SOLID, bd=1, font=("Segoe UI", 10))
            entry.grid(row=i*2+1, column=0, sticky=tk.EW, pady=(0, 4))
            setattr(self, attr_name, entry)

        tk.Label(form_frame, text="Notlar (Opsiyonel):", bg=PALETTE["bg_panel"], fg=PALETTE["fg_text"],
                 font=("Segoe UI", 9, "bold")).grid(row=6, column=0, sticky=tk.W, pady=(2, 1))
        self.entry_notes = tk.Text(form_frame, height=2, bg="#ffffff", fg=PALETTE["fg_text"],
                                   insertbackground=PALETTE["fg_text"], relief=tk.SOLID, bd=1, font=("Segoe UI", 9))
        self.entry_notes.grid(row=7, column=0, sticky=tk.EW, pady=(0, 6))

        form_frame.columnconfigure(0, weight=1)

        ModernButton(card_new, text="🚀  Veri Setini Oluştur", command=self._create_dataset,
                     bg_color=PALETTE["accent_yellow"], fg_color="#ffffff",
                     hover_bg="#c76700", font=("Segoe UI", 10, "bold"), pady=6).pack(fill=tk.X, pady=(2, 8))

        # Radar Donanım Konfigürasyon Bölümü (Mesafe / Reg34 Ayarı: 15m = 128 dec)
        radar_config_frame = tk.LabelFrame(card_new, text=" ⚙️ Radar Menzil & Donanım Konfigürasyonu ", bg=PALETTE["bg_panel"],
                                           fg=PALETTE["accent_blue"], font=("Segoe UI", 9, "bold"), bd=1)
        radar_config_frame.pack(fill=tk.X, pady=(4, 0))

        # 1. Satır: Mesafe (Metre) ve Reg34 (Hex) Çift Girişi
        inputs_row = tk.Frame(radar_config_frame, bg=PALETTE["bg_panel"])
        inputs_row.pack(fill=tk.X, padx=10, pady=(6, 2))

        # Metre Girişi
        tk.Label(inputs_row, text="Menzil (Metre):", font=("Segoe UI", 9, "bold"),
                 bg=PALETTE["bg_panel"], fg=PALETTE["accent_blue"]).pack(side=tk.LEFT)

        self.var_range_m = tk.StringVar(value="3.0")
        self.entry_range_m = tk.Entry(inputs_row, textvariable=self.var_range_m, width=6,
                                      font=("Consolas", 10, "bold"), bg="#ffffff", fg=PALETTE["accent_blue"],
                                      justify=tk.CENTER, relief=tk.SOLID, bd=1)
        self.entry_range_m.pack(side=tk.LEFT, padx=(4, 10))
        self.entry_range_m.bind("<KeyRelease>", lambda e: self._on_range_input_change())

        # Reg34 Hex Girişi
        tk.Label(inputs_row, text="reg34 (0x22):", font=("Segoe UI", 9, "bold"),
                 bg=PALETTE["bg_panel"], fg=PALETTE["fg_text"]).pack(side=tk.LEFT)

        self.var_reg34 = tk.StringVar(value="0x1A")
        self.entry_reg34 = tk.Entry(inputs_row, textvariable=self.var_reg34, width=6,
                                    font=("Consolas", 10, "bold"), bg="#ffffff", fg=PALETTE["accent_purple"],
                                    justify=tk.CENTER, relief=tk.SOLID, bd=1)
        self.entry_reg34.pack(side=tk.LEFT, padx=(4, 10))
        self.entry_reg34.bind("<KeyRelease>", lambda e: self._on_reg34_input_change())

        # 2. Satır: Hızlı Mesafe Preset Butonları (15m = 128 dec oranına göre)
        preset_frame = tk.Frame(radar_config_frame, bg=PALETTE["bg_panel"])
        preset_frame.pack(fill=tk.X, padx=10, pady=(2, 4))

        tk.Label(preset_frame, text="Hızlı Menzil:", font=("Segoe UI", 8, "bold"),
                 bg=PALETTE["bg_panel"], fg=PALETTE["fg_muted"]).pack(side=tk.LEFT, padx=(0, 2))

        range_presets = [
            ("2.0m", 2.0),
            ("3.0m", 3.0),
            ("4.0m", 4.0),
            ("5.0m", 5.0),
            ("7.5m", 7.5),
            ("10.0m", 10.0),
            ("15.0m", 15.0)
        ]
        for p_label, p_meters in range_presets:
            ModernButton(preset_frame, text=p_label, command=lambda m=p_meters: self._set_range_preset(m),
                         bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"],
                         hover_bg=PALETTE["btn_hover"], font=("Segoe UI", 8), padx=3, pady=2).pack(side=tk.LEFT, padx=1)

        # Reg34 Açıklama & Formül & Hedef Bin Hesaplama Etiketi
        self.lbl_reg34_info = tk.Label(
            radar_config_frame,
            text="Menzil: 3.05 m  │  Reg34: 0x1A (26 dec)  │  Hedef Bin: 0x7A+0x1A = 0x94 (148)  │  (15m = 128 dec)",
            font=("Consolas", 8), bg=PALETTE["bg_panel"], fg=PALETTE["accent_cyan"]
        )
        self.lbl_reg34_info.pack(anchor=tk.W, padx=10, pady=(0, 4))

        self.btn_radar_config = ModernButton(
            radar_config_frame, text="📡  Radarı Konfigüre Et  (Menzil & DC Kalibrasyon)",
            command=self._start_radar_configuration,
            bg_color=PALETTE["accent_cyan"], fg_color="#ffffff",
            hover_bg="#0c8599", font=("Segoe UI", 10, "bold"), pady=6
        )
        self.btn_radar_config.pack(fill=tk.X, padx=10, pady=(2, 4))

        self.lbl_radar_config_status = tk.Label(
            radar_config_frame, text="Durum: Bekliyor (Menzili seçip 'Radarı Konfigüre Et'e basın)",
            font=("Segoe UI", 9), bg=PALETTE["bg_panel"], fg=PALETTE["fg_muted"]
        )
        self.lbl_radar_config_status.pack(padx=10, pady=(0, 6))

        self._refresh_dataset_list()

    # --- 2. TOPLAMA / CANLI İZLEME EKRANI ---
    def _build_collection_ui(self):
        self.collection_frame = tk.Frame(self.main_frame, bg=PALETTE["bg_main"])

        # Üst Durum Barı
        top_bar = tk.Frame(self.collection_frame, bg=PALETTE["bg_card"], height=48,
                           highlightthickness=1, highlightbackground=PALETTE["border"])
        top_bar.pack(fill=tk.X, pady=(0, 6))
        top_bar.pack_propagate(False)

        ModernButton(top_bar, text="◀  Veri Seti Değiştir", command=self._back_to_selector,
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"],
                     hover_bg=PALETTE["btn_hover"], font=("Segoe UI", 9, "bold")).pack(side=tk.LEFT, padx=10, pady=8)

        self.lbl_dataset_info = tk.Label(top_bar, text="Veri Seti: -", font=("Segoe UI", 11, "bold"),
                                         bg=PALETTE["bg_card"], fg=PALETTE["accent_blue"])
        self.lbl_dataset_info.pack(side=tk.LEFT, padx=15)

        self.lbl_status = tk.Label(top_bar, text="BEKLEMEDE", font=("Segoe UI", 9, "bold"),
                                   bg="#e9ecef", fg=PALETTE["fg_muted"], padx=8, pady=2)
        self.lbl_status.pack(side=tk.RIGHT, padx=10)

        self.lbl_sync_status = tk.Label(top_bar, text="RADAR: -- FPS │ KAMERA: -- FPS",
                                        font=("Consolas", 9, "bold"), bg=PALETTE["bg_card"], fg=PALETTE["accent_cyan"])
        self.lbl_sync_status.pack(side=tk.RIGHT, padx=10)

        self.lbl_adc_status = tk.Label(top_bar, text="ADC: BAĞLI DEĞİL",
                                       font=("Segoe UI", 9, "bold"), bg="#fff0f6", fg=PALETTE["accent_red"], padx=6, pady=2)
        self.lbl_adc_status.pack(side=tk.RIGHT, padx=10)

        self.lbl_reg34_bar = tk.Label(top_bar, text=f"MENZİL: {self.active_range_m:.2f}m (0x{self.active_reg34:02X})",
                                      font=("Consolas", 9, "bold"), bg=PALETTE["bg_card"], fg=PALETTE["accent_purple"])
        self.lbl_reg34_bar.pack(side=tk.RIGHT, padx=6)

        # 1. Üst Bölüm: Canlı Radar ADC Analizi (Boydan Boya Uzunlamasına)
        self.radar_panel = tk.Frame(self.collection_frame, bg=PALETTE["bg_card"],
                                    highlightthickness=1, highlightbackground=PALETTE["border"])
        self.radar_panel.pack(fill=tk.BOTH, expand=True, pady=(0, 6))

        radar_header = tk.Frame(self.radar_panel, bg=PALETTE["bg_card"])
        radar_header.pack(fill=tk.X, padx=12, pady=(8, 4))

        tk.Label(radar_header, text="📡  CANLI RADAR SİNYAL ANALİZİ", font=("Segoe UI", 11, "bold"),
                 bg=PALETTE["bg_card"], fg=PALETTE["accent_blue"]).pack(side=tk.LEFT)

        # Y-Ölçek Seçici
        self.btn_yscale = ModernButton(radar_header, text="📐 Y-Ölçek: Otomatik",
                                       command=self._toggle_yscale_mode,
                                       bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"],
                                       hover_bg=PALETTE["btn_hover"], font=("Segoe UI", 9))
        self.btn_yscale.pack(side=tk.RIGHT, padx=(4, 0))

        # Görünüm Modu Butonları (Ham ADC / MTI Filtresi / IIR Arka Plan)
        self.btn_mode_iir = ModernButton(radar_header, text="🌊 IIR Arka Plan",
                                         command=lambda: self._set_view_mode("iir_bg"),
                                         bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"],
                                         hover_bg=PALETTE["btn_hover"], font=("Segoe UI", 9, "bold"))
        self.btn_mode_iir.pack(side=tk.RIGHT, padx=3)

        self.btn_mode_mti = ModernButton(radar_header, text="📊 MTI Filtresi",
                                         command=lambda: self._set_view_mode("mti"),
                                         bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"],
                                         hover_bg=PALETTE["btn_hover"], font=("Segoe UI", 9, "bold"))
        self.btn_mode_mti.pack(side=tk.RIGHT, padx=3)

        self.btn_mode_raw = ModernButton(radar_header, text="📈 Ham ADC",
                                         command=lambda: self._set_view_mode("waveform"),
                                         bg_color="#e7f5ff", fg_color=PALETTE["accent_blue"],
                                         hover_bg="#d0ebff", font=("Segoe UI", 9, "bold"))
        self.btn_mode_raw.pack(side=tk.RIGHT, padx=3)

        # ADC Canvas (Beyaz, Keskin, Titremesiz)
        self.adc_canvas = tk.Canvas(self.radar_panel, bg=PALETTE["bg_canvas"],
                                    highlightthickness=1, highlightbackground=PALETTE["border"], cursor="crosshair")
        self.adc_canvas.pack(fill=tk.BOTH, expand=True, padx=12, pady=4)
        self.adc_canvas.bind("<Configure>", lambda e: self._on_canvas_resize())
        self.adc_canvas.bind("<Motion>", self._on_adc_canvas_hover)
        self.adc_canvas.bind("<Leave>", lambda e: self._clear_canvas_hover())

        # Radar İstatistik Çubuğu (Sabit Konum)
        self.adc_stats_frame = tk.Frame(self.radar_panel, bg=PALETTE["bg_panel"], height=30)
        self.adc_stats_frame.pack(fill=tk.X, padx=12, pady=(0, 8))
        self.adc_stats_frame.pack_propagate(False)

        self.adc_stats = tk.Label(self.adc_stats_frame,
                                  text="Min: -  |  Max: -  |  P-P: -  |  Ort: -  |  RMS: -  |  Frame: -",
                                  bg=PALETTE["bg_panel"], fg=PALETTE["fg_muted"], font=("Consolas", 9))
        self.adc_stats.pack(side=tk.LEFT, padx=8, pady=4)

        # 2. Alt Bölüm: Kamera (Sol Alt - Tam 16:9 Sıfır Boşluk) + Kayıt Kontrolleri (Sağ Alt - Kalan Alanı Doldurur)
        self.bottom_container = tk.Frame(self.collection_frame, bg=PALETTE["bg_main"])
        self.bottom_container.pack(fill=tk.BOTH, expand=True)

        # Sol Alt: Canlı Kamera Görüntüsü Paneli (16:9 oranına kilitli genişlik, kenarda boşluk bırakmaz)
        self.cam_panel = tk.Frame(self.bottom_container, bg=PALETTE["bg_card"],
                                  highlightthickness=1, highlightbackground=PALETTE["border"], width=460)
        self.cam_panel.pack(side=tk.LEFT, fill=tk.Y, expand=False, padx=(0, 4))
        self.cam_panel.pack_propagate(False)

        cam_header = tk.Frame(self.cam_panel, bg=PALETTE["bg_card"])
        cam_header.pack(fill=tk.X, padx=12, pady=(8, 4))

        tk.Label(cam_header, text="📷  CANLI KAMERA GÖRÜNTÜSÜ", font=("Segoe UI", 11, "bold"),
                 bg=PALETTE["bg_card"], fg=PALETTE["accent_blue"]).pack(side=tk.LEFT)

        self.lbl_cam_fps = tk.Label(cam_header, text="-- FPS", font=("Segoe UI", 9, "bold"),
                                    bg=PALETTE["btn_bg"], fg=PALETTE["accent_cyan"], padx=6, pady=1)
        self.lbl_cam_fps.pack(side=tk.RIGHT)

        # Sabit Kamera Konteyneri
        self.cam_container = tk.Frame(self.cam_panel, bg="#000000")
        self.cam_container.pack(fill=tk.BOTH, expand=True, padx=12, pady=(4, 8))
        self.cam_container.pack_propagate(False)

        self.cam_label = tk.Label(self.cam_container, bg="#000000")
        self.cam_label.place(x=0, y=0, relwidth=1.0, relheight=1.0)

        # Sağ Alt: Kayıt Kontrol & Oturum Paneli (Genişleyen Panel)
        control = tk.Frame(self.bottom_container, bg=PALETTE["bg_card"],
                           highlightthickness=1, highlightbackground=PALETTE["border"], padx=15, pady=12)
        control.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(4, 0))

        self.bottom_container.bind("<Configure>", self._on_bottom_container_resize)

        ctrl_header = tk.Frame(control, bg=PALETTE["bg_card"])
        ctrl_header.pack(fill=tk.X, pady=(0, 10))

        tk.Label(ctrl_header, text="🎛️  KAYIT & OTURUM KONTROLÜ", font=("Segoe UI", 11, "bold"),
                 bg=PALETTE["bg_card"], fg=PALETTE["accent_blue"]).pack(side=tk.LEFT)

        desc_frame = tk.Frame(control, bg=PALETTE["bg_card"])
        desc_frame.pack(fill=tk.X, pady=(0, 10))

        tk.Label(desc_frame, text="Veri Açıklaması (Kişi / Durum / Eylem):",
                 bg=PALETTE["bg_card"], fg=PALETTE["fg_text"], font=("Segoe UI", 9, "bold")).pack(anchor=tk.W)

        self.entry_data_desc = tk.Entry(desc_frame, bg=PALETTE["bg_panel"], fg=PALETTE["fg_text"],
                                        insertbackground=PALETTE["fg_text"], relief=tk.SOLID, bd=1,
                                        font=("Segoe UI", 10))
        self.entry_data_desc.pack(fill=tk.X, pady=4)
        self.entry_data_desc.bind("<Return>", lambda e: self._start_record() if not self.recording else None)

        btn_bar = tk.Frame(control, bg=PALETTE["bg_card"])
        btn_bar.pack(fill=tk.X, pady=(6, 10))

        self.btn_start = ModernButton(btn_bar, text="▶   KAYDI BAŞLAT  [Space]",
                                     bg_color=PALETTE["accent_green"], fg_color="#ffffff",
                                     hover_bg="#237032", font=("Segoe UI", 11, "bold"),
                                     pady=8, command=self._start_record)
        self.btn_start.pack(fill=tk.X, pady=(0, 6))

        self.btn_stop = ModernButton(btn_bar, text="⏹   KAYDI DURDUR  [Space]",
                                    bg_color="#e9ecef", fg_color=PALETTE["fg_dim"],
                                    hover_bg="#dee2e6", font=("Segoe UI", 11, "bold"),
                                    pady=8, command=self._stop_record, state=tk.DISABLED)
        self.btn_stop.pack(fill=tk.X, pady=(0, 6))

        status_box = tk.Frame(control, bg=PALETTE["bg_panel"], padx=10, pady=8,
                              highlightthickness=1, highlightbackground=PALETTE["border"])
        status_box.pack(fill=tk.X, pady=(4, 0))

        self.lbl_record_detail = tk.Label(status_box, text="Henüz kayıt yapılmadı.",
                                          bg=PALETTE["bg_panel"], fg=PALETTE["fg_muted"], font=("Consolas", 10))
        self.lbl_record_detail.pack(anchor=tk.W)

    # -------------------------------------------------------------------------
    # KLAVYE KISAYOLLARI
    # -------------------------------------------------------------------------
    def _bind_shortcuts(self):
        self.root.bind("<space>", lambda e: self._on_spacebar())
        self.root.bind("<Escape>", lambda e: self._on_escape())

    def _on_spacebar(self):
        focused = self.root.focus_get()
        if isinstance(focused, tk.Entry):
            return
        if self.collection_frame.winfo_viewable():
            if not self.recording:
                self._start_record()
            else:
                self._stop_record()

    def _on_escape(self):
        focused = self.root.focus_get()
        if isinstance(focused, tk.Entry):
            return
        if self.collection_frame.winfo_viewable() and not self.recording:
            self._back_to_selector()

    # -------------------------------------------------------------------------
    # VERİ SETİ İŞLEMLERİ
    # -------------------------------------------------------------------------
    def _refresh_dataset_list(self):
        self.dataset_listbox.delete(0, tk.END)
        datasets = self.dataset_manager.get_datasets()
        for ds in datasets:
            line = (f"{ds['name']:<24} │ Oda: {ds.get('room','?'):<14} │ "
                    f"Radar: {ds.get('radar_position','?'):<14} │ Kayıt: {ds.get('record_count',0):>4}")
            self.dataset_listbox.insert(tk.END, line)

    def _create_dataset(self):
        name = self.entry_name.get().strip()
        room = self.entry_room.get().strip()
        radar = self.entry_radar.get().strip()
        notes = self.entry_notes.get("1.0", tk.END).strip()
        if not name:
            messagebox.showerror("Hata", "Veri seti adı boş olamaz!")
            return
        ok, msg = self.dataset_manager.create_dataset(name, room, radar, notes)
        if ok:
            messagebox.showinfo("Başarılı", f"Veri seti oluşturuldu.\n{msg}")
            self._refresh_dataset_list()
            self.entry_name.delete(0, tk.END)
            self.entry_room.delete(0, tk.END)
            self.entry_radar.delete(0, tk.END)
            self.entry_notes.delete("1.0", tk.END)
        else:
            messagebox.showerror("Hata", msg)

    # -------------------------------------------------------------------------
    # RADAR MENZİL (RANGE) & REG34 HESAPLAMA & DONANIM KONFİGÜRASYON MOTORU
    # -------------------------------------------------------------------------
    @staticmethod
    def _range_to_reg34(meters: float) -> int:
        """15 metre = 128 desimal (0x80) oranına göre reg34 değerini hesaplar"""
        dec = int(round((float(meters) / 15.0) * 128.0))
        return max(1, min(255, dec))

    @staticmethod
    def _reg34_to_range(reg34_dec: int) -> float:
        """reg34 desimal değerinden hesaplanan metreyi döner (128 dec = 15m)"""
        return (float(reg34_dec) / 128.0) * 15.0

    def _parse_reg34_value(self):
        val_str = self.var_reg34.get().strip()
        try:
            if val_str.lower().startswith("0x"):
                val = int(val_str, 16)
            elif any(c in "abcdefABCDEF" for c in val_str):
                val = int(val_str, 16)
            else:
                val = int(val_str)
            if 0 <= val <= 255:
                return val
            return None
        except Exception:
            return None

    def _set_range_preset(self, meters: float):
        self.var_range_m.set(f"{meters:.1f}")
        reg34_dec = self._range_to_reg34(meters)
        self.var_reg34.set(f"0x{reg34_dec:02X}")
        self._update_range_display(meters, reg34_dec)

    def _on_range_input_change(self):
        val_str = self.var_range_m.get().strip().replace(",", ".")
        try:
            meters = float(val_str)
            if meters > 0:
                reg34_dec = self._range_to_reg34(meters)
                self.var_reg34.set(f"0x{reg34_dec:02X}")
                self._update_range_display(meters, reg34_dec)
            else:
                self.lbl_reg34_info.config(text="⚠️ Pozitif bir metre değeri giriniz!", fg=PALETTE["accent_red"])
        except ValueError:
            self.lbl_reg34_info.config(text="⚠️ Geçersiz metre formatı! (Örn: 3.0, 5.5)", fg=PALETTE["accent_red"])

    def _on_reg34_input_change(self):
        reg34_dec = self._parse_reg34_value()
        if reg34_dec is not None:
            meters = self._reg34_to_range(reg34_dec)
            self.var_range_m.set(f"{meters:.2f}")
            self._update_range_display(meters, reg34_dec)
        else:
            self.lbl_reg34_info.config(
                text="⚠️ Geçersiz reg34 değeri! 0x00 - 0xFF (0-255) arasında giriniz.",
                fg=PALETTE["accent_red"]
            )

    def _update_range_display(self, meters: float, reg34_dec: int):
        target_bin = 0x7A + reg34_dec
        actual_m = self._reg34_to_range(reg34_dec)
        self.lbl_reg34_info.config(
            text=f"Menzil: {actual_m:.2f} m  │  Reg34: 0x{reg34_dec:02X} ({reg34_dec} dec)  │  Hedef Bin: 0x7A+0x{reg34_dec:02X} = 0x{target_bin:02X} ({target_bin})  │  (15m = 128 dec)",
            fg=PALETTE["accent_cyan"]
        )

    def _update_top_bar_reg34(self):
        if hasattr(self, "lbl_reg34_bar"):
            actual_m = self._reg34_to_range(self.active_reg34)
            self.root.after(0, lambda: self.lbl_reg34_bar.config(
                text=f"MENZİL: {actual_m:.2f}m (0x{self.active_reg34:02X})"
            ))

    def _start_radar_configuration(self):
        if self._radar_configuring:
            return

        if not self.fallower_system:
            messagebox.showwarning("Uyarı", "Fallower sistem altyapısı bulunamadı!")
            return

        si = getattr(self.fallower_system, "serial_interface", None)
        if not si or not (hasattr(si, "ser") and si.ser and si.ser.is_open):
            messagebox.showerror(
                "Bağlantı Hatası",
                "Radar seri port bağlantısı aktif değil!\n\n"
                "Lütfen radar kartının USB bağlantısını kontrol edin."
            )
            return

        threading.Thread(target=self._run_radar_configuration_sequence, daemon=True).start()

    def _run_radar_configuration_sequence(self):
        self._radar_configuring = True
        self.root.after(0, lambda: self.btn_radar_config.config(state=tk.DISABLED, bg="#adb5bd"))

        reg34_val = self._parse_reg34_value()
        if reg34_val is None:
            self._update_config_status("❌ Hata: Geçersiz reg34 (0x22) değeri!", PALETTE["accent_red"])
            messagebox.showerror(
                "Geçersiz Değer",
                "Girilen reg34 (0x22) veya menzil değeri geçersiz!\n\n"
                "Lütfen geçerli bir metre (örn: 3.0) veya reg34 (örn: 0x1A) değeri girin."
            )
            self._radar_configuring = False
            self.root.after(0, lambda: self.btn_radar_config.config(state=tk.NORMAL, bg=PALETTE["accent_cyan"]))
            return

        try:
            actual_m = self._reg34_to_range(reg34_val)

            # 1. ADIM: INIT (Sistemi Başlatma & Sıfırlama)
            self._update_config_status("⏳ 1/4: send_init_command() gönderiliyor...", PALETTE["accent_blue"])
            if HAS_CORE and hasattr(self.fallower_system, "_update_status"):
                self.fallower_system._update_status(SystemStatus.READY, source="ConfigSequence (INIT-Prep)")

            ok_init = self.fallower_system.send_init_command()
            if not ok_init and self.fallower_system.serial_interface:
                ok_init = self.fallower_system.serial_interface.send_init_command()

            if not ok_init:
                self._update_config_status("❌ Adım 1 Hatası: send_init_command() başarısız!", PALETTE["accent_red"])
                messagebox.showerror("Hata", "Adım 1 (INIT) başarısız oldu!\nfallower_system.send_init_command() komutu iletilemedi.")
                return

            # INIT durumunun donanımda tamamlanmasını bekle
            time.sleep(1.2)

            # 2. ADIM: READY (Komut Kabul Durumuna Geçiş)
            self._update_config_status("⏳ 2/4: send_ready_command() gönderiliyor...", PALETTE["accent_purple"])
            if HAS_CORE and hasattr(self.fallower_system, "_update_status"):
                self.fallower_system._update_status(SystemStatus.READY, source="ConfigSequence (READY-Prep)")

            ok_ready = self.fallower_system.send_ready_command()
            if not ok_ready and self.fallower_system.serial_interface:
                ok_ready = self.fallower_system.serial_interface.send_ready_command()

            if not ok_ready:
                self._update_config_status("❌ Adım 2 Hatası: send_ready_command() başarısız!", PALETTE["accent_red"])
                messagebox.showerror("Hata", "Adım 2 (READY) başarısız oldu!\nfallower_system.send_ready_command() komutu iletilemedi.")
                return

            # READY durumunun tamamlanmasını bekle
            time.sleep(1.2)

            # 3. ADIM: CALIBRATE (Radar DC Offset Kalibrasyonu)
            self._update_config_status("⏳ 3/4: send_calibrate_command() gönderiliyor...", PALETTE["accent_yellow"])
            if HAS_CORE and hasattr(self.fallower_system, "_update_status"):
                self.fallower_system._update_status(SystemStatus.READY, source="ConfigSequence (CALIB-Prep)")

            ok_calib = self.fallower_system.send_calibrate_command()
            if not ok_calib and self.fallower_system.serial_interface:
                ok_calib = self.fallower_system.serial_interface.send_calibrate_command()

            if not ok_calib:
                self._update_config_status("❌ Adım 3 Hatası: send_calibrate_command() başarısız!", PALETTE["accent_red"])
                messagebox.showerror("Hata", "Adım 3 (CALIBRATE) başarısız oldu!\nfallower_system.send_calibrate_command() komutu iletilemedi.")
                return

            # Kalibrasyon ölçümünün tamamlanmasını bekle
            time.sleep(2.2)

            # Kalibrasyon sonrası READY durumuna geç
            if HAS_CORE and hasattr(self.fallower_system, "_update_status"):
                self.fallower_system._update_status(SystemStatus.READY, source="ConfigSequence (READY-PostCalib)")
            self.fallower_system.send_ready_command()
            time.sleep(0.5)

            # 4. ADIM: REGISTER GÜNCELLEME (Menzil: reg34 = 0xXX donanıma yazılıyor)
            self._update_config_status(f"⏳ 4/4: Menzil ayarlanıyor: {actual_m:.2f}m (reg34=0x{reg34_val:02X})...", PALETTE["accent_cyan"])
            if HAS_CORE and hasattr(self.fallower_system, "_update_status"):
                self.fallower_system._update_status(SystemStatus.READY, source="ConfigSequence (REG-Prep)")

            # A) Doğrudan donanıma SET_REG34 komutu gönder
            if hasattr(self.fallower_system, "send_set_reg34"):
                self.fallower_system.send_set_reg34(reg34_val)
            elif self.fallower_system.serial_interface:
                self.fallower_system.serial_interface.send_command(f"SET_REG34:0x{reg34_val:02X}")

            # B) Tam register haritasını gönder
            reg_map = DEFAULT_RADAR_REGISTERS.copy()
            reg_map[34] = reg34_val

            ok_reg = self.fallower_system.send_register_configuration(reg_map)
            if not ok_reg and self.fallower_system.serial_interface:
                ok_reg = self.fallower_system.serial_interface.send_register_configuration(reg_map)

            time.sleep(1.0)

            # Sistemi READY durumuna getir (Kayıt başlangıcı için hazır)
            if HAS_CORE and hasattr(self.fallower_system, "_update_status"):
                self.fallower_system._update_status(SystemStatus.READY, source="ConfigSequence (Complete)")

            self.active_reg34 = reg34_val
            self.active_range_m = actual_m
            target_bin = 0x7A + reg34_val
            self._update_top_bar_reg34()

            # TAMAMLANDI
            self._update_config_status(f"✅ Radar Başarıyla Konfigüre Edildi! ({actual_m:.2f}m │ reg34=0x{reg34_val:02X})", PALETTE["accent_green"])
            messagebox.showinfo(
                "Konfigürasyon Başarılı",
                f"Radar donanımı başarıyla hazırlandı ve kalibre edildi!\n\n"
                f"✓ Menzil: {actual_m:.2f} metre (Oran: 15m = 128 dec)\n"
                f"✓ Reg34 (0x22): 0x{reg34_val:02X} (Desimal: {reg34_val})\n"
                f"✓ Hedef Bin: 0x7A + 0x{reg34_val:02X} = 0x{target_bin:02X} ({target_bin})\n"
                f"✓ DC Offset Kalibrasyonu [Tamamlandı]\n\n"
                f"Artık veri setinizi seçip kayıt ekranına geçebilirsiniz."
            )

        except Exception as e:
            self._update_config_status(f"❌ Hata: {e}", PALETTE["accent_red"])
            messagebox.showerror("Beklenmeyen Hata", f"Radar konfigürasyonu sırasında hata oluştu:\n{e}")
        finally:
            self._radar_configuring = False
            self.root.after(0, lambda: self.btn_radar_config.config(state=tk.NORMAL, bg=PALETTE["accent_cyan"]))

    def _update_config_status(self, text, color):
        self.root.after(0, lambda: self.lbl_radar_config_status.config(text=text, fg=color))

    def _select_dataset(self):
        sel = self.dataset_listbox.curselection()
        if not sel:
            messagebox.showwarning("Uyarı", "Lütfen bir veri seti seçin!")
            return
        line = self.dataset_listbox.get(sel[0])
        name = line.split("│")[0].strip()
        self.current_dataset = name
        meta_path = os.path.join(self.dataset_manager.get_dataset_path(name), "metadata.json")
        if os.path.exists(meta_path):
            with open(meta_path, "r", encoding="utf-8") as f:
                meta = json.load(f)
            info = (f"Veri Seti: {meta['name']}  │  "
                    f"Oda: {meta['room']}  │  Radar: {meta['radar_position']}")
        else:
            info = f"Veri Seti: {name}"
        self.lbl_dataset_info.config(text=info)
        self._show_collection_ui()
        self._start_adc_streaming()

    def _show_dataset_selector(self):
        self.collection_frame.pack_forget()
        self.selector_frame.pack(fill=tk.BOTH, expand=True)

    def _show_collection_ui(self):
        self.selector_frame.pack_forget()
        self.collection_frame.pack(fill=tk.BOTH, expand=True)
        self.entry_data_desc.focus_set()

    def _back_to_selector(self):
        if self.recording:
            messagebox.showwarning("Uyarı", "Önce kaydı durdurun!")
            return
        self._stop_adc_streaming()
        self._show_dataset_selector()

    # -------------------------------------------------------------------------
    # DONANIM & AKIŞ KONTROLÜ
    # -------------------------------------------------------------------------
    def _start_adc_streaming(self):
        if self.fallower_system is None:
            self.lbl_adc_status.config(text="ADC: MOCK (Sistem Yok)", bg="#fff9db", fg=PALETTE["accent_yellow"])
            return
        si = self.fallower_system.serial_interface
        if si is None:
            self.lbl_adc_status.config(text="ADC: Seri Port Yok", bg="#fff0f6", fg=PALETTE["accent_red"])
            return
        if self.adc_source:
            self.adc_source.start()
        if si.send_command("COLLECT"):
            si._streaming_active = True
            self.lbl_adc_status.config(text="ADC: CANLI", bg="#e6fcf5", fg=PALETTE["accent_green"])
        else:
            self.lbl_adc_status.config(text="ADC: KOMUT HATASI", bg="#fff0f6", fg=PALETTE["accent_red"])

    def _stop_adc_streaming(self):
        if self.fallower_system is None:
            return
        si = self.fallower_system.serial_interface
        if si is None:
            return
        if self.adc_source:
            self.adc_source.stop()
        si._streaming_active = False
        si.send_command("STOP")
        time.sleep(0.1)
        if hasattr(si, "pause_reading"):
            si.pause_reading()
            time.sleep(0.05)
        if hasattr(si, "data_buffer"):
            si.data_buffer.clear()
        if hasattr(si, "ser") and si.ser and si.ser.is_open:
            try:
                si.ser.reset_input_buffer()
                while si.ser.in_waiting > 0:
                    si.ser.read(si.ser.in_waiting)
            except Exception:
                pass
        if hasattr(si, "resume_reading"):
            si.resume_reading()
        self.lbl_adc_status.config(text="ADC: DURDURULDU", bg="#fff0f6", fg=PALETTE["accent_red"])

    def _start_camera(self):
        if not self.camera.start():
            self.camera_mock = True
            print("[WARN] Kamera bulunamadi. Mock mod aktif.")
        else:
            self.camera_mock = False

    # -------------------------------------------------------------------------
    # KAYIT BAŞLATMA & DURDURMA (ADC-MASTER & SENKRON VIDEO)
    # -------------------------------------------------------------------------
    def _start_record(self):
        if not self.current_dataset:
            return
        desc = self.entry_data_desc.get().strip()
        if not desc:
            messagebox.showwarning("Uyarı", "Lütfen veri açıklaması girin (kişi / durum / eylem)!")
            self.entry_data_desc.focus_set()
            return

        self.safe_desc = self.dataset_manager.sanitize_folder_name(desc)
        idx = self.dataset_manager.get_next_record_index(self.current_dataset)
        self.record_index = idx
        self.record_desc = desc

        folder_name = f"{idx:03d}_{self.safe_desc}"
        record_dir = os.path.join(
            self.dataset_manager.get_dataset_path(self.current_dataset), folder_name
        )
        os.makedirs(record_dir, exist_ok=True)
        self.current_record_dir = record_dir

        cam_w, cam_h = self.camera.get_size()
        video_path = os.path.join(record_dir, "video_temp.mp4")
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')
        live_radar_fps = self.adc_source.get_live_fps() if self.adc_source else 0.0
        rec_fps = max(1.0, round(live_radar_fps, 2)) if live_radar_fps > 3.0 else self.video_fps
        self.video_writer = cv2.VideoWriter(video_path, fourcc, rec_fps, (cam_w, cam_h))
        if not self.video_writer.isOpened():
            video_path = os.path.join(record_dir, "video_temp.avi")
            fourcc = cv2.VideoWriter_fourcc(*'MJPG')
            self.video_writer = cv2.VideoWriter(video_path, fourcc, rec_fps, (cam_w, cam_h))

        self.adc_buffer = []
        self.adc_meta_buffer = []
        self.adc_durations_buffer = []
        self.camera_ts_buffer = []
        self.record_start_time = time.time()
        self.stm32_ref_ts = None

        self.recording = True
        self.lbl_status.config(text="🔴 KAYIT YAPIYOR", bg="#fff0f6", fg=PALETTE["accent_red"])
        self.lbl_sync_status.config(
            text="🔴 KAYIT │ ADC-MASTER", fg=PALETTE["accent_red"]
        )
        self.btn_start.configure(state=tk.DISABLED, bg=PALETTE["btn_bg"], fg=PALETTE["fg_dim"])
        self.btn_stop.set_color(PALETTE["accent_red"], "#ffffff", "#c2255c")
        self.btn_stop.configure(state=tk.NORMAL)
        self.entry_data_desc.config(state=tk.DISABLED)
        self.lbl_record_detail.config(
            text=f"Kayıt #{idx:03d} │ {desc} │ Süre: 0.0s │ 0 frame", fg=PALETTE["accent_blue"]
        )
        print(f"[RECORD] Kayit baslatildi: {record_dir}  │  Kayit FPS: {rec_fps:.2f}")

    def _stop_record(self):
        if not self.recording:
            return
        self.recording = False
        end_time = time.time()
        duration_seconds = max(0.001, end_time - self.record_start_time)

        if self.video_writer is not None:
            self.video_writer.release()
            self.video_writer = None

        # 1. ADC Verilerini Kaydet
        if self.adc_buffer:
            adc_array = np.array(self.adc_buffer, dtype=object)
            np.save(os.path.join(self.current_record_dir, "adc.npy"), adc_array)

        # 2. ADC Meta Verilerini Kaydet
        if self.adc_meta_buffer:
            adc_meta = np.array(self.adc_meta_buffer, dtype=np.float64)
            np.save(os.path.join(self.current_record_dir, "adc_meta.npy"), adc_meta)

        # 3. Kamera Zaman Damgalarını Kaydet
        if self.camera_ts_buffer:
            cam_ts = np.array(self.camera_ts_buffer, dtype=np.float64)
            np.save(os.path.join(self.current_record_dir, "camera_timestamps.npy"), cam_ts)

        # 4. Video Yeniden Adlandırma
        start_dt = datetime.fromtimestamp(self.record_start_time).strftime("%Y%m%d_%H%M%S")
        end_dt = datetime.fromtimestamp(end_time).strftime("%Y%m%d_%H%M%S")

        old_mp4 = os.path.join(self.current_record_dir, "video_temp.mp4")
        old_avi = os.path.join(self.current_record_dir, "video_temp.avi")
        if os.path.exists(old_mp4):
            new_video_name = f"video_{start_dt}_{end_dt}.mp4"
            os.rename(old_mp4, os.path.join(self.current_record_dir, new_video_name))
        elif os.path.exists(old_avi):
            new_video_name = f"video_{start_dt}_{end_dt}.avi"
            os.rename(old_avi, os.path.join(self.current_record_dir, new_video_name))
        else:
            new_video_name = None

        adc_frame_count = len(self.adc_buffer)
        video_frame_count = len(self.camera_ts_buffer)
        frame_mismatch = abs(adc_frame_count - video_frame_count)

        # Kayıt süresi ve kare sayısı üzerinden net Radar & Kamera FPS hesabı
        radar_fps = round(adc_frame_count / duration_seconds, 2) if duration_seconds > 0 else DEFAULT_RADAR_FPS
        video_fps = round(video_frame_count / duration_seconds, 2) if duration_seconds > 0 else DEFAULT_VIDEO_FPS
        avg_dur_us = float(np.mean(self.adc_durations_buffer)) if self.adc_durations_buffer else 0.0

        # 5. Metadata.json Üretimi
        record_meta = {
            "index": self.record_index,
            "description": self.record_desc,
            "safe_description": self.safe_desc,
            "start_time": datetime.fromtimestamp(self.record_start_time).isoformat(),
            "end_time": datetime.fromtimestamp(end_time).isoformat(),
            "duration_seconds": round(duration_seconds, 3),
            "adc_frames": adc_frame_count,
            "video_frames": video_frame_count,
            "radar_fps": radar_fps,
            "video_fps": video_fps,
            "avg_radar_duration_us": round(avg_dur_us, 1) if avg_dur_us > 0 else round(1_000_000.0 / radar_fps, 1),
            "radar_config": {
                "range_meters": round(self._reg34_to_range(self.active_reg34), 2),
                "formula": "15m = 128 dec (0x80) -> dec = round((m / 15.0) * 128)",
                "reg34_hex": f"0x{self.active_reg34:02X}",
                "reg34_dec": self.active_reg34,
                "step_bin": self.active_reg34,
                "start_bin_hex": "0x7A",
                "start_bin_dec": 122,
                "target_bin_hex": f"0x{122 + self.active_reg34:02X}",
                "target_bin_dec": 122 + self.active_reg34
            },
            "reference_pc_time_epoch": self.record_start_time,
            "reference_stm32_ts_ms": self.stm32_ref_ts if self.stm32_ref_ts is not None else 0,
            "files": {
                "adc": "adc.npy",
                "adc_meta": "adc_meta.npy",
                "camera_timestamps": "camera_timestamps.npy",
                "video": new_video_name
            },
            "sync_guide": {
                "note": f"Sistem Radar FPS: {radar_fps:.2f} / Kamera FPS: {video_fps:.2f} ile kaydedilmiştir. ADC-MASTER senkronizasyon kullanılır.",
                "adc_stm32_rel": "adc_meta.npy[:, 0]  ( (STM32_TS - reference_stm32_ts_ms) / 1000, saniye )",
                "adc_pc_rel": "adc_meta.npy[:, 1]  ( PC_TS - reference_pc_time_epoch, saniye )",
                "camera_ts": "camera_timestamps.npy  (PC epoch) -> reference_pc_time_epoch çıkarılarak göreceli",
                "frame_index_alignment": "ADC frame #k ↔ Video frame #k",
                "warning": f"Frame uyumsuzluğu: {frame_mismatch} frame" if frame_mismatch > 0 else "Tam senkron"
            }
        }

        with open(os.path.join(self.current_record_dir, "metadata.json"), "w", encoding="utf-8") as f:
            json.dump(record_meta, f, ensure_ascii=False, indent=2)

        # Veri seti ana metadata güncelleme
        ds_meta_path = os.path.join(
            self.dataset_manager.get_dataset_path(self.current_dataset), "metadata.json"
        )
        if os.path.exists(ds_meta_path):
            try:
                with open(ds_meta_path, "r", encoding="utf-8") as f:
                    ds_meta = json.load(f)
                ds_meta["record_count"] = self.dataset_manager.get_next_record_index(self.current_dataset) - 1
                with open(ds_meta_path, "w", encoding="utf-8") as f:
                    json.dump(ds_meta, f, ensure_ascii=False, indent=2)
            except Exception:
                pass

        # UI Durumunu Sıfırla
        self.lbl_status.config(text="KAYIT DURDURULDU", bg="#e9ecef", fg=PALETTE["fg_muted"])
        self.btn_start.set_color(PALETTE["accent_green"], "#ffffff", "#237032")
        self.btn_start.configure(state=tk.NORMAL)
        self.btn_stop.configure(state=tk.DISABLED, bg=PALETTE["btn_bg"], fg=PALETTE["fg_dim"])
        self.entry_data_desc.config(state=tk.NORMAL)

        self.lbl_record_detail.config(
            text=(f"Son Kayıt: #{self.record_index:03d} │ {self.record_desc} │ "
                  f"ADC: {adc_frame_count} ({radar_fps:.1f} FPS) │ Video: {video_frame_count} ({video_fps:.1f} FPS) │ "
                  f"{duration_seconds:.1f}s │ Δ={frame_mismatch}"),
            fg=PALETTE["accent_green"] if frame_mismatch == 0 else PALETTE["accent_yellow"]
        )

        msg = (f"✅ Kayıt başarıyla tamamlandı!\n\n"
               f"Klasör: {os.path.basename(self.current_record_dir)}\n"
               f"ADC Frame: {adc_frame_count}  (Radar: {radar_fps:.1f} FPS)\n"
               f"Video Frame: {video_frame_count}  (Kamera: {video_fps:.1f} FPS)\n"
               f"Uyumsuzluk: {frame_mismatch} frame\n"
               f"Süre: {duration_seconds:.2f} saniye")
        messagebox.showinfo("Kayıt Tamamlandı", msg)
        print(f"[SAVE] Kayit kaydedildi: {self.current_record_dir}")

        self.adc_buffer = []
        self.adc_meta_buffer = []
        self.adc_durations_buffer = []
        self.camera_ts_buffer = []
        self.current_record_dir = None
        self.record_start_time = None
        self.stm32_ref_ts = None

    # -------------------------------------------------------------------------
    # CANLI GÜNCELLEME VE ÇİZİM DÖNGÜSÜ
    # -------------------------------------------------------------------------
    def _update_ui(self):
        adc_items = self._get_adc_data()
        if adc_items:
            self.last_adc_item = adc_items[-1]
            for itm in adc_items:
                if "adc" in itm:
                    self._live_adc_history.append(itm["adc"])
            if len(self._live_adc_history) > 10:
                self._live_adc_history = self._live_adc_history[-10:]

        self._draw_adc(self.last_adc_item)

        frame = self.camera.get_frame()
        if frame is None and self.camera_mock:
            frame = np.ones((480, 640, 3), dtype=np.uint8) * 240
            cv2.putText(frame, "KAMERA BAGLI DEGIL", (140, 230), cv2.FONT_HERSHEY_SIMPLEX, 1.1, (200, 30, 30), 2)
            cv2.putText(frame, "(Mock Test Modu)", (220, 270), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (100, 100, 100), 2)

        if frame is not None:
            self._show_frame(frame)

        # Anlık FPS Etiketlerini Güncelle
        radar_fps = self.adc_source.get_live_fps() if self.adc_source else 0.0
        cam_fps = self.camera.get_live_fps() if self.camera else 0.0

        if not self.recording:
            self.lbl_sync_status.config(
                text=f"RADAR: {radar_fps:.1f} FPS │ KAMERA: {cam_fps:.1f} FPS",
                fg=PALETTE["accent_cyan"]
            )
        self.lbl_cam_fps.config(text=f"{cam_fps:.1f} FPS")

        # Kayıt aktifken ADC frame'lerini ve senkron kamera karelerini işle
        if self.recording and adc_items:
            self._append_record(adc_items)

        self.root.after(GUI_UPDATE_INTERVAL_MS, self._update_ui)

    def _get_adc_data(self):
        if self.adc_source is not None:
            return self.adc_source.get_all_frames()
        return []

    def _append_record(self, adc_items):
        if not self.recording or self.current_record_dir is None:
            return

        pc_ref = self.record_start_time

        for adc_item in adc_items:
            self.adc_buffer.append(adc_item["adc"])
            if "duration_us" in adc_item and adc_item["duration_us"] > 0:
                self.adc_durations_buffer.append(adc_item["duration_us"])

            if self.stm32_ref_ts is None:
                self.stm32_ref_ts = adc_item["stm32_ts"]

            stm32_ts_rel = (float(adc_item["stm32_ts"]) - float(self.stm32_ref_ts)) / 1000.0
            pc_ts_rel = float(adc_item["pc_ts"]) - pc_ref
            self.adc_meta_buffer.append([stm32_ts_rel, pc_ts_rel])

            latest_cam = self.camera.get_frame()
            if latest_cam is not None and self.video_writer is not None:
                self.video_writer.write(latest_cam)
                self.camera_ts_buffer.append(time.time())

        elapsed = time.time() - pc_ref if pc_ref else 0
        radar_count = len(self.adc_buffer)
        cam_count = len(self.camera_ts_buffer)
        live_radar_fps = self.adc_source.get_live_fps() if self.adc_source else (radar_count / elapsed if elapsed > 0 else 0.0)
        live_cam_fps = self.camera.get_live_fps() if self.camera else (cam_count / elapsed if elapsed > 0 else 0.0)

        self.lbl_sync_status.config(
            text=f"KAYIT │ RADAR: {live_radar_fps:.1f} FPS │ KAMERA: {live_cam_fps:.1f} FPS",
            fg=PALETTE["accent_red"]
        )
        self.lbl_record_detail.config(
            text=(f"Kayıt #{self.record_index:03d} │ {self.record_desc} │ "
                  f"Süre: {elapsed:.1f}s │ Radar: {live_radar_fps:.1f} FPS ({radar_count}f) │ Kamera: {live_cam_fps:.1f} FPS ({cam_count}f)"),
            fg=PALETTE["accent_blue"]
        )

    # -------------------------------------------------------------------------
    # RADAR ADC, MTI & IIR ARKA PLAN CANLI ÇİZİM MOTORU (BEYAZ TEMA & TİTREMESİZ)
    # -------------------------------------------------------------------------
    def _set_view_mode(self, mode):
        """
        Canlı Radar Sinyal Analiz modunu belirler:
        - "waveform": Ham ADC (Zaman Bölgesi)
        - "mti": 3-Pulse MTI Filtresi (y[n] = x[n] - 2*x[n-1] + x[n-2])
        - "iir_bg": IIR / Exponential Background Removal (b_k = (1-a)b_{k-1} + a*x_k, y_k = x_k - b_k)
        """
        self.view_mode = mode

        # Buton Aktiflik Renklerini Güncelle
        if mode == "waveform":
            self.btn_mode_raw.configure(bg="#e7f5ff", fg=PALETTE["accent_blue"])
            self.btn_mode_mti.configure(bg=PALETTE["btn_bg"], fg=PALETTE["fg_text"])
            self.btn_mode_iir.configure(bg=PALETTE["btn_bg"], fg=PALETTE["fg_text"])
        elif mode == "mti":
            self.btn_mode_raw.configure(bg=PALETTE["btn_bg"], fg=PALETTE["fg_text"])
            self.btn_mode_mti.configure(bg="#f3f0ff", fg=PALETTE["accent_purple"])
            self.btn_mode_iir.configure(bg=PALETTE["btn_bg"], fg=PALETTE["fg_text"])
        elif mode == "iir_bg":
            self.btn_mode_raw.configure(bg=PALETTE["btn_bg"], fg=PALETTE["fg_text"])
            self.btn_mode_mti.configure(bg=PALETTE["btn_bg"], fg=PALETTE["fg_text"])
            self.btn_mode_iir.configure(bg="#e6fcf5", fg=PALETTE["accent_green"])

        self._draw_adc(self.last_adc_item)

    def _toggle_yscale_mode(self):
        modes = ["auto", "fixed_25k", "fixed_10k"]
        curr_idx = modes.index(self.y_scale_mode)
        self.y_scale_mode = modes[(curr_idx + 1) % len(modes)]
        labels = {"auto": "📐 Y-Ölçek: Otomatik", "fixed_25k": "📐 Y-Ölçek: ±25k", "fixed_10k": "📐 Y-Ölçek: ±10k"}
        self.btn_yscale.configure(text=labels[self.y_scale_mode])
        self._draw_adc(self.last_adc_item)

    def _on_canvas_resize(self):
        w = self.adc_canvas.winfo_width()
        h = self.adc_canvas.winfo_height()
        if abs(w - self._last_canvas_w) > 2 or abs(h - self._last_canvas_h) > 2:
            self._last_canvas_w = w
            self._last_canvas_h = h
            self._draw_adc(self.last_adc_item)

    def _compute_mti_live(self):
        """
        3-Pulse MTI Filtresi (Double Canceller):
        y[n] = x[n] - 2*x[n-1] + x[n-2]
        """
        if not hasattr(self, "_live_adc_history") or len(self._live_adc_history) == 0:
            return None

        if len(self._live_adc_history) >= 3:
            x_n = np.asarray(self._live_adc_history[-1], dtype=np.float64)
            x_n1 = np.asarray(self._live_adc_history[-2], dtype=np.float64)
            x_n2 = np.asarray(self._live_adc_history[-3], dtype=np.float64)
            min_len = min(len(x_n), len(x_n1), len(x_n2))
            return x_n[:min_len] - 2.0 * x_n1[:min_len] + x_n2[:min_len]
        elif len(self._live_adc_history) == 2:
            x_n = np.asarray(self._live_adc_history[-1], dtype=np.float64)
            x_n1 = np.asarray(self._live_adc_history[-2], dtype=np.float64)
            min_len = min(len(x_n), len(x_n1))
            return x_n[:min_len] - x_n1[:min_len]
        else:
            x_n = np.asarray(self._live_adc_history[-1], dtype=np.float64)
            return np.zeros_like(x_n)

    def _compute_iir_bg_live(self, alpha=0.10):
        """
        IIR / Exponential Background Removal:
        b_k[n] = (1-alpha)*b_{k-1}[n] + alpha*x_k[n]
        y_k[n] = x_k[n] - b_k[n]
        """
        if not hasattr(self, "_live_adc_history") or len(self._live_adc_history) == 0:
            return None

        frames = [np.asarray(f, dtype=np.float64) for f in self._live_adc_history]
        min_len = min(len(f) for f in frames)
        if min_len == 0:
            return None

        bg = frames[0][:min_len].copy()
        for f in frames[1:]:
            bg = (1.0 - alpha) * bg + alpha * f[:min_len]

        latest = frames[-1][:min_len]
        return latest - bg

    def _draw_adc(self, item):
        c = self.adc_canvas
        c.delete("plot_elem")
        c.delete("hover_overlay")
        w = c.winfo_width()
        h = c.winfo_height()

        if w < 50 or h < 50:
            return

        # Izgara Çizimi (Açık Gri Temiz Çizgiler)
        for x in range(0, w, 60):
            c.create_line(x, 0, x, h, fill="#f1f3f5", width=1, tags="plot_elem")
        for y in range(0, h, 40):
            c.create_line(0, y, w, y, fill="#f1f3f5", width=1, tags="plot_elem")

        if item is not None and "adc" in item:
            data = item["adc"]
            if len(data) > 1:
                data_arr = np.asarray(data, dtype=np.float64)

                # 1. Mod: Ham ADC / 2. Mod: MTI Filtresi / 3. Mod: IIR Arka Plan
                if self.view_mode == "waveform":
                    plot_vals = data_arr
                    line_color = PALETTE["accent_blue"]
                    mode_title = "Ham ADC"
                elif self.view_mode == "mti":
                    mti_vals = self._compute_mti_live()
                    if mti_vals is None:
                        mti_vals = np.zeros_like(data_arr)
                    plot_vals = mti_vals
                    line_color = PALETTE["accent_purple"]
                    mode_title = "MTI (y[n]=x[n]-2x[n-1]+x[n-2])"
                elif self.view_mode == "iir_bg":
                    iir_vals = self._compute_iir_bg_live(alpha=0.10)
                    if iir_vals is None:
                        iir_vals = np.zeros_like(data_arr)
                    plot_vals = iir_vals
                    line_color = PALETTE["accent_green"]
                    mode_title = "IIR Arka Plan (y=x-b, α=0.10)"
                else:
                    plot_vals = data_arr
                    line_color = PALETTE["accent_blue"]
                    mode_title = "Ham ADC"

                n_samples = len(plot_vals)
                min_v = float(np.min(plot_vals))
                max_v = float(np.max(plot_vals))
                mean_v = float(np.mean(plot_vals))
                rms_v = float(np.sqrt(np.mean(plot_vals**2)))
                pp_v = max_v - min_v

                # Y-Ölçeklendirme (Tüm modlar için ortak ölçek)
                if self.y_scale_mode == "fixed_25k":
                    y_min, y_max = -25000.0, 25000.0
                elif self.y_scale_mode == "fixed_10k":
                    y_min, y_max = -10000.0, 10000.0
                else:
                    margin = max(abs(min_v), abs(max_v)) * 0.15 + 100.0
                    limit = max(abs(min_v), abs(max_v)) + margin
                    y_min, y_max = -limit, limit

                rng = max(y_max - y_min, 1.0)
                pad_y = 15

                # Sıfır Referans Çizgisi
                zero_y = int(h - ((0.0 - y_min) / rng * (h - 2 * pad_y) + pad_y))
                c.create_line(0, zero_y, w, zero_y, fill="#ced4da", width=1, dash=(4, 4), tags="plot_elem")

                points = []
                for i, val in enumerate(plot_vals):
                    x = int((i / (n_samples - 1)) * (w - 20) + 10)
                    y = int(h - ((val - y_min) / rng * (h - 2 * pad_y) + pad_y))
                    points.extend([x, y])

                if len(points) >= 4:
                    c.create_line(points, fill=line_color, width=2, smooth=False, tags="plot_elem")

                self.adc_stats.config(
                    text=f"{mode_title} │ Min: {min_v:+.0f} │ Max: {max_v:+.0f} │ P-P: {pp_v:.0f} │ Ort: {mean_v:+.1f} │ RMS: {rms_v:.0f} │ {n_samples} smp"
                )
                return

        c.create_text(w // 2, h // 2, text="Veri akışı bekleniyor...", fill=PALETTE["fg_dim"], font=("Segoe UI", 12), tags="plot_elem")
        self.adc_stats.config(text="Min: - │ Max: - │ P-P: - │ Ort: - │ RMS: - │ Frame: -")

    # Sabit Sağ Üst Köşe İmleç Göstergesi
    def _on_adc_canvas_hover(self, event):
        if self.last_adc_item is None or "adc" not in self.last_adc_item:
            return
        data = self.last_adc_item["adc"]
        if data is None or len(data) == 0:
            return

        c = self.adc_canvas
        w = c.winfo_width()
        h = c.winfo_height()
        if w <= 30 or h <= 30:
            return

        c.delete("hover_overlay")

        data_arr = np.asarray(data, dtype=np.float64)

        if self.view_mode == "waveform":
            plot_vals = data_arr
            label_prefix = "ADC Genlik"
        elif self.view_mode == "mti":
            plot_vals = self._compute_mti_live()
            if plot_vals is None:
                plot_vals = np.zeros_like(data_arr)
            label_prefix = "MTI Genlik"
        elif self.view_mode == "iir_bg":
            plot_vals = self._compute_iir_bg_live(alpha=0.10)
            if plot_vals is None:
                plot_vals = np.zeros_like(data_arr)
            label_prefix = "IIR Genlik"
        else:
            plot_vals = data_arr
            label_prefix = "ADC Genlik"

        n_samples = len(plot_vals)
        if n_samples <= 1:
            return

        bin_idx = int(((event.x - 10) / (w - 20)) * n_samples)
        bin_idx = max(0, min(n_samples - 1, bin_idx))
        val = plot_vals[bin_idx]

        pt_x = int((bin_idx / (n_samples - 1)) * (w - 20) + 10)

        # Y konumu (Ham ADC, MTI ve IIR için ortak ölçek)
        if self.y_scale_mode == "fixed_25k":
            y_min, y_max = -25000.0, 25000.0
        elif self.y_scale_mode == "fixed_10k":
            y_min, y_max = -10000.0, 10000.0
        else:
            min_v = float(np.min(plot_vals))
            max_v = float(np.max(plot_vals))
            margin = max(abs(min_v), abs(max_v)) * 0.15 + 100.0
            limit = max(abs(min_v), abs(max_v)) + margin
            y_min, y_max = -limit, limit

        rng = max(y_max - y_min, 1.0)
        pad_y = 15
        pt_y = int(h - ((val - y_min) / rng * (h - 2 * pad_y) + pad_y))

        # Dikey kılavuz çizgisi ve nokta
        c.create_line(pt_x, 0, pt_x, h, fill="#fab005", width=1, dash=(3, 3), tags="hover_overlay")
        c.create_oval(pt_x - 4, pt_y - 4, pt_x + 4, pt_y + 4, fill=PALETTE["accent_yellow"], outline="#c76700", width=1, tags="hover_overlay")

        # Sabit Sağ Üst Köşe Bilgi Metni
        tooltip_txt = f"Bin #{bin_idx}  │  {label_prefix}: {val:+.1f}"
        c.create_text(w - 15, 15, text=tooltip_txt, anchor=tk.NE,
                      fill=PALETTE["accent_blue"], font=("Consolas", 10, "bold"), tags="hover_overlay")

    def _clear_canvas_hover(self):
        self.adc_canvas.delete("hover_overlay")

    def _on_bottom_container_resize(self, event=None):
        if event:
            bh = event.height
        else:
            bh = self.bottom_container.winfo_height()

        if bh < 60:
            return

        vh = max(bh - 46, 60)
        vw = int(vh * 16 / 9)

        self.cam_panel.config(width=vw + 24)
        self._cam_target_w = vw
        self._cam_target_h = vh

    def _show_frame(self, frame):
        if frame is None:
            return
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        tw = getattr(self, "_cam_target_w", 480)
        th = getattr(self, "_cam_target_h", 270)

        img = Image.fromarray(rgb)
        img = img.resize((tw, th), Image.Resampling.BILINEAR)

        imgtk = ImageTk.PhotoImage(image=img)
        self.cam_label.imgtk = imgtk
        self.cam_label.config(image=imgtk)

    # -------------------------------------------------------------------------
    # GÜVENLİ KAPATMA
    # -------------------------------------------------------------------------
    def on_close(self):
        if self.recording:
            self._stop_record()
        self._stop_adc_streaming()
        self.camera.stop()
        self.root.destroy()


# =============================================================================
# BAŞLATMA
# =============================================================================
def launch_collector(fallower_system=None):
    root = tk.Tk()
    app = DataCollectorApp(root, fallower_system=fallower_system)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()


if __name__ == "__main__":
    print(f"🚀 FALLOWER Veri Toplama Stüdyosu başlatılıyor. Hedef Radar FPS: {DEFAULT_RADAR_FPS}")
    launch_collector(fallower_system=None)