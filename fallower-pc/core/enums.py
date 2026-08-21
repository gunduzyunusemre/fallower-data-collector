from enum import Enum

class SystemStatus(Enum):
    """Sistem durumunu takip etmek için enumerasyon"""
    IDLE = 0
    INITIALIZING = 1
    READY = 2
    CALIBRATING = 3
    STREAMING = 4
    ERROR = 6
    SHUTDOWN = 7
    STOPPING = 8

class EventType(Enum):
    """Sistem olayları için enumerasyon"""
    # Mevcut olay tipleri
    FRAME_RECEIVED = 0
    FRAME_PROCESSED = 1
    WINDOW_READY_FOR_PREDICTION = 2
    PREDICTION_READY = 3
    COMMAND_SENT = 4
    COMMAND_RESPONSE = 5
    STATUS_CHANGED = 6
    ERROR_OCCURRED = 7
    SYSTEM_START = 8
    SYSTEM_STOP = 9
    
    # Veri işleme adımları için yeni olay tipleri
    RAW_FRAME_RECEIVED = 10    # Seri porttan gelen ham veri
    ADC_BEFORE = 11            # 16-bit ADC değerleri
    ADC_AFTER = 12             # 12-bit'e dönüştürülmüş ADC
    FRAME_RESIZED = 13         # 688 boyutuna çevrilmiş
    PRE_BANDPASS = 14          # Bandpass filtresi öncesi
    POST_BANDPASS = 15         # Bandpass filtresi sonrası
    
    FRAME_METADATA = 16      # Meta veriler için
    RAW_FRAME_DATA = 17      # Ham frame verisi için
    MODEL_INPUT_DATA = 18    # Model girdi verisi için (DataProcessor'da kullanılacak)