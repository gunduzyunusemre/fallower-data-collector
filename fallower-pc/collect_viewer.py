"""
=============================================================================
FALLOWER — Radar Veri Görüntüleme & Etiketleme Arayüzü (Studio Pro)
=============================================================================
- Modern Beyaz / Aydınlık Tema (Clean Modern Light Theme)
- Gerçek Donanım Zaman Damgaları (Timestamps) Bazlı Hassas Oynatma & Senkronizasyon
- Titremesiz (Flicker-Free) İnteraktif Radar ADC & MTI Filtresi Görselleştirme (y[n]=x[n]-2x[n-1]+x[n-2])
- Yeni Etiket İsimlendirme Formatı: {dosyaadi}_{etiketadi}
- İnteraktif Zaman Çizelgesi, Dinamik Aralık İşaretleme ve Canlı Canvas Tooltip
- Kırpılmış Etiket İnceleme & Yönetim (Oynatma, Filtreleme, Silme)
- Klavye Kısayolları ile Hızlı Operatör Kontrolü
=============================================================================
"""

import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import cv2
import numpy as np
import os
import json
import time
import re
import shutil
from datetime import datetime
from PIL import Image, ImageTk

# Scipy Sinyal İşleme Kütüphanesi
try:
    from scipy.signal import butter, filtfilt, find_peaks, fftconvolve
    HAS_SCIPY = True
except Exception:
    HAS_SCIPY = False

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


# =============================================================================
# VARSAYILAN SABİTLER (Metadata bulunamazsa kullanılır)
# =============================================================================
DEFAULT_RADAR_PERIOD_MS = 50.0
DEFAULT_VIDEO_FPS = 20.0

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
    "accent_green": "#2b8a3e",   # Başarı / Başlangıç / Özel Filtre (Emerald Green)
    "accent_red": "#d6336c",     # Hata / Bitiş (Crimson / Rose)
    "accent_yellow": "#e67700",  # Uyarı / Zaman Çizelgesi (Amber)
    "accent_purple": "#7048e8",  # MTI / Vurgu (Violet)
    "btn_bg": "#e9ecef",         # Standart Buton
    "btn_hover": "#dee2e6",      # Buton Üzerine Gelince
}


# =============================================================================
# YARDIMCI WIDGET: Modern Hover Buton
# =============================================================================
class ModernButton(tk.Button):
    def __init__(self, master, text="", command=None, bg_color=None, fg_color=None,
                 hover_bg=None, font=("Segoe UI", 9, "bold"), padx=10, pady=4, **kwargs):
        # kwargs içinden bg/fg parametrelerini güvenle ayıkla
        bg_kw = kwargs.pop("bg", None) or kwargs.pop("background", None)
        fg_kw = kwargs.pop("fg", None) or kwargs.pop("foreground", None)

        self.normal_bg = bg_color or bg_kw or PALETTE["btn_bg"]
        self.normal_fg = fg_color or fg_kw or PALETTE["fg_text"]
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
# ANA UYGULAMA SINIFI
# =============================================================================
class DataViewerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("⚡ FALLOWER Radar Veri Stüdyosu Pro")
        self.root.geometry("1400x920")
        self.root.minsize(1100, 750)
        self.root.configure(bg=PALETTE["bg_main"])

        # Yüklenen veri durumu
        self.record_dir = None
        self.metadata = {}
        self.adc_data = None            # object array: her eleman bir ADC frame
        self.adc_meta = None            # [N, 2]: [stm32_rel, pc_rel]
        self.camera_ts = None           # [M]: ham PC epoch
        self.camera_ts_rel = None       # [M]: pc referansına göre göreceli
        self.frame_timestamps = None    # [total_frames]: Her frame'in saniye cinsinden göreceli zaman damgası
        self.cap = None                 # cv2.VideoCapture
        self.video_path = None
        self.video_fps = DEFAULT_VIDEO_FPS
        self.video_frame_count = 0
        self.radar_period_ms = DEFAULT_RADAR_PERIOD_MS
        self.radar_period_s = DEFAULT_RADAR_PERIOD_MS / 1000.0
        self.total_frames = 0
        self.duration = 0.0
        self.ref_pc = 0.0

        # Oynatma & Akış Durumu
        self.playing = False
        self.current_frame_idx = 0
        self._last_cap_frame_idx = -1   # Son okunan video frame index'i (hızlı sıralı okuma için)
        self._last_tick = None
        self._slider_updating = False
        self.playback_speed = 1.0
        self.loop_playback = False

        # Görselleştirme Ayarları & Titreme Önleme
        self.view_mode = "waveform"     # "waveform" veya "mti"
        self.label_view_mode = "waveform" # "waveform" veya "mti"
        self.y_scale_mode = "auto"      # "auto", "fixed_25k", "fixed_10k"
        self._last_canvas_w = 0
        self._last_canvas_h = 0

        # Etiketleme Durumu
        self.mark_start = None          # Frame index
        self.mark_end = None            # Frame index

        # Veri Seti & Oturum Listesi Durumu
        self.dataset_dir = None
        self.dataset_recordings = []
        self.filtered_recordings = []
        self.selected_recording_info = None

        # Etiket İnceleme Modu & Veri Bölme / Çoğaltma
        self.label_base_dir = None
        self.label_list = []
        self.current_label_idx = None
        self.current_label_adc = None
        self.current_label_meta = None
        self.current_label_info = None
        self.current_label_timestamps = None
        self.current_label_video_path = None
        self.current_label_cap = None
        self.current_label_frame_count = 0
        self.current_label_radar_period_ms = DEFAULT_RADAR_PERIOD_MS
        self.label_playing = False
        self.label_current_frame_idx = 0
        self._last_label_cap_frame_idx = -1
        self._label_last_tick = None
        self.label_mark_start = None
        self.label_mark_end = None
        self.label_model_input_size = tk.IntVar(value=20)
        self.label_stride_size = tk.IntVar(value=1)

        self._build_ui()
        self._bind_keyboard_shortcuts()
        self._show_selector()

    # -------------------------------------------------------------------------
    # UI İNŞASI
    # -------------------------------------------------------------------------
    def _build_ui(self):
        self.main_container = tk.Frame(self.root, bg=PALETTE["bg_main"])
        self.main_container.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

        self._build_selector_ui()
        self._build_viewer_ui()
        self._build_label_viewer_ui()

    # --- 1. SEÇİCİ EKRANI ---
    def _build_selector_ui(self):
        self.selector_frame = tk.Frame(self.main_container, bg=PALETTE["bg_card"],
                                       highlightthickness=1, highlightbackground=PALETTE["border"])

        # Başlık Bölümü
        header_frame = tk.Frame(self.selector_frame, bg=PALETTE["bg_card"])
        header_frame.pack(fill=tk.X, pady=(20, 10))

        tk.Label(header_frame, text="⚡ FALLOWER RADAR VERİ STÜDYOSU",
                 font=("Segoe UI", 20, "bold"), bg=PALETTE["bg_card"],
                 fg=PALETTE["accent_blue"]).pack()
        tk.Label(header_frame, text="Yüksek Hassasiyetli Radar & Video Senkron İnceleme ve Etiketleme Platformu",
                 font=("Segoe UI", 10), bg=PALETTE["bg_card"], fg=PALETTE["fg_muted"]).pack(pady=2)

        # Kartlar Konteyneri
        cards_container = tk.Frame(self.selector_frame, bg=PALETTE["bg_card"])
        cards_container.pack(fill=tk.BOTH, expand=True, padx=30, pady=(0, 10))

        # Sol Kart: Veri Seti & Oturum Seçici (Genişletilmiş Oturum Listeli)
        card_full = tk.Frame(cards_container, bg=PALETTE["bg_panel"],
                             highlightthickness=1, highlightbackground=PALETTE["border"], padx=20, pady=16)
        card_full.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 10))

        tk.Label(card_full, text="📂  VERİ SETİ & HAM KAYIT İNCELEME",
                 font=("Segoe UI", 13, "bold"), bg=PALETTE["bg_panel"], fg=PALETTE["accent_green"]).pack(anchor=tk.W)
        tk.Label(card_full, text="Bir veri seti klasörü veya tekil kayıt klasörü seçin. Oturumlar aşağıda listelenecektir.",
                 font=("Segoe UI", 9), bg=PALETTE["bg_panel"], fg=PALETTE["fg_muted"], justify=tk.LEFT).pack(anchor=tk.W, pady=(2, 8))

        # Butonlar Satırı
        btn_box = tk.Frame(card_full, bg=PALETTE["bg_panel"])
        btn_box.pack(fill=tk.X, pady=(0, 6))

        ModernButton(btn_box, text="📁  Veri Seti / Kayıt Klasörü Seç...", command=self._browse_folder,
                     bg_color=PALETTE["accent_green"], fg_color="#ffffff",
                     hover_bg="#237032", font=("Segoe UI", 10, "bold"), pady=6).pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 4))

        if os.path.isdir("FALLOWER DATASETS"):
            ModernButton(btn_box, text="⚡ 'FALLOWER DATASETS' Yükle", command=lambda: self._scan_and_display_dataset("FALLOWER DATASETS"),
                         bg_color=PALETTE["btn_bg"], fg_color=PALETTE["accent_cyan"],
                         hover_bg=PALETTE["btn_hover"], font=("Segoe UI", 9, "bold"), pady=6).pack(side=tk.LEFT, padx=(4, 0))

        self.lbl_selected_path = tk.Label(card_full, text="Henüz klasör seçilmedi.",
                                          font=("Consolas", 8), bg=PALETTE["bg_panel"], fg=PALETTE["fg_dim"], wraplength=480, justify=tk.LEFT)
        self.lbl_selected_path.pack(anchor=tk.W, pady=(0, 6))

        # Oturum Arama / Filtre
        filter_box = tk.Frame(card_full, bg=PALETTE["bg_panel"])
        filter_box.pack(fill=tk.X, pady=(0, 4))
        tk.Label(filter_box, text="🔍", bg=PALETTE["bg_panel"], fg=PALETTE["fg_muted"]).pack(side=tk.LEFT)
        self.entry_rec_search = tk.Entry(filter_box, bg=PALETTE["bg_card"], fg=PALETTE["fg_text"],
                                         insertbackground=PALETTE["fg_text"], relief=tk.SOLID, bd=1, font=("Segoe UI", 9))
        self.entry_rec_search.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=4)
        self.entry_rec_search.bind("<KeyRelease>", lambda e: self._filter_dataset_recordings())

        # Oturumlar Listbox (Kaydırma Çubuklu)
        list_frame = tk.Frame(card_full, bg=PALETTE["bg_panel"])
        list_frame.pack(fill=tk.BOTH, expand=True, pady=(2, 6))

        scrollbar = tk.Scrollbar(list_frame, orient=tk.VERTICAL)
        self.rec_listbox = tk.Listbox(list_frame, bg=PALETTE["bg_card"], fg=PALETTE["fg_text"],
                                      selectbackground=PALETTE["accent_blue"], selectforeground="#ffffff",
                                      font=("Consolas", 9), highlightthickness=1,
                                      highlightbackground=PALETTE["border"], bd=0, yscrollcommand=scrollbar.set)
        scrollbar.config(command=self.rec_listbox.yview)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.rec_listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self.rec_listbox.bind("<<ListboxSelect>>", self._on_record_select)
        self.rec_listbox.bind("<Double-Button-1>", lambda e: self._open_selected_recording())

        # Seçili Oturum Bilgi Kutusu
        self.lbl_rec_info_box = tk.Label(card_full, text="Listeden bir oturuma tıklayın veya çift tıklayarak açın.",
                                         font=("Segoe UI", 9), bg=PALETTE["bg_card"], fg=PALETTE["fg_muted"],
                                         padx=8, pady=4, relief=tk.SOLID, bd=1, anchor=tk.W, justify=tk.LEFT)
        self.lbl_rec_info_box.pack(fill=tk.X, pady=(0, 6))

        # Açma Butonu
        self.btn_open_rec = ModernButton(card_full, text="🚀  Seçili Kaydı Aç & İncele  [Çift Tıkla]",
                                         command=self._open_selected_recording,
                                         bg_color=PALETTE["accent_blue"], fg_color="#ffffff",
                                         hover_bg="#1971c2", font=("Segoe UI", 10, "bold"), pady=6)
        self.btn_open_rec.pack(fill=tk.X)

        self.selector_status = tk.Label(card_full, text="", font=("Segoe UI", 9),
                                        bg=PALETTE["bg_panel"], fg=PALETTE["accent_red"])
        self.selector_status.pack(pady=2)

        # Sağ Kart: Etiketlenmiş Veri İnceleme
        card_labels = tk.Frame(cards_container, bg=PALETTE["bg_panel"],
                               highlightthickness=1, highlightbackground=PALETTE["border"], padx=20, pady=16)
        card_labels.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(10, 0))

        tk.Label(card_labels, text="🔖  ETİKETLENMİŞ KESİTLERİ İNCELEME",
                 font=("Segoe UI", 13, "bold"), bg=PALETTE["bg_panel"], fg=PALETTE["accent_yellow"]).pack(anchor=tk.W)
        tk.Label(card_labels, text="Daha önce kırpılmış ve etiketlenmiş 'labels' klasörünü açarak kesitleri bağımsız denetleyin.",
                 font=("Segoe UI", 9), bg=PALETTE["bg_panel"], fg=PALETTE["fg_muted"], justify=tk.LEFT).pack(anchor=tk.W, pady=(2, 8))

        lbl_desc_box = tk.LabelFrame(card_labels, text=" Özellikler ", bg=PALETTE["bg_panel"],
                                     fg=PALETTE["accent_yellow"], font=("Segoe UI", 9, "bold"), bd=1)
        lbl_desc_box.pack(fill=tk.BOTH, expand=True, pady=(0, 10))
        tk.Label(lbl_desc_box, text="• Otomatik Etiket Listeleme & Filtreleme\n• Kırpılmış Video & ADC Senkron Oynatma\n• MTI Filtresi & Zaman Bölgesi Analizi\n• Hatalı Etiketleri Silme / Düzeltme Desteği\n• JSON Metadata Denetçisi",
                 font=("Segoe UI", 9), bg=PALETTE["bg_panel"], fg=PALETTE["fg_text"], justify=tk.LEFT).pack(anchor=tk.NW, padx=10, pady=10)

        ModernButton(card_labels, text="📂  'labels' Klasörünü Seç...", command=self._browse_label_folder,
                     bg_color=PALETTE["accent_yellow"], fg_color="#ffffff",
                     hover_bg="#c76700", font=("Segoe UI", 10, "bold"), pady=8).pack(fill=tk.X)

        # Alt Bilgi Barı
        footer_frame = tk.Frame(self.selector_frame, bg=PALETTE["bg_card"])
        footer_frame.pack(fill=tk.X, side=tk.BOTTOM, pady=10, padx=30)
        tk.Label(footer_frame, text="Kısayollar: [Boşluk] Oynat/Duraklat  |  [←/→] Frame İlerle/Geri  |  [I/O] Başlangıç/Bitiş İşaretle  |  [S] Etiketi Kaydet",
                 font=("Segoe UI", 9), bg=PALETTE["bg_card"], fg=PALETTE["fg_dim"]).pack()

    # --- 2. ANA GÖRÜNTÜLEYİCİ EKRANI ---
    def _build_viewer_ui(self):
        self.viewer_frame = tk.Frame(self.main_container, bg=PALETTE["bg_main"])

        # Üst Bilgi Barı
        top_bar = tk.Frame(self.viewer_frame, bg=PALETTE["bg_card"], height=48,
                           highlightthickness=1, highlightbackground=PALETTE["border"])
        top_bar.pack(fill=tk.X, pady=(0, 6))
        top_bar.pack_propagate(False)

        ModernButton(top_bar, text="◀  Ana Menü", command=self._back_to_selector,
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"],
                     hover_bg=PALETTE["btn_hover"], font=("Segoe UI", 9, "bold")).pack(side=tk.LEFT, padx=10, pady=8)

        self.lbl_record_title = tk.Label(top_bar, text="Kayıt: -", font=("Segoe UI", 11, "bold"),
                                         bg=PALETTE["bg_card"], fg=PALETTE["accent_blue"])
        self.lbl_record_title.pack(side=tk.LEFT, padx=15)

        self.badge_sync_status = tk.Label(top_bar, text="Senkron: Normal", font=("Segoe UI", 9, "bold"),
                                          bg="#e6fcf5", fg=PALETTE["accent_green"], padx=8, pady=2)
        self.badge_sync_status.pack(side=tk.RIGHT, padx=10)

        self.lbl_record_details = tk.Label(top_bar, text="", font=("Consolas", 9),
                                           bg=PALETTE["bg_card"], fg=PALETTE["fg_muted"])
        self.lbl_record_details.pack(side=tk.RIGHT, padx=15)

        # 1. Üst Bölüm: Radar Sinyal Ekranı (Boydan Boya Uzunlamasına)
        self.radar_panel = tk.Frame(self.viewer_frame, bg=PALETTE["bg_card"],
                                    highlightthickness=1, highlightbackground=PALETTE["border"])
        self.radar_panel.pack(fill=tk.BOTH, expand=True, pady=(0, 6))

        # Radar Kontrol Başlığı
        radar_header = tk.Frame(self.radar_panel, bg=PALETTE["bg_card"])
        radar_header.pack(fill=tk.X, padx=12, pady=(8, 4))

        tk.Label(radar_header, text="📡  RADAR SİNYAL ANALİZİ", font=("Segoe UI", 11, "bold"),
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

        # Radar Canvas (Beyaz Arka Plan & Keskin Çizim)
        self.adc_canvas = tk.Canvas(self.radar_panel, bg=PALETTE["bg_canvas"],
                                    highlightthickness=1, highlightbackground=PALETTE["border"], cursor="crosshair")
        self.adc_canvas.pack(fill=tk.BOTH, expand=True, padx=12, pady=4)
        self.adc_canvas.bind("<Configure>", lambda e: self._on_canvas_resize())
        self.adc_canvas.bind("<Motion>", self._on_adc_canvas_hover)
        self.adc_canvas.bind("<Leave>", lambda e: self._clear_canvas_hover())

        # Radar İstatistik & Bilgi Çubuğu (Sabit Konum - Titreme Yapmaz)
        self.adc_stats_frame = tk.Frame(self.radar_panel, bg=PALETTE["bg_panel"], height=32)
        self.adc_stats_frame.pack(fill=tk.X, padx=12, pady=(0, 8))
        self.adc_stats_frame.pack_propagate(False)

        self.adc_stats = tk.Label(self.adc_stats_frame,
                                  text="Min: -  |  Max: -  |  P-P: -  |  Ort: -  |  RMS: -  |  Örnek: -",
                                  bg=PALETTE["bg_panel"], fg=PALETTE["fg_muted"], font=("Consolas", 9))
        self.adc_stats.pack(side=tk.LEFT, padx=8, pady=4)

        # 2. Alt Bölüm: Kamera (Sol Alt - Tam 16:9 Sıfır Boşluk) + Kontroller (Sağ Alt - Kalan Alanı Doldurur)
        self.bottom_container = tk.Frame(self.viewer_frame, bg=PALETTE["bg_main"])
        self.bottom_container.pack(fill=tk.BOTH, expand=True)

        # Sol Alt Panel: Video Ekranı (16:9 oranına kilitli genişlik, kenarda boşluk bırakmaz)
        self.video_panel = tk.Frame(self.bottom_container, bg=PALETTE["bg_card"],
                                    highlightthickness=1, highlightbackground=PALETTE["border"], width=460)
        self.video_panel.pack(side=tk.LEFT, fill=tk.Y, expand=False, padx=(0, 4))
        self.video_panel.pack_propagate(False)

        video_header = tk.Frame(self.video_panel, bg=PALETTE["bg_card"])
        video_header.pack(fill=tk.X, padx=12, pady=(8, 4))
        tk.Label(video_header, text="📷  KAMERA SENKRON VİDEOSU", font=("Segoe UI", 11, "bold"),
                 bg=PALETTE["bg_card"], fg=PALETTE["accent_blue"]).pack(side=tk.LEFT)

        self.lbl_video_fps_badge = tk.Label(video_header, text="FPS: -", font=("Segoe UI", 9, "bold"),
                                            bg=PALETTE["btn_bg"], fg=PALETTE["accent_cyan"], padx=6, pady=1)
        self.lbl_video_fps_badge.pack(side=tk.RIGHT)

        # Sabit Video Konteyneri
        self.video_container = tk.Frame(self.video_panel, bg="#000000")
        self.video_container.pack(fill=tk.BOTH, expand=True, padx=12, pady=(4, 8))
        self.video_container.pack_propagate(False)

        self.cam_label = tk.Label(self.video_container, bg="#000000")
        self.cam_label.place(x=0, y=0, relwidth=1.0, relheight=1.0)

        # Sağ Alt Panel: Oynatma & Etiketleme Kontrolleri (Genişleyen Panel)
        right_controls = tk.Frame(self.bottom_container, bg=PALETTE["bg_main"])
        right_controls.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(4, 0))

        self.bottom_container.bind("<Configure>", self._on_bottom_container_resize)

        # Oynatma & Zaman Kontrolü Kartı
        control_card = tk.Frame(right_controls, bg=PALETTE["bg_card"],
                                highlightthickness=1, highlightbackground=PALETTE["border"], padx=12, pady=10)
        control_card.pack(fill=tk.BOTH, expand=True, pady=(0, 6))

        # Oynatma Butonları Satırı
        btn_row = tk.Frame(control_card, bg=PALETTE["bg_card"])
        btn_row.pack(fill=tk.X, pady=(0, 6))

        self.btn_play = ModernButton(btn_row, text="▶  OYNAT", command=self._toggle_play,
                                     bg_color=PALETTE["accent_green"], fg_color="#ffffff",
                                     hover_bg="#237032", font=("Segoe UI", 10, "bold"), width=12)
        self.btn_play.pack(side=tk.LEFT, padx=3)

        ModernButton(btn_row, text="⏮ Başa", command=lambda: self._seek_to_frame(0),
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"]).pack(side=tk.LEFT, padx=2)
        ModernButton(btn_row, text="◀ -10", command=lambda: self._seek_relative(-10),
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"]).pack(side=tk.LEFT, padx=2)
        ModernButton(btn_row, text="◀ -1", command=lambda: self._seek_relative(-1),
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"]).pack(side=tk.LEFT, padx=2)
        ModernButton(btn_row, text="+1 ▶", command=lambda: self._seek_relative(1),
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"]).pack(side=tk.LEFT, padx=2)
        ModernButton(btn_row, text="+10 ▶", command=lambda: self._seek_relative(10),
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"]).pack(side=tk.LEFT, padx=2)
        ModernButton(btn_row, text="Sona ⏭", command=lambda: self._seek_to_frame(self.total_frames - 1),
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"]).pack(side=tk.LEFT, padx=2)

        self.lbl_time_display = tk.Label(btn_row, text="Frame #0 / #0  │  0.000s / 0.000s",
                                         font=("Consolas", 10, "bold"), bg=PALETTE["bg_card"], fg=PALETTE["accent_blue"])
        self.lbl_time_display.pack(side=tk.RIGHT, padx=5)

        # Hız Seçici & Döngü Satırı
        speed_row = tk.Frame(control_card, bg=PALETTE["bg_card"])
        speed_row.pack(fill=tk.X, pady=(2, 6))

        tk.Label(speed_row, text="Hız:", font=("Segoe UI", 9, "bold"),
                 bg=PALETTE["bg_card"], fg=PALETTE["fg_muted"]).pack(side=tk.LEFT, padx=(4, 4))
        self.speed_buttons = {}
        for spd in [0.25, 0.5, 1.0, 1.5, 2.0]:
            btn_spd = ModernButton(speed_row, text=f"{spd}x", command=lambda s=spd: self._set_speed(s),
                                   bg_color=PALETTE["accent_blue"] if spd == 1.0 else PALETTE["btn_bg"],
                                   fg_color="#ffffff" if spd == 1.0 else PALETTE["fg_text"],
                                   padx=6, pady=2, font=("Segoe UI", 8, "bold"))
            btn_spd.pack(side=tk.LEFT, padx=1)
            self.speed_buttons[spd] = btn_spd

        self.loop_var = tk.BooleanVar(value=False)
        chk_loop = tk.Checkbutton(speed_row, text="🔁 Döngü (Loop)", variable=self.loop_var,
                                  command=self._on_loop_toggle, bg=PALETTE["bg_card"], fg=PALETTE["fg_text"],
                                  selectcolor=PALETTE["bg_panel"], activebackground=PALETTE["bg_card"],
                                  activeforeground=PALETTE["fg_text"], font=("Segoe UI", 9))
        chk_loop.pack(side=tk.LEFT, padx=15)

        # Zaman Çizelgesi Slider & Canvas
        self.frame_var = tk.IntVar(value=0)
        self.frame_slider = tk.Scale(
            control_card, from_=0, to=1, orient=tk.HORIZONTAL,
            variable=self.frame_var, command=self._on_slider_change,
            bg=PALETTE["bg_card"], fg=PALETTE["fg_text"], troughcolor=PALETTE["bg_panel"],
            highlightthickness=0, activebackground=PALETTE["accent_blue"],
            length=600, showvalue=0, bd=0
        )
        self.frame_slider.pack(fill=tk.X, padx=5, pady=(2, 4))

        # İnteraktif Timeline Canvas (Bölge Vurgusu)
        self.timeline_canvas = tk.Canvas(control_card, height=20, bg=PALETTE["bg_panel"],
                                         highlightthickness=1, highlightbackground=PALETTE["border"], cursor="hand2")
        self.timeline_canvas.pack(fill=tk.X, padx=5, pady=(0, 2))
        self.timeline_canvas.bind("<Configure>", lambda e: self._draw_timeline())
        self.timeline_canvas.bind("<Button-1>", self._on_timeline_click)
        self.timeline_canvas.bind("<B1-Motion>", self._on_timeline_click)

        # Etiketleme Paneli (Marking & Slicing)
        label_card = tk.Frame(right_controls, bg=PALETTE["bg_card"],
                              highlightthickness=1, highlightbackground=PALETTE["border"], padx=12, pady=10)
        label_card.pack(fill=tk.X)

        label_header = tk.Frame(label_card, bg=PALETTE["bg_card"])
        label_header.pack(fill=tk.X, pady=(0, 6))
        tk.Label(label_header, text="🏷️  ARALIK ETİKETLEME", font=("Segoe UI", 10, "bold"),
                 bg=PALETTE["bg_card"], fg=PALETTE["accent_yellow"]).pack(side=tk.LEFT)

        label_row = tk.Frame(label_card, bg=PALETTE["bg_card"])
        label_row.pack(fill=tk.X, pady=(2, 6))

        ModernButton(label_row, text="🟢  Başlangıç [I]", command=self._mark_start_point,
                     bg_color="#e6fcf5", fg_color=PALETTE["accent_green"], hover_bg="#c3fae8",
                     font=("Segoe UI", 9, "bold")).pack(side=tk.LEFT, padx=3)

        ModernButton(label_row, text="🔴  Bitiş [O]", command=self._mark_end_point,
                     bg_color="#fff0f6", fg_color=PALETTE["accent_red"], hover_bg="#ffdeeb",
                     font=("Segoe UI", 9, "bold")).pack(side=tk.LEFT, padx=3)

        ModernButton(label_row, text="✖ Temizle [Esc]", command=self._clear_marks,
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_muted"]).pack(side=tk.LEFT, padx=6)

        self.lbl_marks_summary = tk.Label(label_row, text="Aralık: -  │  Süre: -  │  ADC Frame: -",
                                          font=("Consolas", 9), bg=PALETTE["bg_card"], fg=PALETTE["fg_muted"])
        self.lbl_marks_summary.pack(side=tk.LEFT, padx=8)

        # Hızlı Etiket Seçim Butonları (Presets)
        preset_row = tk.Frame(label_card, bg=PALETTE["bg_card"])
        preset_row.pack(fill=tk.X, pady=(4, 4))

        tk.Label(preset_row, text="Hızlı Etiket:", font=("Segoe UI", 9, "bold"),
                 bg=PALETTE["bg_card"], fg=PALETTE["fg_muted"]).pack(side=tk.LEFT, padx=(4, 4))

        presets = ["fall", "walk", "getup", "getupgr", "stillnoact", "other"]
        for p in presets:
            ModernButton(preset_row, text=p, command=lambda name=p: self._set_quick_label(name),
                         bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"],
                         hover_bg="#d0ebff", font=("Segoe UI", 9, "bold"), padx=8, pady=3).pack(side=tk.LEFT, padx=2)

        # Etiket Adı ve Kaydetme Satırı
        save_row = tk.Frame(label_card, bg=PALETTE["bg_card"])
        save_row.pack(fill=tk.X, pady=(4, 2))

        tk.Label(save_row, text="Etiket Adı:", font=("Segoe UI", 9, "bold"),
                 bg=PALETTE["bg_card"], fg=PALETTE["fg_text"]).pack(side=tk.LEFT, padx=(4, 4))

        self.entry_label_name = tk.Entry(save_row, bg=PALETTE["bg_panel"], fg=PALETTE["fg_text"],
                                         insertbackground=PALETTE["fg_text"], relief=tk.SOLID, bd=1,
                                         font=("Segoe UI", 10))
        self.entry_label_name.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=4)
        self.entry_label_name.bind("<Return>", lambda e: self._save_label())

        ModernButton(save_row, text="💾  Etiketi Kes & Kaydet [S]", command=self._save_label,
                     bg_color=PALETTE["accent_yellow"], fg_color="#ffffff",
                     hover_bg="#c76700", font=("Segoe UI", 9, "bold"), padx=12).pack(side=tk.LEFT, padx=6)

        self.lbl_label_status = tk.Label(label_row, text="", font=("Segoe UI", 9, "bold"),
                                         bg=PALETTE["bg_card"], fg=PALETTE["accent_green"])
        self.lbl_label_status.pack(side=tk.RIGHT, padx=5)

    # --- 3. ETİKET İNCELEME EKRANI ---
    def _build_label_viewer_ui(self):
        self.label_viewer_frame = tk.Frame(self.main_container, bg=PALETTE["bg_main"])

        # Üst Bar
        top_bar = tk.Frame(self.label_viewer_frame, bg=PALETTE["bg_card"], height=48,
                           highlightthickness=1, highlightbackground=PALETTE["border"])
        top_bar.pack(fill=tk.X, pady=(0, 6))
        top_bar.pack_propagate(False)

        ModernButton(top_bar, text="◀  Ana Menü", command=self._show_selector,
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"],
                     hover_bg=PALETTE["btn_hover"], font=("Segoe UI", 9, "bold")).pack(side=tk.LEFT, padx=10, pady=8)

        self.lbl_label_folder_info = tk.Label(top_bar, text="Etiket Klasörü: -", font=("Segoe UI", 11, "bold"),
                                              bg=PALETTE["bg_card"], fg=PALETTE["accent_yellow"])
        self.lbl_label_folder_info.pack(side=tk.LEFT, padx=15)

        ModernButton(top_bar, text="🗑️  Seçili Etiketi Sil", command=self._delete_selected_label,
                     bg_color="#fff0f6", fg_color=PALETTE["accent_red"],
                     hover_bg="#ffdeeb", font=("Segoe UI", 9, "bold")).pack(side=tk.RIGHT, padx=10, pady=8)

        # Bölünmüş Panel (Sol Liste, Sağ Detay & Oynatıcı)
        split_frame = tk.Frame(self.label_viewer_frame, bg=PALETTE["bg_main"])
        split_frame.pack(fill=tk.BOTH, expand=True)

        # Sol Panel: Etiket Listesi & Arama
        left_panel = tk.Frame(split_frame, bg=PALETTE["bg_card"], width=320,
                              highlightthickness=1, highlightbackground=PALETTE["border"])
        left_panel.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 4))
        left_panel.pack_propagate(False)

        tk.Label(left_panel, text="📋  KAYITLI ETİKETLER", font=("Segoe UI", 11, "bold"),
                 bg=PALETTE["bg_card"], fg=PALETTE["fg_text"]).pack(pady=(12, 6), padx=10, anchor=tk.W)

        # Filtre Girişi
        filter_frame = tk.Frame(left_panel, bg=PALETTE["bg_card"])
        filter_frame.pack(fill=tk.X, padx=10, pady=(0, 6))
        tk.Label(filter_frame, text="🔍", bg=PALETTE["bg_card"], fg=PALETTE["fg_muted"]).pack(side=tk.LEFT)
        self.entry_filter = tk.Entry(filter_frame, bg=PALETTE["bg_panel"], fg=PALETTE["fg_text"],
                                     insertbackground=PALETTE["fg_text"], relief=tk.SOLID, bd=1)
        self.entry_filter.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=4)
        self.entry_filter.bind("<KeyRelease>", lambda e: self._filter_label_list())

        self.label_listbox = tk.Listbox(left_panel, bg=PALETTE["bg_panel"], fg=PALETTE["fg_text"],
                                        selectbackground=PALETTE["accent_blue"], selectforeground="#ffffff",
                                        font=("Segoe UI", 10), highlightthickness=0, bd=0, relief=tk.FLAT)
        self.label_listbox.pack(fill=tk.BOTH, expand=True, padx=10, pady=(0, 10))
        self.label_listbox.bind("<<ListboxSelect>>", self._on_label_select)

        # Sağ Panel: Kırpılmış Veri İnceleme
        right_panel = tk.Frame(split_frame, bg=PALETTE["bg_main"])
        right_panel.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(4, 0))

        # 1. Üst Bölüm: Kırpılmış Radar ADC & MTI Ekranı (Boydan Boya Uzunlamasına)
        lbl_adc_panel = tk.Frame(right_panel, bg=PALETTE["bg_card"],
                                 highlightthickness=1, highlightbackground=PALETTE["border"])
        lbl_adc_panel.pack(fill=tk.BOTH, expand=True, pady=(0, 6))

        lbl_adc_header = tk.Frame(lbl_adc_panel, bg=PALETTE["bg_card"])
        lbl_adc_header.pack(fill=tk.X, padx=12, pady=(8, 4))
        tk.Label(lbl_adc_header, text="📡  KIRPILMIŞ RADAR SİNYAL ANALİZİ", font=("Segoe UI", 11, "bold"),
                 bg=PALETTE["bg_card"], fg=PALETTE["accent_blue"]).pack(side=tk.LEFT)

        # Görünüm Modu Butonları (Ham ADC / MTI Filtresi / IIR Arka Plan)
        self.btn_lbl_mode_iir = ModernButton(lbl_adc_header, text="🌊 IIR Arka Plan",
                                             command=lambda: self._set_label_view_mode("iir_bg"),
                                             bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"],
                                             hover_bg=PALETTE["btn_hover"], font=("Segoe UI", 9, "bold"))
        self.btn_lbl_mode_iir.pack(side=tk.RIGHT, padx=3)

        self.btn_lbl_mode_mti = ModernButton(lbl_adc_header, text="📊 MTI Filtresi",
                                             command=lambda: self._set_label_view_mode("mti"),
                                             bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"],
                                             hover_bg=PALETTE["btn_hover"], font=("Segoe UI", 9, "bold"))
        self.btn_lbl_mode_mti.pack(side=tk.RIGHT, padx=3)

        self.btn_lbl_mode_raw = ModernButton(lbl_adc_header, text="📈 Ham ADC",
                                             command=lambda: self._set_label_view_mode("waveform"),
                                             bg_color="#e7f5ff", fg_color=PALETTE["accent_blue"],
                                             hover_bg="#d0ebff", font=("Segoe UI", 9, "bold"))
        self.btn_lbl_mode_raw.pack(side=tk.RIGHT, padx=3)

        self.label_adc_canvas = tk.Canvas(lbl_adc_panel, bg=PALETTE["bg_canvas"],
                                          highlightthickness=1, highlightbackground=PALETTE["border"])
        self.label_adc_canvas.pack(fill=tk.BOTH, expand=True, padx=12, pady=4)
        self.label_adc_canvas.bind("<Configure>", lambda e: self._redraw_current_label_adc())

        lbl_adc_stats_frame = tk.Frame(lbl_adc_panel, bg=PALETTE["bg_panel"], height=30)
        lbl_adc_stats_frame.pack(fill=tk.X, padx=12, pady=(0, 8))
        lbl_adc_stats_frame.pack_propagate(False)

        self.label_adc_stats = tk.Label(lbl_adc_stats_frame, text="Lütfen soldaki menüden bir etiket seçin.",
                                        bg=PALETTE["bg_panel"], fg=PALETTE["fg_muted"], font=("Consolas", 9))
        self.label_adc_stats.pack(side=tk.LEFT, padx=8, pady=4)
        # 2. Alt Bölüm: Kırpılmış Video (Sol Alt - 16:9 Sıfır Boşluk) + Etiket Oynatıcı Kontrolleri (Sağ Alt - Genişleyen)
        self.lbl_bottom_split = tk.Frame(right_panel, bg=PALETTE["bg_main"])
        self.lbl_bottom_split.pack(fill=tk.BOTH, expand=True)

        # Video Paneli (Sol Alt)
        self.lbl_video_panel = tk.Frame(self.lbl_bottom_split, bg=PALETTE["bg_card"],
                                        highlightthickness=1, highlightbackground=PALETTE["border"], width=420)
        self.lbl_video_panel.pack(side=tk.LEFT, fill=tk.Y, expand=False, padx=(0, 4))
        self.lbl_video_panel.pack_propagate(False)

        lbl_video_header = tk.Frame(self.lbl_video_panel, bg=PALETTE["bg_card"])
        lbl_video_header.pack(fill=tk.X, padx=12, pady=(8, 4))
        tk.Label(lbl_video_header, text="📷  KIRPILMIŞ VİDEO", font=("Segoe UI", 11, "bold"),
                 bg=PALETTE["bg_card"], fg=PALETTE["accent_blue"]).pack(side=tk.LEFT)

        self.label_video_container = tk.Frame(self.lbl_video_panel, bg="#000000")
        self.label_video_container.pack(fill=tk.BOTH, expand=True, padx=12, pady=(4, 8))
        self.label_video_container.pack_propagate(False)

        self.label_cam_label = tk.Label(self.label_video_container, bg="#000000")
        self.label_cam_label.place(x=0, y=0, relwidth=1.0, relheight=1.0)

        # Etiket Oynatıcı Kontrolleri & Detay Paneli (Sağ Alt - Genişleyen)
        lbl_control_card = tk.Frame(self.lbl_bottom_split, bg=PALETTE["bg_card"],
                                    highlightthickness=1, highlightbackground=PALETTE["border"], padx=12, pady=8)
        lbl_control_card.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(4, 0))

        self.lbl_bottom_split.bind("<Configure>", self._on_lbl_bottom_split_resize)

        lbl_ctrl_header = tk.Frame(lbl_control_card, bg=PALETTE["bg_card"])
        lbl_ctrl_header.pack(fill=tk.X, pady=(0, 4))
        tk.Label(lbl_ctrl_header, text="🎛️  KESİT OYNATMA & VERİ BÖLME", font=("Segoe UI", 10, "bold"),
                 bg=PALETTE["bg_card"], fg=PALETTE["accent_yellow"]).pack(side=tk.LEFT)

        # 1. Oynatma Butonları Satırı
        lbl_btn_row = tk.Frame(lbl_control_card, bg=PALETTE["bg_card"])
        lbl_btn_row.pack(fill=tk.X, pady=(0, 2))

        self.btn_label_play = ModernButton(lbl_btn_row, text="▶  OYNAT", command=self._toggle_label_play,
                                           bg_color=PALETTE["accent_green"], fg_color="#ffffff",
                                           hover_bg="#237032", font=("Segoe UI", 9, "bold"), width=10)
        self.btn_label_play.pack(side=tk.LEFT, padx=3)

        ModernButton(lbl_btn_row, text="⏮ Başa", command=lambda: self._seek_label_to_frame(0),
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"]).pack(side=tk.LEFT, padx=2)
        ModernButton(lbl_btn_row, text="◀ -1", command=lambda: self._seek_label_relative(-1),
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"]).pack(side=tk.LEFT, padx=2)
        ModernButton(lbl_btn_row, text="+1 ▶", command=lambda: self._seek_label_relative(1),
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"]).pack(side=tk.LEFT, padx=2)
        ModernButton(lbl_btn_row, text="Sona ⏭", command=lambda: self._seek_label_to_frame(len(self.current_label_adc)-1 if self.current_label_adc is not None else 0),
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_text"]).pack(side=tk.LEFT, padx=2)

        self.lbl_label_time_info = tk.Label(lbl_btn_row, text="Frame #0 / #0",
                                            font=("Consolas", 10, "bold"), bg=PALETTE["bg_card"], fg=PALETTE["accent_blue"])
        self.lbl_label_time_info.pack(side=tk.RIGHT, padx=5)

        # 2. Zaman Çizelgesi Slider
        self.label_frame_var = tk.IntVar(value=0)
        self.label_slider = tk.Scale(
            lbl_control_card, from_=0, to=0, orient=tk.HORIZONTAL, variable=self.label_frame_var,
            command=self._on_label_slider_change, bg=PALETTE["bg_card"], fg=PALETTE["fg_text"],
            troughcolor=PALETTE["bg_panel"], highlightthickness=0, activebackground=PALETTE["accent_blue"],
            showvalue=0, bd=0
        )
        self.label_slider.pack(fill=tk.X, padx=5, pady=(2, 2))

        # 3. İnteraktif Zaman Çizelgesi Canvas
        self.label_timeline_canvas = tk.Canvas(lbl_control_card, height=16, bg=PALETTE["bg_panel"],
                                               highlightthickness=1, highlightbackground=PALETTE["border"], cursor="hand2")
        self.label_timeline_canvas.pack(fill=tk.X, padx=5, pady=(0, 4))
        self.label_timeline_canvas.bind("<Configure>", lambda e: self._draw_label_timeline())
        self.label_timeline_canvas.bind("<Button-1>", self._on_label_timeline_click)
        self.label_timeline_canvas.bind("<B1-Motion>", self._on_label_timeline_click)

        # 4. Aralık İşaretleme & Bölme Paneli (Slicing & Augmentation Box)
        lbl_split_box = tk.LabelFrame(lbl_control_card, text=" ✂️ Veriyi Böl & Çoğalt (Model Boyutlu Dilimleme) ",
                                      bg=PALETTE["bg_panel"], fg=PALETTE["accent_yellow"], font=("Segoe UI", 9, "bold"),
                                      padx=8, pady=6, bd=1)
        lbl_split_box.pack(fill=tk.X, pady=(2, 4))

        # 2 Nokta İşaretleme Butonları Satırı
        lbl_marks_row = tk.Frame(lbl_split_box, bg=PALETTE["bg_panel"])
        lbl_marks_row.pack(fill=tk.X, pady=(0, 4))

        ModernButton(lbl_marks_row, text="🟢 Başlangıç [I]", command=self._label_mark_start_point,
                     bg_color="#e6fcf5", fg_color=PALETTE["accent_green"], hover_bg="#c3fae8",
                     font=("Segoe UI", 8, "bold"), padx=6, pady=2).pack(side=tk.LEFT, padx=2)

        ModernButton(lbl_marks_row, text="🔴 Bitiş [O]", command=self._label_mark_end_point,
                     bg_color="#fff0f6", fg_color=PALETTE["accent_red"], hover_bg="#ffdeeb",
                     font=("Segoe UI", 8, "bold"), padx=6, pady=2).pack(side=tk.LEFT, padx=2)

        ModernButton(lbl_marks_row, text="✖ Temizle [Esc]", command=self._label_clear_marks,
                     bg_color=PALETTE["btn_bg"], fg_color=PALETTE["fg_muted"],
                     font=("Segoe UI", 8), padx=6, pady=2).pack(side=tk.LEFT, padx=4)

        self.lbl_label_marks_summary = tk.Label(lbl_marks_row, text="Aralık: Tamamı",
                                                font=("Consolas", 9), bg=PALETTE["bg_panel"], fg=PALETTE["fg_muted"])
        self.lbl_label_marks_summary.pack(side=tk.LEFT, padx=4)

        # Model Input Boyutu & Stride & Bölme Butonu
        lbl_params_row = tk.Frame(lbl_split_box, bg=PALETTE["bg_panel"])
        lbl_params_row.pack(fill=tk.X, pady=(2, 2))

        tk.Label(lbl_params_row, text="Model Boyutu:", font=("Segoe UI", 9, "bold"),
                 bg=PALETTE["bg_panel"], fg=PALETTE["fg_text"]).pack(side=tk.LEFT, padx=(2, 2))

        self.spin_label_model_size = ttk.Spinbox(lbl_params_row, from_=1, to=1000, width=5,
                                                 textvariable=self.label_model_input_size,
                                                 command=self._on_label_param_changed)
        self.spin_label_model_size.pack(side=tk.LEFT, padx=(0, 8))
        self.spin_label_model_size.bind("<KeyRelease>", lambda e: self._on_label_param_changed())

        tk.Label(lbl_params_row, text="Stride:", font=("Segoe UI", 9, "bold"),
                 bg=PALETTE["bg_panel"], fg=PALETTE["fg_text"]).pack(side=tk.LEFT, padx=(2, 2))

        self.spin_label_stride = ttk.Spinbox(lbl_params_row, from_=1, to=100, width=4,
                                             textvariable=self.label_stride_size,
                                             command=self._on_label_param_changed)
        self.spin_label_stride.pack(side=tk.LEFT, padx=(0, 10))
        self.spin_label_stride.bind("<KeyRelease>", lambda e: self._on_label_param_changed())

        ModernButton(lbl_params_row, text="✂️  Veriyi Böl & Çoğalt", command=self._split_augment_current_label,
                     bg_color=PALETTE["accent_yellow"], fg_color="#ffffff",
                     hover_bg="#c76700", font=("Segoe UI", 9, "bold"), padx=10, pady=3).pack(side=tk.RIGHT, padx=2)

        # Önizleme Bildirim Satırı
        self.lbl_label_split_preview = tk.Label(lbl_split_box, text="💡 Önizleme: -",
                                                font=("Segoe UI", 8), bg=PALETTE["bg_panel"],
                                                fg=PALETTE["accent_blue"], justify=tk.LEFT, anchor=tk.W)
        self.lbl_label_split_preview.pack(fill=tk.X, pady=(2, 0))

        # 5. Metadata Detayları Satırı
        self.lbl_label_metadata_desc = tk.Label(lbl_control_card, text="",
                                                font=("Segoe UI", 8), bg=PALETTE["bg_card"], fg=PALETTE["fg_muted"], justify=tk.LEFT)
        self.lbl_label_metadata_desc.pack(anchor=tk.W, padx=5, pady=(2, 0))

    # -------------------------------------------------------------------------
    # GÖRÜNÜM VE PENCERE GEÇİŞLERİ
    # -------------------------------------------------------------------------
    def _show_selector(self):
        self._set_playing(False)
        self._set_label_playing(False)
        self.viewer_frame.pack_forget()
        self.label_viewer_frame.pack_forget()
        self.selector_frame.pack(fill=tk.BOTH, expand=True)
        # Eğer henüz taranmış bir veri seti yoksa ve FALLOWER DATASETS mevcutsa otomatik yükle
        if not self.dataset_recordings and os.path.isdir("FALLOWER DATASETS"):
            self._scan_and_display_dataset("FALLOWER DATASETS")

    def _show_viewer(self):
        self._set_label_playing(False)
        self.selector_frame.pack_forget()
        self.label_viewer_frame.pack_forget()
        self.viewer_frame.pack(fill=tk.BOTH, expand=True)
        self.root.update_idletasks()
        self._update_display(self.current_frame_idx)

    def _show_label_viewer(self):
        self._set_playing(False)
        self.selector_frame.pack_forget()
        self.viewer_frame.pack_forget()
        self.label_viewer_frame.pack(fill=tk.BOTH, expand=True)

    def _back_to_selector(self):
        self._set_playing(False)
        if self.cap is not None:
            self.cap.release()
            self.cap = None
        self._show_selector()

    # -------------------------------------------------------------------------
    # KLAVYE KISAYOLLARI
    # -------------------------------------------------------------------------
    def _bind_keyboard_shortcuts(self):
        self.root.bind("<space>", lambda e: self._on_spacebar())
        self.root.bind("<Left>", lambda e: self._on_arrow_left(e))
        self.root.bind("<Right>", lambda e: self._on_arrow_right(e))
        self.root.bind("<Home>", lambda e: self._seek_to_frame(0) if self.viewer_frame.winfo_viewable() else (self._seek_label_to_frame(0) if self.label_viewer_frame.winfo_viewable() else None))
        self.root.bind("<End>", lambda e: self._seek_to_frame(self.total_frames - 1) if self.viewer_frame.winfo_viewable() else (self._seek_label_to_frame(len(self.current_label_adc)-1 if self.current_label_adc is not None else 0) if self.label_viewer_frame.winfo_viewable() else None))
        self.root.bind("<i>", lambda e: self._mark_start_point() if self.viewer_frame.winfo_viewable() else (self._label_mark_start_point() if self.label_viewer_frame.winfo_viewable() else None))
        self.root.bind("<I>", lambda e: self._mark_start_point() if self.viewer_frame.winfo_viewable() else (self._label_mark_start_point() if self.label_viewer_frame.winfo_viewable() else None))
        self.root.bind("<o>", lambda e: self._mark_end_point() if self.viewer_frame.winfo_viewable() else (self._label_mark_end_point() if self.label_viewer_frame.winfo_viewable() else None))
        self.root.bind("<O>", lambda e: self._mark_end_point() if self.viewer_frame.winfo_viewable() else (self._label_mark_end_point() if self.label_viewer_frame.winfo_viewable() else None))
        self.root.bind("<Escape>", lambda e: self._clear_marks() if self.viewer_frame.winfo_viewable() else (self._label_clear_marks() if self.label_viewer_frame.winfo_viewable() else None))
        self.root.bind("<c>", lambda e: self._clear_marks() if self.viewer_frame.winfo_viewable() else (self._label_clear_marks() if self.label_viewer_frame.winfo_viewable() else None))
        self.root.bind("<C>", lambda e: self._clear_marks() if self.viewer_frame.winfo_viewable() else (self._label_clear_marks() if self.label_viewer_frame.winfo_viewable() else None))

    def _on_spacebar(self):
        focused = self.root.focus_get()
        if isinstance(focused, tk.Entry):
            return
        if self.viewer_frame.winfo_viewable():
            self._toggle_play()
        elif self.label_viewer_frame.winfo_viewable():
            self._toggle_label_play()

    def _on_arrow_left(self, event):
        focused = self.root.focus_get()
        if isinstance(focused, tk.Entry):
            return
        step = -10 if (event.state & 0x0001) else -1
        if self.viewer_frame.winfo_viewable():
            self._seek_relative(step)
        elif self.label_viewer_frame.winfo_viewable():
            self._seek_label_relative(step)

    def _on_arrow_right(self, event):
        focused = self.root.focus_get()
        if isinstance(focused, tk.Entry):
            return
        step = 10 if (event.state & 0x0001) else 1
        if self.viewer_frame.winfo_viewable():
            self._seek_relative(step)
        elif self.label_viewer_frame.winfo_viewable():
            self._seek_label_relative(step)

    # -------------------------------------------------------------------------
    # VERİ SETİ & OTURUM TARAMA, SEÇME VE YÜKLEME
    # -------------------------------------------------------------------------
    def _browse_folder(self):
        initial = "FALLOWER DATASETS" if os.path.isdir("FALLOWER DATASETS") else os.getcwd()
        folder = filedialog.askdirectory(title="Veri seti veya kayıt klasörünü seçin", initialdir=initial)
        if not folder:
            return
        self._scan_and_display_dataset(folder)

    def _scan_and_display_dataset(self, folder):
        self.dataset_dir = folder
        self.lbl_selected_path.config(text=f"📂 {os.path.abspath(folder)}")

        # Eğer seçilen klasörün içinde doğrudan adc.npy varsa ve tekil bir kayıtsa:
        if os.path.exists(os.path.join(folder, "adc.npy")) and not os.path.isdir(os.path.join(folder, "labels")):
            summary = self._get_record_summary(folder)
            self.dataset_recordings = [summary]
        else:
            self.dataset_recordings = self._find_recordings_in_dir(folder)

        if not self.dataset_recordings:
            self.selector_status.config(text="⚠️ Seçilen klasörde geçerli kayıt oturumu (adc.npy) bulunamadı.")
            self.rec_listbox.delete(0, tk.END)
            self.lbl_rec_info_box.config(text="Kayıt bulunamadı.")
            return

        self.selector_status.config(text=f"✅ {len(self.dataset_recordings)} kayıt oturumu bulundu.")
        self._filter_dataset_recordings()

        # İlk kaydı otomatik seç
        if self.rec_listbox.size() > 0:
            self.rec_listbox.selection_set(0)
            self._on_record_select(None)

    def _find_recordings_in_dir(self, root_dir):
        """Sadece 1 alt klasör seviyesini inceler (derine inmez)."""
        recordings = []
        if not os.path.isdir(root_dir):
            return recordings

        # 1. Klasörün kendisi doğrudan bir kayıt mı?
        if os.path.exists(os.path.join(root_dir, "adc.npy")):
            recordings.append(self._get_record_summary(root_dir))
            return recordings

        # 2. Sadece doğrudan 1 alt seviyedeki klasörleri kontrol et (derine inme)
        try:
            entries = sorted(os.listdir(root_dir))
        except Exception:
            entries = []

        for item in entries:
            full_path = os.path.join(root_dir, item)
            if os.path.isdir(full_path):
                base = item.lower()
                if base == "labels":
                    continue
                if os.path.exists(os.path.join(full_path, "adc.npy")):
                    recordings.append(self._get_record_summary(full_path))

        recordings.sort(key=lambda x: x.get("name", "").lower())
        return recordings

    def _get_record_summary(self, folder):
        name = os.path.basename(folder)
        meta_path = os.path.join(folder, "metadata.json")
        desc = ""
        date_str = ""
        duration_s = 0.0
        frame_cnt = 0

        if os.path.exists(meta_path):
            try:
                with open(meta_path, "r", encoding="utf-8") as f:
                    meta = json.load(f)
                desc = meta.get("description", "") or meta.get("desc", "") or ""
                date_str = meta.get("record_date", "") or meta.get("date", "") or ""
                duration_s = meta.get("duration_s", 0.0) or meta.get("duration", 0.0) or 0.0
            except Exception:
                pass

        adc_path = os.path.join(folder, "adc.npy")
        if os.path.exists(adc_path):
            try:
                adc_data = np.load(adc_path, mmap_mode="r")
                frame_cnt = len(adc_data)
                if duration_s == 0.0 and frame_cnt > 0:
                    duration_s = round(frame_cnt * 0.05, 1)
            except Exception:
                pass

        video_files = [f for f in os.listdir(folder) if f.lower().startswith("video_") and f.lower().endswith((".mp4", ".avi", ".mkv"))]
        has_video = len(video_files) > 0

        return {
            "path": folder,
            "name": name,
            "desc": desc,
            "date": date_str,
            "duration_s": duration_s,
            "frame_cnt": frame_cnt,
            "has_video": has_video,
        }

    def _filter_dataset_recordings(self):
        query = self.entry_rec_search.get().strip().lower()
        self.rec_listbox.delete(0, tk.END)
        self.filtered_recordings = []

        for rec in self.dataset_recordings:
            name = rec["name"].lower()
            desc = rec["desc"].lower()
            date = rec["date"].lower()
            if not query or query in name or query in desc or query in date:
                self.filtered_recordings.append(rec)
                dur_txt = f"{rec['duration_s']:.1f}s" if rec['duration_s'] > 0 else "-"
                cnt_txt = f"{rec['frame_cnt']}f" if rec['frame_cnt'] > 0 else "-"
                vid_ico = "🎥" if rec["has_video"] else "⚪"
                display_str = f" {vid_ico}  {rec['name']:<24} │ {cnt_txt:>5} ({dur_txt:>6}) │ {rec['desc'][:26]}"
                self.rec_listbox.insert(tk.END, display_str)

    def _on_record_select(self, event=None):
        sel = self.rec_listbox.curselection()
        if not sel or sel[0] >= len(self.filtered_recordings):
            return
        rec = self.filtered_recordings[sel[0]]
        self.selected_recording_info = rec
        vid_txt = "Var ✅" if rec["has_video"] else "Yok ❌"
        desc_txt = rec["desc"] if rec["desc"] else "(Açıklama yok)"
        date_txt = rec["date"] if rec["date"] else "-"
        info_str = f"📁 {rec['name']}  │  Frame: {rec['frame_cnt']}  │  Süre: {rec['duration_s']:.1f}s  │  Video: {vid_txt}\n📝 Açıklama: {desc_txt}  │  Tarih: {date_txt}"
        self.lbl_rec_info_box.config(text=info_str)

    def _open_selected_recording(self):
        if not self.selected_recording_info:
            sel = self.rec_listbox.curselection()
            if sel and sel[0] < len(self.filtered_recordings):
                self.selected_recording_info = self.filtered_recordings[sel[0]]
            else:
                messagebox.showwarning("Uyarı", "Lütfen açmak için listeden bir kayıt seçin!")
                return

        rec_path = self.selected_recording_info["path"]
        ok, msg = self._load_record(rec_path)
        if ok:
            self.selector_status.config(text="")
            self._show_viewer()
        else:
            self.selector_status.config(text=f"⚠️ {msg}")
            messagebox.showerror("Yükleme Hatası", msg)

    def _load_record(self, folder):
        required = ["adc.npy", "adc_meta.npy", "camera_timestamps.npy", "metadata.json"]
        missing = [f for f in required if not os.path.exists(os.path.join(folder, f))]
        if missing:
            return False, f"Eksik dosya(lar): {', '.join(missing)}"

        video_files = [f for f in os.listdir(folder) if f.lower().startswith("video_") and f.lower().endswith((".mp4", ".avi", ".mkv"))]
        if not video_files:
            return False, "Klasörde video_*.mp4 / .avi dosyası bulunamadı."
        video_path = os.path.join(folder, video_files[0])

        try:
            with open(os.path.join(folder, "metadata.json"), "r", encoding="utf-8") as f:
                metadata = json.load(f)
            adc_data = np.load(os.path.join(folder, "adc.npy"), allow_pickle=True)
            adc_meta = np.load(os.path.join(folder, "adc_meta.npy"))
            camera_ts = np.load(os.path.join(folder, "camera_timestamps.npy"))
        except Exception as e:
            return False, f"Dosyalar okunurken hata oluştu:\n{e}"

        cap = cv2.VideoCapture(video_path)
        if not cap.isOpened():
            return False, f"Video dosyası açılamadı: {video_files[0]}"

        if self.cap is not None:
            self.cap.release()

        self.record_dir = folder
        self.metadata = metadata
        self.adc_data = adc_data
        self.adc_meta = adc_meta
        self.camera_ts = camera_ts
        self.video_path = video_path
        self.cap = cap
        self.video_frame_count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))

        # Toplam frame sayısı
        self.total_frames = min(len(adc_data), self.video_frame_count)
        if self.total_frames == 0:
            self.total_frames = max(len(adc_data), self.video_frame_count)

        # --- GERÇEK DONANIM ZAMAN DAMGALARINI OLUŞTURMA (TIMESTAMPS) ---
        # adc_meta[:, 1] gerçek PC zaman damgalarını saniye cinsinden içerir.
        if len(adc_meta) >= self.total_frames and len(adc_meta) > 0:
            raw_ts = adc_meta[:self.total_frames, 1].astype(np.float64)
            t0 = raw_ts[0]
            self.frame_timestamps = raw_ts - t0
        elif len(camera_ts) >= self.total_frames and len(camera_ts) > 0:
            raw_ts = camera_ts[:self.total_frames].astype(np.float64)
            t0 = raw_ts[0]
            self.frame_timestamps = raw_ts - t0
        else:
            # Yedek: varsayılan periyot ile yapay zaman dizisi
            p_s = DEFAULT_RADAR_PERIOD_MS / 1000.0
            self.frame_timestamps = np.arange(self.total_frames, dtype=np.float64) * p_s

        # Toplam Süre & Ortalama Periyot
        if len(self.frame_timestamps) > 1:
            self.duration = float(self.frame_timestamps[-1])
            diffs = np.diff(self.frame_timestamps)
            pos_diffs = diffs[diffs > 0]
            mean_dt_s = float(np.median(pos_diffs)) if len(pos_diffs) else (DEFAULT_RADAR_PERIOD_MS / 1000.0)
            self.radar_period_s = mean_dt_s
            self.radar_period_ms = mean_dt_s * 1000.0
        else:
            self.duration = 0.0
            self.radar_period_ms = DEFAULT_RADAR_PERIOD_MS
            self.radar_period_s = DEFAULT_RADAR_PERIOD_MS / 1000.0

        v_fps = metadata.get("video_fps") or cap.get(cv2.CAP_PROP_FPS)
        if v_fps and float(v_fps) > 0:
            self.video_fps = float(v_fps)
        elif self.radar_period_s > 0:
            self.video_fps = 1.0 / self.radar_period_s
        else:
            self.video_fps = DEFAULT_VIDEO_FPS

        r_fps = metadata.get("radar_fps")
        if r_fps and float(r_fps) > 0:
            self.radar_fps = float(r_fps)
        elif self.radar_period_s > 0:
            self.radar_fps = 1.0 / self.radar_period_s
        else:
            self.radar_fps = DEFAULT_RADAR_FPS

        # Slider ve Durum Ayarı
        self.frame_slider.config(from_=0, to=max(self.total_frames - 1, 0))
        self.current_frame_idx = 0
        self._last_cap_frame_idx = -1
        self.frame_var.set(0)
        self.mark_start = None
        self.mark_end = None
        self.entry_label_name.delete(0, tk.END)
        self.lbl_label_status.config(text="")

        # Başlık ve Rozet Bilgileri
        rec_name = os.path.basename(folder)
        self.lbl_record_title.config(text=f"📁 {rec_name}")

        frame_mismatch = abs(len(adc_data) - self.video_frame_count)
        if frame_mismatch > 0:
            self.badge_sync_status.config(
                text=f"⚠️ Uyumsuzluk: Δ{frame_mismatch} frame (ADC:{len(adc_data)}, Vid:{self.video_frame_count})",
                bg="#fff9db", fg=PALETTE["accent_yellow"]
            )
        else:
            self.badge_sync_status.config(
                text=f"✓ Tam Senkron ({self.total_frames} frame)",
                bg="#e6fcf5", fg=PALETTE["accent_green"]
            )

        radar_cfg = metadata.get("radar_config", {})
        range_m = radar_cfg.get("range_meters")
        reg34_hex = radar_cfg.get("reg34_hex")
        range_str = f"  │  Menzil: {range_m:.1f}m ({reg34_hex})" if range_m is not None else ""
        details_txt = f"Ort. Periyot: {self.radar_period_ms:.1f}ms  │  FPS: {self.video_fps:.1f}  │  Süre: {self.duration:.2f}s{range_str}"
        self.lbl_record_details.config(text=details_txt)
        self.lbl_video_fps_badge.config(text=f"{self.video_fps:.1f} FPS")

        self.root.title(f"⚡ FALLOWER Stüdyo — {rec_name} [{self.radar_period_ms:.1f}ms]")
        self._update_marks_label()
        self._update_display(0)
        return True, "OK"

    # -------------------------------------------------------------------------
    # OYNATMA VE NAVİGASYON MOTORU (GERÇEK ZAMAN DAMGALI)
    # -------------------------------------------------------------------------
    def _on_slider_change(self, value):
        if self._slider_updating:
            return
        idx = int(float(value))
        self.current_frame_idx = idx
        self._update_display(idx)

    def _on_timeline_click(self, event):
        w = self.timeline_canvas.winfo_width()
        if w > 0 and self.total_frames > 0:
            target_idx = int((event.x / w) * self.total_frames)
            self._seek_to_frame(target_idx)

    def _seek_to_frame(self, idx):
        idx = max(0, min(self.total_frames - 1, idx))
        self._slider_updating = True
        self.frame_var.set(idx)
        self._slider_updating = False
        self.current_frame_idx = idx
        self._update_display(idx)

    def _seek_relative(self, delta):
        self._seek_to_frame(self.current_frame_idx + delta)

    def _toggle_play(self):
        self._set_playing(not self.playing)

    def _set_playing(self, value):
        self.playing = value
        if self.playing:
            if self.current_frame_idx >= self.total_frames - 1:
                self.current_frame_idx = 0
            self.btn_play.set_color(PALETTE["accent_red"], "#ffffff", "#c2255c")
            self.btn_play.configure(text="⏸  DURDUR")
            self._last_tick = time.time()
            self._play_tick()
        else:
            self.btn_play.set_color(PALETTE["accent_green"], "#ffffff", "#237032")
            self.btn_play.configure(text="▶  OYNAT")

    def _set_speed(self, speed):
        self.playback_speed = float(speed)
        for spd, btn in self.speed_buttons.items():
            if spd == speed:
                btn.set_color(PALETTE["accent_blue"], "#ffffff", "#1864ab")
            else:
                btn.set_color(PALETTE["btn_bg"], PALETTE["fg_text"], PALETTE["btn_hover"])

    def _on_loop_toggle(self):
        self.loop_playback = self.loop_var.get()

    def _play_tick(self):
        if not self.playing:
            return

        now = time.time()
        self._last_tick = now

        # Bir sonraki frame ile şu anki frame arasındaki gerçek zaman damgası farkı
        idx = self.current_frame_idx
        if self.frame_timestamps is not None and idx + 1 < len(self.frame_timestamps):
            dt_real = self.frame_timestamps[idx + 1] - self.frame_timestamps[idx]
            if dt_real <= 0 or np.isnan(dt_real):
                dt_real = self.radar_period_s
        else:
            dt_real = self.radar_period_s

        new_idx = idx + 1

        # Döngü veya son kontrolü
        if self.loop_playback and self.mark_start is not None and self.mark_end is not None:
            i0, i1 = sorted([self.mark_start, self.mark_end])
            if new_idx > i1:
                new_idx = i0
        elif new_idx >= self.total_frames:
            if self.loop_playback:
                new_idx = 0
            else:
                new_idx = self.total_frames - 1
                self._seek_to_frame(new_idx)
                self._set_playing(False)
                return

        self._seek_to_frame(new_idx)

        # Bir sonraki gecikmeyi gerçek zaman damgasına ve oynatma hızına göre belirle
        delay_ms = max(5, int((dt_real / max(self.playback_speed, 0.01)) * 1000.0))
        self.root.after(delay_ms, self._play_tick)

    # -------------------------------------------------------------------------
    # EKRAN GÜNCELLEME (SYNCHRONIZED DISPLAY)
    # -------------------------------------------------------------------------
    def _update_display(self, frame_idx):
        if self.frame_timestamps is not None and frame_idx < len(self.frame_timestamps):
            t = float(self.frame_timestamps[frame_idx])
        else:
            t = frame_idx * self.radar_period_s

        self.lbl_time_display.config(
            text=f"Frame #{frame_idx} / #{max(self.total_frames-1, 0)}  │  {t:.3f}s / {self.duration:.3f}s"
        )

        # 1. Video Güncelleme (Hızlı Sıralı Okuma veya Hassas Seek)
        if self.cap is not None and self.video_frame_count > 0:
            video_idx = min(frame_idx, self.video_frame_count - 1)
            if video_idx == self._last_cap_frame_idx + 1:
                ret, frame = self.cap.read()
            else:
                self.cap.set(cv2.CAP_PROP_POS_FRAMES, video_idx)
                ret, frame = self.cap.read()

            if ret:
                self._last_cap_frame_idx = video_idx
                self._show_video_frame(frame)
            else:
                self._last_cap_frame_idx = -1

        # 2. Radar ADC & Spektrum Çizimi
        if self.adc_data is not None and len(self.adc_data) > 0:
            adc_idx = min(frame_idx, len(self.adc_data) - 1)
            data = self.adc_data[adc_idx]
            stm32_rel = float(self.adc_meta[adc_idx, 0]) if self.adc_meta is not None and adc_idx < len(self.adc_meta) else 0.0
            pc_rel = float(self.adc_meta[adc_idx, 1]) if self.adc_meta is not None and adc_idx < len(self.adc_meta) else 0.0
            self._draw_radar_data(data, stm32_rel, pc_rel, adc_idx)
        else:
            self._draw_radar_data(None, 0.0, 0.0, None)

        # 3. Zaman Çizelgesi Vurgusu
        self._draw_timeline()

    def _on_bottom_container_resize(self, event=None):
        if event:
            bh = event.height
        else:
            bh = self.bottom_container.winfo_height()

        if bh < 60:
            return

        # Kullanılabilir video yüksekliği (header ve kenar boşlukları çıkarıldıktan sonra)
        vh = max(bh - 46, 60)
        vw = int(vh * 16 / 9)

        # Video panelinin genişliğini tam 16:9 oranına kitle (kenarlarda sıfır boşluk)
        self.video_panel.config(width=vw + 24)
        self._cam_target_w = vw
        self._cam_target_h = vh

    def _show_video_frame(self, frame):
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
    # RADAR ADC, MTI & IIR ARKA PLAN GÖRSELLEŞTİRME MOTORU
    # -------------------------------------------------------------------------
    def _set_view_mode(self, mode):
        self.view_mode = mode
        if hasattr(self, "btn_mode_raw"):
            self.btn_mode_raw.set_color("#e7f5ff" if mode == "waveform" else PALETTE["btn_bg"],
                                       PALETTE["accent_blue"] if mode == "waveform" else PALETTE["fg_text"])
        if hasattr(self, "btn_mode_mti"):
            self.btn_mode_mti.set_color("#f3f0ff" if mode == "mti" else PALETTE["btn_bg"],
                                       PALETTE["accent_purple"] if mode == "mti" else PALETTE["fg_text"])
        if hasattr(self, "btn_mode_iir"):
            self.btn_mode_iir.set_color("#e6fcf5" if mode == "iir_bg" else PALETTE["btn_bg"],
                                       PALETTE["accent_cyan"] if mode == "iir_bg" else PALETTE["fg_text"])
        self._update_display(self.current_frame_idx)

    def _set_label_view_mode(self, mode):
        self.label_view_mode = mode
        if hasattr(self, "btn_lbl_mode_raw"):
            self.btn_lbl_mode_raw.set_color("#e7f5ff" if mode == "waveform" else PALETTE["btn_bg"],
                                           PALETTE["accent_blue"] if mode == "waveform" else PALETTE["fg_text"])
        if hasattr(self, "btn_lbl_mode_mti"):
            self.btn_lbl_mode_mti.set_color("#f3f0ff" if mode == "mti" else PALETTE["btn_bg"],
                                           PALETTE["accent_purple"] if mode == "mti" else PALETTE["fg_text"])
        if hasattr(self, "btn_lbl_mode_iir"):
            self.btn_lbl_mode_iir.set_color("#e6fcf5" if mode == "iir_bg" else PALETTE["btn_bg"],
                                           PALETTE["accent_cyan"] if mode == "iir_bg" else PALETTE["fg_text"])
        self._redraw_current_label_adc()

    def _toggle_yscale_mode(self):
        modes = ["auto", "fixed_25k", "fixed_10k"]
        curr_idx = modes.index(self.y_scale_mode)
        self.y_scale_mode = modes[(curr_idx + 1) % len(modes)]
        labels = {"auto": "📐 Y-Ölçek: Otomatik", "fixed_25k": "📐 Y-Ölçek: ±25k", "fixed_10k": "📐 Y-Ölçek: ±10k"}
        self.btn_yscale.configure(text=labels[self.y_scale_mode])
        self._update_display(self.current_frame_idx)

    def _on_canvas_resize(self):
        w = self.adc_canvas.winfo_width()
        h = self.adc_canvas.winfo_height()
        # Sadece gerçek boyut değiştiğinde yeniden çizim yap (titremeyi engeller)
        if abs(w - self._last_canvas_w) > 2 or abs(h - self._last_canvas_h) > 2:
            self._last_canvas_w = w
            self._last_canvas_h = h
            self._update_display(self.current_frame_idx)

    def _compute_mti_frame(self, adc_array, frame_idx):
        """
        3-Pulse MTI Filtresi (Double Canceller):
        y[n] = x[n] - 2*x[n-1] + x[n-2]
        """
        if adc_array is None or len(adc_array) == 0 or frame_idx is None:
            return None
        
        idx = max(0, min(frame_idx, len(adc_array) - 1))
        x_n = np.asarray(adc_array[idx], dtype=np.float64)
        
        if idx >= 2:
            x_n1 = np.asarray(adc_array[idx - 1], dtype=np.float64)
            x_n2 = np.asarray(adc_array[idx - 2], dtype=np.float64)
            min_len = min(len(x_n), len(x_n1), len(x_n2))
            return x_n[:min_len] - 2.0 * x_n1[:min_len] + x_n2[:min_len]
        elif idx == 1:
            x_n1 = np.asarray(adc_array[0], dtype=np.float64)
            min_len = min(len(x_n), len(x_n1))
            return x_n[:min_len] - x_n1[:min_len]
        else:
            return np.zeros_like(x_n)

    def _compute_iir_bg_frame(self, adc_array, frame_idx, alpha=0.10):
        """
        IIR / Exponential Background Removal:
        b_k[n] = (1 - alpha) * b_{k-1}[n] + alpha * x_k[n]
        y_k[n] = x_k[n] - b_k[n]
        """
        if adc_array is None or len(adc_array) == 0 or frame_idx is None:
            return None

        idx = max(0, min(frame_idx, len(adc_array) - 1))
        
        # Son ~60 frame üzerinden b_k hesapla (anlık arama & titreşimsiz seek)
        w_start = max(0, idx - 60)
        b_k = np.asarray(adc_array[w_start], dtype=np.float64).copy()
        
        for k in range(w_start + 1, idx + 1):
            x_k = np.asarray(adc_array[k], dtype=np.float64)
            b_k = (1.0 - alpha) * b_k + alpha * x_k

        x_curr = np.asarray(adc_array[idx], dtype=np.float64)
        min_len = min(len(x_curr), len(b_k))
        return x_curr[:min_len] - b_k[:min_len]

    def _draw_radar_data(self, data, stm32_rel, pc_rel, idx):
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

        if data is None or len(data) <= 1:
            c.create_text(w // 2, h // 2, text="ADC Verisi Yok", fill=PALETTE["fg_dim"], font=("Segoe UI", 12), tags="plot_elem")
            self.adc_stats.config(text="Min: -  |  Max: -  |  P-P: -  |  Ort: -  |  RMS: -  |  Örnek: -")
            return

        data_arr = np.asarray(data, dtype=np.float64)

        if self.view_mode == "waveform":
            plot_vals = data_arr
            line_color = PALETTE["accent_blue"]
            mode_title = "Ham ADC"
        elif self.view_mode == "mti":
            mti_vals = self._compute_mti_frame(self.adc_data, idx)
            if mti_vals is None:
                mti_vals = np.zeros_like(data_arr)
            plot_vals = mti_vals
            line_color = PALETTE["accent_purple"]
            mode_title = "MTI (y[n]=x[n]-2x[n-1]+x[n-2])"
        else: # iir_bg
            iir_vals = self._compute_iir_bg_frame(self.adc_data, idx)
            if iir_vals is None:
                iir_vals = np.zeros_like(data_arr)
            plot_vals = iir_vals
            line_color = PALETTE["accent_cyan"]
            mode_title = "IIR Arka Plan (y[n]=x[n]-b[n])"

        n_samples = len(plot_vals)
        min_v = float(np.min(plot_vals))
        max_v = float(np.max(plot_vals))
        mean_v = float(np.mean(plot_vals))
        rms_v = float(np.sqrt(np.mean(plot_vals**2)))
        pp_v = max_v - min_v

        # Y-Ölçeklendirme (Ham ADC, MTI ve IIR Arka Plan için ortak ölçek yönetimi)
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
            text=f"{mode_title}  │  Min: {min_v:+.0f}  │  Max: {max_v:+.0f}  │  P-P: {pp_v:.0f}  │  Ort: {mean_v:+.1f}  │  RMS: {rms_v:.0f}  │  Örnek: {n_samples}  │  #{idx}"
        )

    # Titremesiz Canvas İçi Tooltip & Crosshair
    def _on_adc_canvas_hover(self, event):
        if self.adc_data is None or len(self.adc_data) == 0:
            return
        adc_idx = min(self.current_frame_idx, len(self.adc_data) - 1)
        data = self.adc_data[adc_idx]
        if data is None or len(data) == 0:
            return

        c = self.adc_canvas
        w = c.winfo_width()
        h = c.winfo_height()
        if w <= 30 or h <= 30:
            return

        # Sadece hover_overlay etiketini sil (tüm canvas'ı yeniden çizmeden)
        c.delete("hover_overlay")

        data_arr = np.asarray(data, dtype=np.float64)

        if self.view_mode == "waveform":
            plot_vals = data_arr
            label_prefix = "ADC Genlik"
        elif self.view_mode == "mti":
            plot_vals = self._compute_mti_frame(self.adc_data, adc_idx)
            if plot_vals is None:
                plot_vals = np.zeros_like(data_arr)
            label_prefix = "MTI Genlik"
        else: # iir_bg
            plot_vals = self._compute_iir_bg_frame(self.adc_data, adc_idx)
            if plot_vals is None:
                plot_vals = np.zeros_like(data_arr)
            label_prefix = "IIR Arka Plan"

        n_samples = len(plot_vals)
        if n_samples <= 1:
            return

        bin_idx = int(((event.x - 10) / (w - 20)) * n_samples)
        bin_idx = max(0, min(n_samples - 1, bin_idx))
        val = plot_vals[bin_idx]

        # Nokta X konumu
        pt_x = int((bin_idx / (n_samples - 1)) * (w - 20) + 10)

        # Y konumu (Ham ADC, MTI ve IIR Arka Plan için ortak ölçek)
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

    # -------------------------------------------------------------------------
    # İNTERAKTİF ZAMAN ÇİZELGESİ (TIMELINE & REGION MARKING)
    # -------------------------------------------------------------------------
    def _draw_timeline(self):
        c = self.timeline_canvas
        c.delete("all")
        w = c.winfo_width()
        h = c.winfo_height()
        if w < 10 or h < 5 or self.total_frames <= 0:
            return

        def x_of(idx):
            return int((idx / max(self.total_frames - 1, 1)) * w)

        # Arka plan rayı
        c.create_rectangle(0, h // 2 - 2, w, h // 2 + 2, fill="#ced4da", outline="")

        # Seçili Etiket Bölgesi (Amber Vurgusu)
        if self.mark_start is not None and self.mark_end is not None:
            i0, i1 = sorted([self.mark_start, self.mark_end])
            c.create_rectangle(x_of(i0), 0, x_of(i1), h, fill="#ffe066", outline=PALETTE["accent_yellow"], width=1)

        # Başlangıç İşareti (Yeşil Bayrak)
        if self.mark_start is not None:
            x_s = x_of(self.mark_start)
            c.create_polygon(x_s - 4, 0, x_s + 4, 0, x_s, 8, fill=PALETTE["accent_green"], outline="")
            c.create_line(x_s, 0, x_s, h, fill=PALETTE["accent_green"], width=2)

        # Bitiş İşareti (Kırmızı Bayrak)
        if self.mark_end is not None:
            x_e = x_of(self.mark_end)
            c.create_polygon(x_e - 4, 0, x_e + 4, 0, x_e, 8, fill=PALETTE["accent_red"], outline="")
            c.create_line(x_e, 0, x_e, h, fill=PALETTE["accent_red"], width=2)

        # Oynatma İmleci (Mavi Çizgi)
        x_cur = x_of(self.current_frame_idx)
        c.create_line(x_cur, 0, x_cur, h, fill=PALETTE["accent_blue"], width=2)

    def _mark_start_point(self):
        if self.adc_data is None:
            return
        self.mark_start = self.current_frame_idx
        self._update_marks_label()
        self._draw_timeline()

    def _mark_end_point(self):
        if self.adc_data is None:
            return
        self.mark_end = self.current_frame_idx
        self._update_marks_label()
        self._draw_timeline()

    def _clear_marks(self):
        self.mark_start = None
        self.mark_end = None
        self.lbl_label_status.config(text="")
        self._update_marks_label()
        self._draw_timeline()

    def _update_marks_label(self):
        start_txt = f"#{self.mark_start}" if self.mark_start is not None else "-"
        end_txt = f"#{self.mark_end}" if self.mark_end is not None else "-"
        if self.mark_start is not None and self.mark_end is not None:
            i0, i1 = sorted([self.mark_start, self.mark_end])
            cnt = i1 - i0 + 1
            if self.frame_timestamps is not None and i1 < len(self.frame_timestamps):
                dur_s = abs(self.frame_timestamps[i1] - self.frame_timestamps[i0])
            else:
                dur_s = cnt * self.radar_period_s
            dur_txt = f"{dur_s:.3f}s ({cnt} frame)"
            frame_txt = f"{cnt}"
            if cnt < 30:
                self.lbl_marks_summary.config(
                    text=f"Aralık: {start_txt} → {end_txt}  │  Süre: {dur_txt}  │  ADC Frame: {frame_txt}  ⚠️ (En az 30 frame gerekli)",
                    fg=PALETTE["accent_red"]
                )
            else:
                self.lbl_marks_summary.config(
                    text=f"Aralık: {start_txt} → {end_txt}  │  Süre: {dur_txt}  │  ADC Frame: {frame_txt}  ✅",
                    fg=PALETTE["fg_muted"]
                )
        else:
            self.lbl_marks_summary.config(
                text=f"Aralık: {start_txt} → {end_txt}  │  Süre: -  │  ADC Frame: -",
                fg=PALETTE["fg_muted"]
            )

    def _set_quick_label(self, name):
        self.entry_label_name.delete(0, tk.END)
        self.entry_label_name.insert(0, name)
        self.entry_label_name.focus_set()

    @staticmethod
    def _sanitize_label_name(text):
        tr_map = str.maketrans("çğıöşüÇĞİÖŞÜ", "cgiosuCGIOSU")
        text = text.translate(tr_map)
        text = re.sub(r"[^a-zA-Z0-9_\-]", "_", text)
        text = re.sub(r"_+", "_", text)
        return text.strip("_").lower()

    # -------------------------------------------------------------------------
    # ETİKET KAYDETME & VİDEO/ADC KESME (DOSYAADI_ETİKETADI FORMATI)
    # -------------------------------------------------------------------------
    def _save_label(self):
        if self.record_dir is None or self.adc_data is None:
            messagebox.showwarning("Uyarı", "Önce bir kayıt klasörü yükleyin!")
            return
        if self.mark_start is None or self.mark_end is None:
            messagebox.showwarning("Uyarı", "Lütfen önce başlangıç ve bitiş frame'lerini işaretleyin!")
            return

        i0, i1 = sorted([self.mark_start, self.mark_end])
        idx_start = max(0, min(i0, len(self.adc_data) - 1))
        idx_end = max(idx_start, min(i1, len(self.adc_data) - 1)) + 1
        frame_count = idx_end - idx_start

        if frame_count < 30:
            messagebox.showwarning(
                "Yetersiz Frame Sayısı",
                f"Etiketlenecek kesit en az 30 frame olmalıdır!\n\n"
                f"• Seçilen Aralık: Frame #{idx_start} - #{idx_end - 1}\n"
                f"• Seçilen Frame Sayısı: {frame_count} frame\n"
                f"• Gerekli Minimum: 30 frame\n\n"
                f"Lütfen başlangıç ve bitiş noktalarını en az 30 frame aralık olacak şekilde seçin."
            )
            return

        raw_name = self.entry_label_name.get().strip()
        if not raw_name:
            messagebox.showwarning("Uyarı", "Lütfen bir etiket adı girin!")
            self.entry_label_name.focus_set()
            return

        safe_name = self._sanitize_label_name(raw_name)
        if not safe_name:
            messagebox.showerror("Hata", "Etiket adı geçersiz karakterlerden oluşuyor!")
            return

        # Zaman damgaları
        if self.frame_timestamps is not None and idx_end - 1 < len(self.frame_timestamps):
            t0 = float(self.frame_timestamps[idx_start])
            t1 = float(self.frame_timestamps[idx_end - 1])
            dur = round(abs(t1 - t0), 3)
        else:
            t0 = idx_start * self.radar_period_s
            t1 = (idx_end - 1) * self.radar_period_s
            dur = round((idx_end - idx_start) * self.radar_period_s, 3)

        adc_subset = self.adc_data[idx_start:idx_end]
        adc_meta_subset = self.adc_meta[idx_start:idx_end] if self.adc_meta is not None else None

        labels_dir = os.path.join(self.record_dir, "labels")
        os.makedirs(labels_dir, exist_ok=True)

        # İsimlendirme formatı: {dosyaadi}_{etiketadi}
        source_record_name = os.path.basename(self.record_dir)
        base_folder_name = f"{source_record_name}_{safe_name}"
        folder_name = base_folder_name
        counter = 2
        while os.path.exists(os.path.join(labels_dir, folder_name)):
            folder_name = f"{base_folder_name}_{counter}"
            counter += 1
        label_dir = os.path.join(labels_dir, folder_name)
        os.makedirs(label_dir, exist_ok=True)

        # ADC ve Meta Verileri Kaydet
        adc_array = np.array(adc_subset, dtype=object)
        np.save(os.path.join(label_dir, "adc.npy"), adc_array)
        if adc_meta_subset is not None:
            np.save(os.path.join(label_dir, "adc_meta.npy"), adc_meta_subset)

        # Video Kırpma İşlemi
        video_saved = False
        video_filename = None
        if self.cap is not None and self.cap.isOpened():
            orig_pos = self.current_frame_idx
            video_idx_start = min(idx_start, self.video_frame_count - 1)
            video_idx_end = min(idx_end, self.video_frame_count)
            frame_count_to_write = video_idx_end - video_idx_start

            if frame_count_to_write > 0:
                w = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
                h = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
                video_filename = f"video_{folder_name}.mp4"
                video_path = os.path.join(label_dir, video_filename)

                fourcc = cv2.VideoWriter_fourcc(*'mp4v')
                writer = cv2.VideoWriter(video_path, fourcc, self.video_fps, (w, h))
                if not writer.isOpened():
                    video_filename = f"video_{folder_name}.avi"
                    video_path = os.path.join(label_dir, video_filename)
                    fourcc = cv2.VideoWriter_fourcc(*'MJPG')
                    writer = cv2.VideoWriter(video_path, fourcc, self.video_fps, (w, h))

                if writer.isOpened():
                    self.cap.set(cv2.CAP_PROP_POS_FRAMES, video_idx_start)
                    written = 0
                    for _ in range(frame_count_to_write):
                        ret, frame = self.cap.read()
                        if not ret:
                            break
                        writer.write(frame)
                        written += 1
                    writer.release()
                    if written > 0:
                        video_saved = True

            # Orijinal video pozisyonunu geri yükle
            self.cap.set(cv2.CAP_PROP_POS_FRAMES, orig_pos)
            self._last_cap_frame_idx = orig_pos

        # label_info.json Oluşturma
        label_info = {
            "label": raw_name,
            "safe_name": safe_name,
            "folder_name": folder_name,
            "source_record": source_record_name,
            "start_frame": idx_start,
            "end_frame": idx_end - 1,
            "start_time_sec": round(t0, 4),
            "end_time_sec": round(t1, 4),
            "duration_sec": dur,
            "adc_frame_count": int(frame_count),
            "adc_index_range": [idx_start, idx_end],
            "video_frame_count": int(frame_count),
            "video_fps": round(self.video_fps, 4),
            "radar_fps": round(self.radar_fps, 2) if hasattr(self, "radar_fps") and self.radar_fps > 0 else round(1000.0/self.radar_period_ms, 2),
            "video_file": video_filename if video_saved else None,
            "created_at": datetime.now().isoformat(),
            "radar_period_ms": self.radar_period_ms,
            "note": f"Frame index bazlı etiket. {source_record_name}_{safe_name}"
        }
        with open(os.path.join(label_dir, "label_info.json"), "w", encoding="utf-8") as f:
            json.dump(label_info, f, ensure_ascii=False, indent=2)

        # labels_index.json Güncelleme
        index_path = os.path.join(labels_dir, "labels_index.json")
        index_data = []
        if os.path.exists(index_path):
            try:
                with open(index_path, "r", encoding="utf-8") as f:
                    index_data = json.load(f)
            except Exception:
                index_data = []

        index_data.append({
            "folder": folder_name,
            "label": raw_name,
            "start_frame": idx_start,
            "end_frame": idx_end - 1,
            "duration_sec": dur,
            "adc_frame_count": int(frame_count),
            "video_file": video_filename if video_saved else None,
            "created_at": label_info["created_at"]
        })
        with open(index_path, "w", encoding="utf-8") as f:
            json.dump(index_data, f, ensure_ascii=False, indent=2)

        status_msg = f"✅ Kaydedildi: labels/{folder_name} ({frame_count} frame)"
        self.lbl_label_status.config(text=status_msg)
        self.entry_label_name.delete(0, tk.END)
        print(f"🏷️  Etiket kaydedildi: {label_dir}")

    # -------------------------------------------------------------------------
    # ETİKET İNCELEME MODU (LABEL VIEWER & MANAGEMENT)
    # -------------------------------------------------------------------------
    def _browse_label_folder(self):
        initial = "FALLOWER DATASETS" if os.path.isdir("FALLOWER DATASETS") else os.getcwd()
        folder = filedialog.askdirectory(title="'labels' Klasörünü Seçin", initialdir=initial)
        if not folder:
            return

        self.label_base_dir = folder
        self._refresh_label_list()
        if not self.label_list:
            messagebox.showerror("Hata", "Seçilen klasörde etiketlenmiş kesit bulunamadı.")
            return

        self.lbl_label_folder_info.config(text=f"Etiket Klasörü: {os.path.basename(folder)}")
        self._show_label_viewer()

    def _refresh_label_list(self):
        if not self.label_base_dir or not os.path.isdir(self.label_base_dir):
            return
        self.label_list = []
        for item in sorted(os.listdir(self.label_base_dir)):
            subpath = os.path.join(self.label_base_dir, item)
            if os.path.isdir(subpath) and os.path.exists(os.path.join(subpath, "adc.npy")):
                display_name = item
                info_path = os.path.join(subpath, "label_info.json")
                label_info = {}
                if os.path.exists(info_path):
                    try:
                        with open(info_path, "r", encoding="utf-8") as f:
                            label_info = json.load(f)
                            display_name = f"{label_info.get('label', item)} ({label_info.get('duration_sec', 0)}s - {label_info.get('adc_frame_count', 0)}fr)"
                    except Exception:
                        pass
                self.label_list.append({
                    "name": display_name,
                    "folder": item,
                    "path": subpath,
                    "info": label_info
                })
        self._filter_label_list()

    def _filter_label_list(self):
        query = self.entry_filter.get().strip().lower()
        self.label_listbox.delete(0, tk.END)
        self.filtered_indices = []
        for i, lbl in enumerate(self.label_list):
            if not query or query in lbl["name"].lower():
                self.label_listbox.insert(tk.END, lbl["name"])
                self.filtered_indices.append(i)

    def _on_label_select(self, event):
        selection = self.label_listbox.curselection()
        if not selection:
            return
        real_idx = self.filtered_indices[selection[0]]
        self.current_label_idx = real_idx
        selected_item = self.label_list[real_idx]
        selected_path = selected_item["path"]

        try:
            self.current_label_adc = np.load(os.path.join(selected_path, "adc.npy"), allow_pickle=True)
            meta_path = os.path.join(selected_path, "adc_meta.npy")
            self.current_label_meta = np.load(meta_path) if os.path.exists(meta_path) else None
            self.current_label_info = selected_item["info"]

            # Periyot & Zaman Damgaları
            if self.current_label_meta is not None and len(self.current_label_meta) > 0:
                raw_ts = self.current_label_meta[:, 1].astype(np.float64)
                self.current_label_timestamps = raw_ts - raw_ts[0]
            else:
                p = self.current_label_info.get("radar_period_ms") or DEFAULT_RADAR_PERIOD_MS
                p_s = float(p) / 1000.0
                self.current_label_timestamps = np.arange(len(self.current_label_adc), dtype=np.float64) * p_s

            p_info = self.current_label_info.get("radar_period_ms") or DEFAULT_RADAR_PERIOD_MS
            self.current_label_radar_period_ms = float(p_info)

            # Video bulma
            video_files = [f for f in os.listdir(selected_path) if f.lower().startswith("video_") and f.lower().endswith((".mp4", ".avi", ".mkv"))]
            if video_files:
                self.current_label_video_path = os.path.join(selected_path, video_files[0])
                if self.current_label_cap is not None:
                    self.current_label_cap.release()
                self.current_label_cap = cv2.VideoCapture(self.current_label_video_path)
                self.current_label_frame_count = int(self.current_label_cap.get(cv2.CAP_PROP_FRAME_COUNT))
            else:
                self.current_label_video_path = None
                self.current_label_frame_count = 0
                if self.current_label_cap is not None:
                    self.current_label_cap.release()
                    self.current_label_cap = None

            max_frames = len(self.current_label_adc)
            self.label_slider.config(to=max(max_frames - 1, 0))
            self.label_frame_var.set(0)
            self.label_current_frame_idx = 0
            self._last_label_cap_frame_idx = -1
            self.label_mark_start = None
            self.label_mark_end = None

            # Detay Metni
            info_txt = (
                f"🏷️ Etiket: {self.current_label_info.get('label', '-')}  │  "
                f"Süre: {self.current_label_info.get('duration_sec', '-')}s  │  "
                f"Frame: {len(self.current_label_adc)}  │  "
                f"Kaynak: {self.current_label_info.get('source_record', '-')}  │  "
                f"Tarih: {self.current_label_info.get('created_at', '-')[:19]}"
            )
            self.lbl_label_metadata_desc.config(text=info_txt)
            self._update_label_marks_label()
            self._on_label_slider_change(0)
            self._draw_label_timeline()
        except Exception as e:
            messagebox.showerror("Hata", f"Etiket verisi yüklenirken hata oluştu:\n{e}")

    def _on_label_slider_change(self, value):
        if self.current_label_adc is None:
            return
        frame_idx = int(float(value))
        self.label_current_frame_idx = frame_idx
        if 0 <= frame_idx < len(self.current_label_adc):
            data = self.current_label_adc[frame_idx]
            stm32_rel = float(self.current_label_meta[frame_idx, 0]) if self.current_label_meta is not None and frame_idx < len(self.current_label_meta) else 0.0
            pc_rel = float(self.current_label_meta[frame_idx, 1]) if self.current_label_meta is not None and frame_idx < len(self.current_label_meta) else 0.0
            self._draw_label_adc(data, stm32_rel, pc_rel, frame_idx)

            # Video
            if self.current_label_cap is not None and self.current_label_cap.isOpened():
                v_idx = min(frame_idx, self.current_label_frame_count - 1)
                if v_idx == self._last_label_cap_frame_idx + 1:
                    ret, frame = self.current_label_cap.read()
                else:
                    self.current_label_cap.set(cv2.CAP_PROP_POS_FRAMES, v_idx)
                    ret, frame = self.current_label_cap.read()
                if ret:
                    self._last_label_cap_frame_idx = v_idx
                    self._show_label_frame(frame)

            if self.current_label_timestamps is not None and frame_idx < len(self.current_label_timestamps):
                t = float(self.current_label_timestamps[frame_idx])
            else:
                t = frame_idx * (self.current_label_radar_period_ms / 1000.0)

            self.lbl_label_time_info.config(text=f"Frame #{frame_idx} / #{len(self.current_label_adc)-1}  │  {t:.3f}s")
            self._draw_label_timeline()

    def _redraw_current_label_adc(self):
        self._on_label_slider_change(self.label_frame_var.get())

    def _on_lbl_bottom_split_resize(self, event=None):
        if event:
            bh = event.height
        else:
            bh = self.lbl_bottom_split.winfo_height()

        if bh < 60:
            return

        vh = max(bh - 46, 60)
        vw = int(vh * 16 / 9)

        self.lbl_video_panel.config(width=vw + 24)
        self._label_cam_target_w = vw
        self._label_cam_target_h = vh

    def _show_label_frame(self, frame):
        if frame is None:
            return
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        tw = getattr(self, "_label_cam_target_w", 480)
        th = getattr(self, "_label_cam_target_h", 270)

        img = Image.fromarray(rgb)
        img = img.resize((tw, th), Image.Resampling.BILINEAR)

        imgtk = ImageTk.PhotoImage(image=img)
        self.label_cam_label.imgtk = imgtk
        self.label_cam_label.config(image=imgtk)

    def _draw_label_adc(self, data, stm32_rel, pc_rel, idx):
        c = self.label_adc_canvas
        c.delete("all")
        w = c.winfo_width()
        h = c.winfo_height()
        if w < 50 or h < 50:
            return

        for x in range(0, w, 50):
            c.create_line(x, 0, x, h, fill="#f1f3f5", width=1)
        for y in range(0, h, 30):
            c.create_line(0, y, w, y, fill="#f1f3f5", width=1)

        if data is not None and len(data) > 1:
            data_arr = np.asarray(data, dtype=np.float64)

            view_mode = getattr(self, "label_view_mode", "waveform")
            if view_mode == "mti":
                plot_vals = self._compute_mti_frame(self.current_label_adc, idx)
                if plot_vals is None:
                    plot_vals = np.zeros_like(data_arr)
                line_color = PALETTE["accent_purple"]
                mode_name = "MTI (y[n]=x[n]-2x[n-1]+x[n-2])"
            elif view_mode == "iir_bg":
                plot_vals = self._compute_iir_bg_frame(self.current_label_adc, idx)
                if plot_vals is None:
                    plot_vals = np.zeros_like(data_arr)
                line_color = PALETTE["accent_cyan"]
                mode_name = "IIR Arka Plan (y[n]=x[n]-b[n])"
            else:
                plot_vals = data_arr
                line_color = PALETTE["accent_blue"]
                mode_name = "Ham ADC"

            n_samples = len(plot_vals)
            min_v = float(np.min(plot_vals))
            max_v = float(np.max(plot_vals))
            mean_v = float(np.mean(plot_vals))
            rms_v = float(np.sqrt(np.mean(plot_vals**2)))
            pp_v = max_v - min_v

            y_scale_mode = getattr(self, "y_scale_mode", "auto")
            if y_scale_mode == "fixed_25k":
                y_min, y_max = -25000.0, 25000.0
            elif y_scale_mode == "fixed_10k":
                y_min, y_max = -10000.0, 10000.0
            else:
                margin = max(abs(min_v), abs(max_v)) * 0.15 + 100.0
                limit = max(abs(min_v), abs(max_v)) + margin
                y_min, y_max = -limit, limit
            rng = max(y_max - y_min, 1.0)
            pad_y = 15

            zero_y = int(h - ((0.0 - y_min) / rng * (h - 2 * pad_y) + pad_y))
            c.create_line(0, zero_y, w, zero_y, fill="#ced4da", width=1, dash=(4, 4))

            points = []
            for i, val in enumerate(plot_vals):
                x = int((i / (n_samples - 1)) * (w - 20) + 10)
                y = int(h - ((val - y_min) / rng * (h - 2 * pad_y) + pad_y))
                points.extend([x, y])

            if len(points) >= 4:
                c.create_line(points, fill=line_color, width=2)

            self.label_adc_stats.config(
                text=f"{mode_name}  │  Min: {min_v:+.0f}  │  Max: {max_v:+.0f}  │  P-P: {pp_v:.0f}  │  Ort: {mean_v:+.1f}  │  RMS: {rms_v:.0f}  │  Örnek: {n_samples}  │  #{idx}"
            )
        else:
            c.create_text(w // 2, h // 2, text="Veri Yok", fill=PALETTE["fg_dim"], font=("Segoe UI", 12))
            self.label_adc_stats.config(text="Veri bulunamadı.")

    def _toggle_label_play(self):
        self._set_label_playing(not self.label_playing)

    def _set_label_playing(self, value):
        self.label_playing = value
        if self.label_playing:
            if self.current_label_adc is None:
                self.label_playing = False
                return
            if self.label_current_frame_idx >= len(self.current_label_adc) - 1:
                self.label_current_frame_idx = 0
            self.btn_label_play.set_color(PALETTE["accent_red"], "#ffffff", "#c2255c")
            self.btn_label_play.configure(text="⏸  DURDUR")
            self._label_last_tick = time.time()
            self._label_play_tick()
        else:
            self.btn_label_play.set_color(PALETTE["accent_green"], "#ffffff", "#237032")
            self.btn_label_play.configure(text="▶  OYNAT")

    def _label_play_tick(self):
        if not self.label_playing or self.current_label_adc is None:
            return
        total = len(self.current_label_adc)
        idx = self.label_current_frame_idx
        new_idx = idx + 1
        if new_idx >= total:
            new_idx = 0

        self.label_frame_var.set(new_idx)
        self._on_label_slider_change(new_idx)

        # Zaman damgası bazlı gecikme
        if self.current_label_timestamps is not None and idx + 1 < len(self.current_label_timestamps):
            dt_s = self.current_label_timestamps[idx + 1] - self.current_label_timestamps[idx]
            interval_ms = max(10, int(dt_s * 1000.0))
        else:
            interval_ms = max(10, int(self.current_label_radar_period_ms))

        self.root.after(interval_ms, self._label_play_tick)

    def _seek_label_relative(self, delta):
        if self.current_label_adc is None:
            return
        new_idx = max(0, min(len(self.current_label_adc) - 1, self.label_current_frame_idx + delta))
        self.label_frame_var.set(new_idx)
        self._on_label_slider_change(new_idx)

    def _seek_label_to_frame(self, idx):
        if self.current_label_adc is None or len(self.current_label_adc) == 0:
            return
        idx = max(0, min(len(self.current_label_adc) - 1, idx))
        self.label_frame_var.set(idx)
        self._on_label_slider_change(idx)

    # -------------------------------------------------------------------------
    # ETİKET İNCELEME: ZAMAN ÇİZELGESİ VE 2 NOKTA İŞARETLEME (TIMELINE & MARKS)
    # -------------------------------------------------------------------------
    def _draw_label_timeline(self):
        if not hasattr(self, "label_timeline_canvas"):
            return
        c = self.label_timeline_canvas
        c.delete("all")
        w = c.winfo_width()
        h = c.winfo_height()
        if w < 10 or h < 5 or self.current_label_adc is None or len(self.current_label_adc) <= 0:
            return

        total = len(self.current_label_adc)
        def x_of(idx):
            return int((idx / max(total - 1, 1)) * w)

        # Arka plan rayı
        c.create_rectangle(0, h // 2 - 2, w, h // 2 + 2, fill="#ced4da", outline="")

        # Seçili Etiket Bölgesi (Amber Vurgusu)
        if self.label_mark_start is not None and self.label_mark_end is not None:
            i0, i1 = sorted([self.label_mark_start, self.label_mark_end])
            c.create_rectangle(x_of(i0), 0, x_of(i1), h, fill="#ffe066", outline=PALETTE["accent_yellow"], width=1)

        # Başlangıç İşareti (Yeşil Bayrak)
        if self.label_mark_start is not None:
            x_s = x_of(self.label_mark_start)
            c.create_polygon(x_s - 4, 0, x_s + 4, 0, x_s, 7, fill=PALETTE["accent_green"], outline="")
            c.create_line(x_s, 0, x_s, h, fill=PALETTE["accent_green"], width=2)

        # Bitiş İşareti (Kırmızı Bayrak)
        if self.label_mark_end is not None:
            x_e = x_of(self.label_mark_end)
            c.create_polygon(x_e - 4, 0, x_e + 4, 0, x_e, 7, fill=PALETTE["accent_red"], outline="")
            c.create_line(x_e, 0, x_e, h, fill=PALETTE["accent_red"], width=2)

        # Oynatma İmleci (Mavi Çizgi)
        x_cur = x_of(self.label_current_frame_idx)
        c.create_line(x_cur, 0, x_cur, h, fill=PALETTE["accent_blue"], width=2)

    def _on_label_timeline_click(self, event):
        w = self.label_timeline_canvas.winfo_width()
        if w > 0 and self.current_label_adc is not None and len(self.current_label_adc) > 0:
            target_idx = int((event.x / w) * len(self.current_label_adc))
            self._seek_label_to_frame(target_idx)

    def _label_mark_start_point(self):
        if self.current_label_adc is None:
            return
        self.label_mark_start = self.label_current_frame_idx
        self._update_label_marks_label()
        self._draw_label_timeline()

    def _label_mark_end_point(self):
        if self.current_label_adc is None:
            return
        self.label_mark_end = self.label_current_frame_idx
        self._update_label_marks_label()
        self._draw_label_timeline()

    def _label_clear_marks(self):
        self.label_mark_start = None
        self.label_mark_end = None
        self._update_label_marks_label()
        self._draw_label_timeline()

    def _on_label_param_changed(self, *args):
        self._update_label_marks_label()

    def _update_label_marks_label(self):
        if not hasattr(self, "lbl_label_marks_summary"):
            return
        if self.current_label_adc is None or len(self.current_label_adc) == 0:
            self.lbl_label_marks_summary.config(text="Aralık: -", fg=PALETTE["fg_muted"])
            if hasattr(self, "lbl_label_split_preview"):
                self.lbl_label_split_preview.config(text="💡 Lütfen soldan bir etiket seçin.", fg=PALETTE["fg_muted"])
            return

        total_frames = len(self.current_label_adc)
        if self.label_mark_start is not None and self.label_mark_end is not None:
            i0, i1 = sorted([self.label_mark_start, self.label_mark_end])
            range_txt = f"#{i0} → #{i1}"
            cnt = i1 - i0 + 1
        elif self.label_mark_start is not None:
            i0 = self.label_mark_start
            i1 = total_frames - 1
            range_txt = f"#{i0} → #{i1}"
            cnt = i1 - i0 + 1
        elif self.label_mark_end is not None:
            i0 = 0
            i1 = self.label_mark_end
            range_txt = f"#0 → #{i1}"
            cnt = i1 + 1
        else:
            i0 = 0
            i1 = total_frames - 1
            range_txt = f"Tamamı (#0 → #{i1})"
            cnt = total_frames

        p_s = self.current_label_radar_period_ms / 1000.0
        dur_s = cnt * p_s
        self.lbl_label_marks_summary.config(text=f"Aralık: {range_txt} ({cnt} frame / {dur_s:.2f}s)", fg=PALETTE["fg_text"])

        try:
            w_size = int(self.label_model_input_size.get())
            stride = int(self.label_stride_size.get())
        except Exception:
            w_size = 20
            stride = 1

        if not hasattr(self, "lbl_label_split_preview"):
            return

        if w_size <= 0 or stride <= 0:
            self.lbl_label_split_preview.config(
                text="⚠️ Model input boyutu ve Stride en az 1 olmalıdır!",
                fg=PALETTE["accent_red"]
            )
        elif cnt < w_size:
            self.lbl_label_split_preview.config(
                text=f"⚠️ Seçilen aralık ({cnt} frame) < Model boyutu ({w_size} frame)! En az {w_size} frame seçilmeli.",
                fg=PALETTE["accent_red"]
            )
        else:
            num_slices = ((cnt - w_size) // stride) + 1
            self.lbl_label_split_preview.config(
                text=f"⚡ Önizleme: {cnt} frame aralıktan {w_size} frame model boyutu & {stride} stride ile {num_slices} adet veri üretilecek.",
                fg=PALETTE["accent_blue"]
            )

    # -------------------------------------------------------------------------
    # VERİYİ BÖL & ÇOĞALT (DATASET SLICING & STRIDE AUGMENTATION)
    # -------------------------------------------------------------------------
    def _split_augment_current_label(self):
        if self.current_label_idx is None or self.current_label_adc is None:
            messagebox.showwarning("Uyarı", "Lütfen önce soldaki listeden bölünecek bir etiket seçin!")
            return

        if not self.label_base_dir or not os.path.isdir(self.label_base_dir):
            messagebox.showerror("Hata", "Etiket klasörü yolu bulunamadı!")
            return

        try:
            w_size = int(self.label_model_input_size.get())
            stride = int(self.label_stride_size.get())
        except Exception:
            messagebox.showerror("Hata", "Model input boyutu ve Stride geçerli bir tam sayı olmalıdır!")
            return

        if w_size <= 0 or stride <= 0:
            messagebox.showerror("Hata", "Model input boyutu ve Stride en az 1 olmalıdır!")
            return

        total_frames = len(self.current_label_adc)
        if self.label_mark_start is not None and self.label_mark_end is not None:
            i0, i1 = sorted([self.label_mark_start, self.label_mark_end])
        elif self.label_mark_start is not None:
            i0, i1 = self.label_mark_start, total_frames - 1
        elif self.label_mark_end is not None:
            i0, i1 = 0, self.label_mark_end
        else:
            i0, i1 = 0, total_frames - 1

        i0 = max(0, min(i0, total_frames - 1))
        i1 = max(i0, min(i1, total_frames - 1))
        selected_count = i1 - i0 + 1

        if selected_count < w_size:
            messagebox.showwarning(
                "Yetersiz Frame Sayısı",
                f"Seçilen frame sayısı ({selected_count}), model input pencere boyutundan ({w_size}) küçük olamaz!\n\n"
                f"• Seçilen Aralık: Frame #{i0} - #{i1} ({selected_count} frame)\n"
                f"• İstenen Model Boyutu: {w_size} frame\n\n"
                f"Lütfen aralığı genişletin veya model boyutunu küçültün."
            )
            return

        num_slices = ((selected_count - w_size) // stride) + 1
        selected_item = self.label_list[self.current_label_idx]
        src_folder_name = selected_item["folder"]
        src_label_name = self.current_label_info.get("label", src_folder_name)

        confirm = messagebox.askyesno(
            "Veriyi Böl & Çoğalt (Augmentation)",
            f"Aşağıdaki ayarlarla veri çoğaltma işlemi başlatılacak:\n\n"
            f"• Kaynak Etiket: {src_label_name} ({src_folder_name})\n"
            f"• Seçilen Aralık: Frame #{i0} - #{i1} ({selected_count} frame)\n"
            f"• Model Input Boyutu: {w_size} frame\n"
            f"• Stride (Adım): {stride} frame\n"
            f"• Üretilecek Yeni Kesit Sayısı: {num_slices} adet\n\n"
            f"Bu işlem sonucunda 'labels' klasöründe {num_slices} adet yeni klasör oluşturulacaktır.\n"
            f"Devam etmek istiyor musunuz?"
        )
        if not confirm:
            return

        # İlerleme / Üretim
        self._set_label_playing(False)
        src_meta = self.current_label_meta
        src_timestamps = self.current_label_timestamps
        radar_p_ms = self.current_label_radar_period_ms
        p_s = radar_p_ms / 1000.0

        # Video hazırlığı
        has_video = self.current_label_video_path is not None and os.path.exists(self.current_label_video_path)
        v_cap = None
        v_w = 0
        v_h = 0
        v_fps = self.video_fps
        if has_video:
            v_cap = cv2.VideoCapture(self.current_label_video_path)
            if v_cap.isOpened():
                v_w = int(v_cap.get(cv2.CAP_PROP_FRAME_WIDTH))
                v_h = int(v_cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
                fps_val = v_cap.get(cv2.CAP_PROP_FPS)
                if fps_val and fps_val > 0:
                    v_fps = float(fps_val)
            else:
                has_video = False
                v_cap = None

        index_path = os.path.join(self.label_base_dir, "labels_index.json")
        index_data = []
        if os.path.exists(index_path):
            try:
                with open(index_path, "r", encoding="utf-8") as f:
                    index_data = json.load(f)
            except Exception:
                index_data = []

        created_folders = []

        try:
            for k in range(num_slices):
                sub_s = i0 + k * stride
                sub_e = sub_s + w_size  # exclusive index in current label

                # ADC dilimleme
                adc_subset = self.current_label_adc[sub_s:sub_e]
                adc_meta_subset = src_meta[sub_s:sub_e] if src_meta is not None and len(src_meta) >= sub_e else None

                # Klasör adı: {src_folder_name}_w{w_size}_s{stride}_{k+1:02d}
                raw_target_name = f"{src_folder_name}_w{w_size}_s{stride}_{k+1:02d}"
                target_folder = raw_target_name
                counter = 2
                while os.path.exists(os.path.join(self.label_base_dir, target_folder)):
                    target_folder = f"{raw_target_name}_{counter}"
                    counter += 1

                target_dir = os.path.join(self.label_base_dir, target_folder)
                os.makedirs(target_dir, exist_ok=True)
                created_folders.append(target_folder)

                # 1. ADC ve Meta Kaydet
                np.save(os.path.join(target_dir, "adc.npy"), np.array(adc_subset, dtype=object))
                if adc_meta_subset is not None:
                    np.save(os.path.join(target_dir, "adc_meta.npy"), adc_meta_subset)

                # 2. Video Kırpma & Kaydetme
                video_saved = False
                video_filename = None
                if v_cap is not None and v_cap.isOpened():
                    v_cap.set(cv2.CAP_PROP_POS_FRAMES, sub_s)
                    video_filename = f"video_{target_folder}.mp4"
                    video_path = os.path.join(target_dir, video_filename)
                    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
                    writer = cv2.VideoWriter(video_path, fourcc, v_fps, (v_w, v_h))
                    if not writer.isOpened():
                        video_filename = f"video_{target_folder}.avi"
                        video_path = os.path.join(target_dir, video_filename)
                        fourcc = cv2.VideoWriter_fourcc(*'MJPG')
                        writer = cv2.VideoWriter(video_path, fourcc, v_fps, (v_w, v_h))

                    if writer.isOpened():
                        written = 0
                        for _ in range(w_size):
                            ret, v_frame = v_cap.read()
                            if not ret:
                                break
                            writer.write(v_frame)
                            written += 1
                        writer.release()
                        if written > 0:
                            video_saved = True

                # 3. Zaman Damgaları & Süre
                if src_timestamps is not None and sub_e - 1 < len(src_timestamps):
                    t0 = float(src_timestamps[sub_s])
                    t1 = float(src_timestamps[sub_e - 1])
                    dur = round(abs(t1 - t0), 4)
                else:
                    t0 = round(sub_s * p_s, 4)
                    t1 = round((sub_e - 1) * p_s, 4)
                    dur = round(w_size * p_s, 4)

                # 4. label_info.json
                sub_label_name = f"{src_label_name}_w{w_size}_s{stride}_{k+1:02d}"
                sub_info = {
                    "label": sub_label_name,
                    "safe_name": self._sanitize_label_name(sub_label_name),
                    "folder_name": target_folder,
                    "source_record": self.current_label_info.get("source_record", ""),
                    "parent_label_folder": src_folder_name,
                    "parent_frame_range": [sub_s, sub_e - 1],
                    "model_input_size": w_size,
                    "stride": stride,
                    "slice_index": k + 1,
                    "total_slices": num_slices,
                    "start_frame": sub_s,
                    "end_frame": sub_e - 1,
                    "start_time_sec": t0,
                    "end_time_sec": t1,
                    "duration_sec": dur,
                    "adc_frame_count": int(w_size),
                    "adc_index_range": [sub_s, sub_e],
                    "video_frame_count": int(w_size),
                    "video_fps": round(v_fps, 4),
                    "video_file": video_filename if video_saved else None,
                    "created_at": datetime.now().isoformat(),
                    "radar_period_ms": radar_p_ms,
                    "augmented": True,
                    "note": f"Model input penceresi: {w_size} frame, Stride: {stride}. Kaynak: {src_folder_name} (Frame #{sub_s}-#{sub_e-1})"
                }
                with open(os.path.join(target_dir, "label_info.json"), "w", encoding="utf-8") as f:
                    json.dump(sub_info, f, ensure_ascii=False, indent=2)

                # 5. Index listesine ekleme
                index_data.append({
                    "folder": target_folder,
                    "label": sub_label_name,
                    "start_frame": sub_s,
                    "end_frame": sub_e - 1,
                    "duration_sec": dur,
                    "adc_frame_count": int(w_size),
                    "video_file": video_filename if video_saved else None,
                    "created_at": sub_info["created_at"],
                    "augmented": True
                })

            # Index dosyasını güncelle
            with open(index_path, "w", encoding="utf-8") as f:
                json.dump(index_data, f, ensure_ascii=False, indent=2)

        finally:
            if v_cap is not None:
                v_cap.release()

        # Listeyi yenile
        self._refresh_label_list()

        # Üretilen ilk klasörü seç
        if created_folders and self.label_list:
            for idx, item in enumerate(self.label_list):
                if item["folder"] == created_folders[0]:
                    if idx < self.label_listbox.size():
                        self.label_listbox.selection_clear(0, tk.END)
                        self.label_listbox.selection_set(idx)
                        self.label_listbox.see(idx)
                        self._on_label_select(None)
                    break

        messagebox.showinfo(
            "İşlem Tamamlandı",
            f"✅ Başarılı!\n\n"
            f"• Toplam {num_slices} adet {w_size} frame'lik veri kesiti üretildi.\n"
            f"• Stride: {stride} frame\n"
            f"• Kaydedilen Klasör: {self.label_base_dir}\n"
            f"• Üretilen kesitler sol menüye eklendi ve ilk kesit inceleme için açıldı."
        )

    def _delete_selected_label(self):
        if self.current_label_idx is None or not self.label_list:
            messagebox.showwarning("Uyarı", "Lütfen önce silinecek bir etiket seçin!")
            return
        item = self.label_list[self.current_label_idx]
        folder_path = item["path"]
        folder_name = item["folder"]

        confirm = messagebox.askyesno(
            "Etiketi Sil",
            f"'{item['name']}' etiketini kalıcı olarak silmek istediğinizden emin misiniz?\n\nKlasör: {folder_name}"
        )
        if not confirm:
            return

        try:
            self._set_label_playing(False)
            if self.current_label_cap is not None:
                self.current_label_cap.release()
                self.current_label_cap = None

            # Klasörü sil
            if os.path.exists(folder_path):
                shutil.rmtree(folder_path)

            # labels_index.json'dan sil
            index_path = os.path.join(self.label_base_dir, "labels_index.json")
            if os.path.exists(index_path):
                try:
                    with open(index_path, "r", encoding="utf-8") as f:
                        index_data = json.load(f)
                    index_data = [x for x in index_data if x.get("folder") != folder_name]
                    with open(index_path, "w", encoding="utf-8") as f:
                        json.dump(index_data, f, ensure_ascii=False, indent=2)
                except Exception:
                    pass

            self.current_label_idx = None
            self.current_label_adc = None
            self._refresh_label_list()
            messagebox.showinfo("Başarılı", "Etiket başarıyla silindi.")
        except Exception as e:
            messagebox.showerror("Hata", f"Etiket silinirken hata oluştu:\n{e}")

    # -------------------------------------------------------------------------
    # GÜVENLİ KAPATMA
    # -------------------------------------------------------------------------
    def on_close(self):
        self._set_playing(False)
        self._set_label_playing(False)
        if self.cap is not None:
            self.cap.release()
            self.cap = None
        if self.current_label_cap is not None:
            self.current_label_cap.release()
            self.current_label_cap = None
        self.root.destroy()


# =============================================================================
# BAŞLATMA
# =============================================================================
def launch_viewer():
    root = tk.Tk()
    app = DataViewerApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()


if __name__ == "__main__":
    launch_viewer()