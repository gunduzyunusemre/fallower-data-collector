import numpy as np
import struct
from datetime import datetime
from collections import deque
from typing import Optional, Tuple, Dict, Any
from typing import List

# core.logger'dan Logger'ı import et (veya Configuration üzerinden al)
# Logger'ı doğrudan import etmek daha basit olabilir
from core.logger import Logger


class Frame:
    """Radar sensöründen gelen bir veri çerçevesi"""
    def __init__(self, raw_data: bytes, frame_id: int, timestamp: Optional[datetime] = None):
        self.raw_data = raw_data
        self._processed_data: Optional[np.ndarray] = None
        self.timestamp = timestamp if timestamp else datetime.now()
        self.frame_id = frame_id

    def process(self, expected_features: int = None) -> np.ndarray:
        """Ham veriyi işleyerek numpy dizisine dönüştürür."""
        if self._processed_data is not None:
            return self._processed_data

        if self.raw_data is None:
            raise ValueError(f"Frame {self.frame_id}: No raw data to process")
        
        # Gerçek veri boyutunu kullan, padding yapma
        actual_bytes = len(self.raw_data)
        actual_features = actual_bytes // 2  # Her özellik 2 bayt
        
        try:
            # Gerçek veri boyutuna göre unpack yap
            format_string = f'<{actual_features}h'
            values = struct.unpack(format_string, self.raw_data)
            self._processed_data = np.array(values, dtype=np.float32)
            
            # Eğer expected_features verilmişse, log'a yaz ama değiştirme
            if expected_features and actual_features != expected_features:
                pass
                # Logger().debug(f"Frame {self.frame_id}: Actual feature count {actual_features} differs from expected {expected_features}")
                
            return self._processed_data

        except struct.error as e:
            raise ValueError(f"Frame {self.frame_id}: Failed to unpack raw data. Size: {len(self.raw_data)}, Error: {e}")
        except Exception as e:
            raise ValueError(f"Frame {self.frame_id}: Unexpected error during processing. Error: {e}")

class FrameWindow:
    """Tahmin için kullanılan çerçeve penceresi"""
    def __init__(self, window_size: int, stride: int):
        if not (isinstance(window_size, int) and window_size > 0):
            raise ValueError("window_size must be a positive integer")
        if not (isinstance(stride, int) and stride > 0):
            raise ValueError("stride must be a positive integer")

        self.window_size = window_size
        self.stride = stride
        self.frames = deque(maxlen=window_size)
        self.timestamps = deque(maxlen=window_size)
        self._frames_added_since_last_full = 0

    def add_frame(self, processed_data: np.ndarray, timestamp: datetime) -> bool:
        self.frames.append(processed_data)
        self.timestamps.append(timestamp)
        self._frames_added_since_last_full += 1

        is_full = len(self.frames) == self.window_size
        # Pencere dolduktan SONRA stride kadar frame gelince hazır olur
        # Yani ilk dolduğunda da hazır kabul edilebilir (stride=1 ise her zaman)
        # Stride'ı pencere dolduktan sonra saymaya başlarsak:
        # stride_reached = is_full and self._frames_added_since_last_full >= self.stride
        # Ama genellikle her stride adımında bir pencere istenir, doluluk kontrolü yeterli
        stride_reached = self._frames_added_since_last_full >= self.stride

        if is_full and stride_reached:
            # Stride için sayacı sıfırlarken dikkatli olmalı.
            # Eğer stride=1 ise, her frame'de sıfırlanır.
            # Eğer stride=5 ise, 5 frame sonra sıfırlanır.
            self._frames_added_since_last_full = 0 # Bir sonraki stride için sayacı sıfırla
            return True
        # Eğer pencere dolu ama stride henüz dolmadıysa False döner.
        # Eğer stride=1 ise ve pencere doluysa hep True döner.
        return False


    def get_window_data(self) -> Optional[np.ndarray]:
        if len(self.frames) < self.window_size:
            return None
        try:
            return np.stack(list(self.frames))
        except ValueError as e:
            Logger().error(f"Error stacking frames in window: {e}. Frame shapes: {[f.shape for f in self.frames]}", exc_info=True)
            return None
        except Exception as e:
             Logger().error(f"Unexpected error getting window data: {e}", exc_info=True)
             return None

    def get_time_range(self) -> Optional[Tuple[datetime, datetime]]:
        if len(self.timestamps) < self.window_size:
            return None
        # Deque'nin son elemanları en yeni olanlardır. Zaman aralığı ilkinden sonuncuya doğrudur.
        return self.timestamps[0], self.timestamps[-1]

    def clear(self):
        self.frames.clear()
        self.timestamps.clear()
        self._frames_added_since_last_full = 0

    def is_full(self) -> bool:
        return len(self.frames) == self.window_size


class PredictionResult:
    """Model tahmini sonucu"""
    # Global sample result for header generation in DataStorage
    # Bu biraz kirli bir yöntem, alternatif olarak header listesini DataStorage'da tanımlayabiliriz.
    _sample_dict_keys = ['label', 'confidence', 'confidence_percent', 'start_time', 'end_time', 'prediction_time']

    def __init__(self, label: str, confidence: float, time_range: Tuple[datetime, datetime], raw_scores: Optional[np.ndarray] = None):
        self.label = label
        self.confidence = confidence
        self.start_time = time_range[0]
        self.end_time = time_range[1]
        self.prediction_time = datetime.now()
        self.raw_scores = raw_scores

    def to_dict(self) -> Dict[str, Any]:
        start_str = self.start_time.strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
        end_str = self.end_time.strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
        pred_str = self.prediction_time.strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]

        return {
            'label': self.label,
            'confidence': f"{self.confidence:.4f}", # String olarak kaydetmek CSV için daha güvenli
            'confidence_percent': f"{self.confidence * 100.0:.2f}%", # String formatında
            'start_time': start_str,
            'end_time': end_str,
            'prediction_time': pred_str
            # Ham skorları CSV'ye eklemek isterseniz buraya ekleyebilirsiniz, ancak listeyi string'e çevirmeniz gerekir.
            # 'raw_scores': str(self.raw_scores.tolist()) if self.raw_scores is not None else ''
        }

    @classmethod
    def get_csv_header(cls) -> List[str]:
        """CSV başlığını döndürür."""
        # Örnek bir nesne oluşturup dict keylerini almak yerine sabit listeyi kullanabiliriz
        return cls._sample_dict_keys
        # Alternatif:
        # sample = cls("SAMPLE", 0.0, (datetime.now(), datetime.now()))
        # return list(sample.to_dict().keys())

    def __str__(self) -> str:
        start_str = self.start_time.strftime('%H:%M:%S.%f')[:-3]
        end_str = self.end_time.strftime('%H:%M:%S.%f')[:-3]
        return f"[{start_str} -> {end_str}] Prediction: {self.label} ({self.confidence*100:.2f}%)"