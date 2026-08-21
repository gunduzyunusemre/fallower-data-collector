## 📖 Genel Bakış (System Overview)

**FALLOWER**, 24GHz FMCW Radar teknolojisi kullanarak gizlilik ihlali yapmadan (kamera görüntüsü olmadan) ortamdaki bireylerin hareketlerini izleyen, düşme olaylarını milisaniyeler içinde tespit eden ve insan aktivitelerini tanıyan (HAR) uçtan uca entegre bir yapay zeka sistemidir.

Bu depo, sistemin iki ana sütununu tek bir çatı altında birleştirir:
1. **`fallower-stm32h7/`**: Radar sensörünü süren, yüksek hızlı ADC verilerini toplayan, USB üzerinden canlı yayınlayan ve cihaz üzerinde Edge AI çıkarımı koşturan **STM32H7 Gömülü Bellenimi**.
2. **`fallower-pc/`**: Radar verilerini kamera ile senkronize toplayan, canlı sinyal analizi yapan (Ham ADC, 3-Pulse MTI, IIR Arka Plan) ve yapay zeka modelleri için aralık etiketleyen **PC Veri Toplama & Etiketleme Stüdyosu**.

---

## 📂 Proje Monorepo Yapısı

```
fallower-data-collector/
├── fallower-pc/                    # 🖥️ PC Veri Toplama ve Etiketleme Yazılımı
│   ├── core/                       # Yapılandırma, EventBus ve loglayıcı
│   ├── interfaces/                 # Donanım seri port sürücüsü
│   ├── processing/                 # Veri işleme ve ONNX çıkarımı
│   ├── storage/                    # Veri depolama yöneticisi
│   ├── system/                     # Sistem yaşam döngüsü orkestrasyonu
│   ├── models/                     # Eğitilmiş ONNX modelleri
│   ├── visualization/              # NPZ ve veri dönüşüm araçları
│   ├── collect_main.py             # Ana terminal ve başlatıcı
│   ├── collect_operator.py         # Canlı Veri Toplama Arayüzü (Collector Pro)
│   ├── collect_viewer.py           # Sinyal İnceleme & Etiketleme Arayüzü (Viewer Pro)
│   ├── requirements.txt            # Python bağımlılıkları
│   ├── run_operator.bat            # Toplama arayüzünü tek tıkla başlatıcı
│   ├── run_viewer.bat              # İnceleme arayüzünü tek tıkla başlatıcı
│   └── README.md                   # PC bileşeni detaylı dokümantasyonu
│
├── fallower-stm32h7/               # ⚡ STM32H7 Gömülü Bellenimi (C/C++)
│   ├── Core/
│   │   ├── Inc/                    # Başlık dosyaları (.h / .hpp)
│   │   └── Src/                    # C/C++ kaynak kodları
│   │       ├── state/              # FreeRTOS Durum Makinesi sınıfları
│   │       ├── UsbCommunication.cpp# USB CDC protokolü ve veri akış motoru
│   │       ├── radar_driver.cpp    # 24GHz Radar SPI/Register sürücüsü
│   │       ├── radarHandler.cpp    # ADC örnekleme ve zamanlama yöneticisi
│   │       └── main.cpp            # Ana başlatıcı ve FreeRTOS görevleri
│   ├── Drivers/                    # STM32 HAL & CMSIS sürücüleri
│   ├── Middlewares/                # FreeRTOS & ST USB kütüphaneleri
│   ├── USB_DEVICE/                 # USB CDC konfigürasyonu
│   ├── X-CUBE-AI/                  # ST Edge AI çalışma zamanı
│   ├── STM32H743ZITX_FLASH.ld      # Linker script
│   ├── STM32_Git.ioc               # STM32CubeMX donanım yapılandırma dosyası
│   └── README.md                   # Gömülü yazılım detaylı dokümantasyonu
│
├── .gitignore                      # Evrensel Git filtreleme kural seti
└── README.md                       # Monorepo ana dokümantasyonu
```

---

## 🚀 Hızlı Başlangıç (Quick Start)

### 1. STM32H7 Kartını Hazırlama
1. `fallower-stm32h7` klasörünü **STM32CubeIDE** ile açın.
2. `Project -> Build Project` ile derleyin ve ST-Link ile karta yükleyin (Flash).
3. Kartın USB portunu bilgisayara bağlayın.

### 2. PC Veri Toplama Arayüzünü Başlatma
```bash
cd fallower-pc
python -m venv venv
venv\Scripts\activate
pip install -r requirements.txt

# Canlı Veri Toplama Stüdyosu:
python collect_operator.py
# veya run_operator.bat dosyasına çift tıklayın
```

### 3. Sinyal İnceleme & Aralık Etiketleme
```bash
cd fallower-pc
# Sinyal Görüntüleyici ve Etiketleme Stüdyosu:
python collect_viewer.py
# veya run_viewer.bat dosyasına çift tıklayın
```

---

## ⌨️ Klavye Kısayolları (Viewer)

| Kısayol | İşlev |
|:---|:---|
| **`Space`** | Sinyal ve Video Senkron Oynat / Duraklat |
| **`←` / `→`** | 1 Frame Geri / İleri Git |
| **`Shift + ←` / `Shift + →`** | 10 Frame Hızlı Sar |
| **`I` / `O`** | Başlangıç / Bitiş Frame'ini İşaretle (Mark In/Out) |
| **`S`** | İşaretli Aralığı Kırp ve Etiket Olarak Kaydet |
| **`Esc`** | Aralık İşaretlerini Temizle |

---

## 📄 Lisans

Bu proje özel araştırma ve geliştirme projesi kapsamında geliştirilmiştir. Tüm hakları saklıdır.
