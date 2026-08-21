import queue
import threading
import logging
from datetime import datetime
from typing import Callable, Any, Dict

# core.enums'dan EventType'ı import et
from .enums import EventType

class Event:
    """Sistem içi olay iletişimi için temel sınıf"""
    def __init__(self, event_type: EventType, data: Any = None, source: str = None):
        self.event_type = event_type
        self.data = data
        self.source = source
        self.timestamp = datetime.now()

    def __str__(self):
        return f"Event(type={self.event_type.name}, source={self.source}, time={self.timestamp.strftime('%H:%M:%S.%f')[:-3]})"

class EventBus:
    """Sistem içi olay iletişimi için merkezi olay yöneticisi (Singleton)"""
    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(EventBus, cls).__new__(cls)
            cls._instance._subscribers: Dict[EventType, list[Callable[[Event], None]]] = {}
            cls._instance._event_queue = queue.Queue(maxsize=50000)  # Kuyruk boyutu sınırlı
            cls._instance._running = False
            cls._instance._thread = None
            cls._instance._lock = threading.Lock()
            
            # Öncelikli olay tipleri
            cls._instance._high_priority_events = [
                EventType.FRAME_RECEIVED,
                EventType.WINDOW_READY_FOR_PREDICTION,
                EventType.PREDICTION_READY,
                EventType.STATUS_CHANGED,
                EventType.ERROR_OCCURRED,
                EventType.FRAME_METADATA,      # ADD THIS
                EventType.RAW_FRAME_DATA,      # ADD THIS
                EventType.MODEL_INPUT_DATA     # ADD THIS (anticipating fix for point 2)
            ]
            
            # Veri kaydı olayları (düşük öncelikli)
            cls._instance._low_priority_events = [
                EventType.RAW_FRAME_RECEIVED,
                EventType.ADC_BEFORE,
                EventType.ADC_AFTER,
                EventType.FRAME_RESIZED,
                EventType.PRE_BANDPASS,
                EventType.POST_BANDPASS

            ]
            
        return cls._instance

    def subscribe(self, event_type: EventType, callback: Callable[[Event], None],
                  high_priority: bool = True):
        """
        Olaya abone ol.
        high_priority=False olan callback'ler EventBus yük altındayken atlanır.
        DataStorage gibi non-critical subscriber'lar False ile kayıt olmalı.
        """
        with self._lock:
            if event_type not in self._subscribers:
                self._subscribers[event_type] = []
            entry = (callback, high_priority)
            if entry not in self._subscribers[event_type]:
                self._subscribers[event_type].append(entry)

    def unsubscribe(self, event_type: EventType, callback: Callable[[Event], None]):
        with self._lock:
            if event_type in self._subscribers:
                self._subscribers[event_type] = [
                    entry for entry in self._subscribers[event_type]
                    if entry[0] is not callback
                ]

    def publish(self, event: Event):
        """Olayı önceliğine göre yayınla, kuyruk doluysa düşük öncelikli olayları atla"""
        if not self._running:
            print(f"WARN: EventBus not running. Discarding event: {event}")
            return
            
        try:
            # Kuyruk çok doluysa ve düşük öncelikli bir olaysa onu atla
            if (event.event_type in self._low_priority_events and 
                    self._event_queue.qsize() > self._event_queue.maxsize * 0.8):
                return
                
            # Olayı kuyruğa ekle
            self._event_queue.put_nowait(event)
        except queue.Full:
            if event.event_type in self._high_priority_events:
                # Yüksek öncelikli olay, bu yüzden son çare olarak bloklayan put kullan
                try:
                    self._event_queue.put(event, timeout=0.1)
                except queue.Full:
                    logging.warning(f"EventBus queue full, even high priority event discarded: {event}")
            # Düşük öncelikli olaylar sessizce atlanır

    def start(self):
        if not self._running:
            self._running = True
            self._thread = threading.Thread(target=self._event_loop, name="EventBusThread")
            self._thread.daemon = True
            self._thread.start()
            logging.info("EventBus started.") # Logger başlatıldıktan sonra bu çalışır

    def stop(self):
        self._running = False
        if self._thread and self._thread.is_alive():
            self._event_queue.put(None)
            self._thread.join(timeout=2.0)
        logging.info("EventBus stopped.")

    def _event_loop(self):
        logging.info("EventBus loop started.")
        while self._running:
            try:
                event = self._event_queue.get(timeout=1.0)
                if event is None:
                    break
                self._process_event(event)
                self._event_queue.task_done()
            except queue.Empty:
                continue
            except Exception as e:
                logging.error(f"Event processing error: {e}", exc_info=True)
        logging.info("EventBus loop finished.")

    def _process_event(self, event: Event):
        """Olayı işle - önceliğe göre abone yönetimi"""
        callbacks_to_run = []
        queue_busy = self._event_queue.qsize() > self._event_queue.maxsize * 0.5

        with self._lock:
            entries = self._subscribers.get(event.event_type, [])
            for callback, cb_high_priority in entries:
                # Kuyruk yoğunsa low-priority subscriber'ları atla
                if queue_busy and not cb_high_priority:
                    continue
                callbacks_to_run.append(callback)

        for callback in callbacks_to_run:
            try:
                callback(event)
            except Exception as e:
                logging.error(f"Error in event subscriber {callback.__qualname__}: {e}", exc_info=True)
                # ERROR_OCCURRED olayı işlenirken hata olursa tekrar publish etme (döngüyü kır)
                if event.event_type != EventType.ERROR_OCCURRED:
                    error_event = Event(EventType.ERROR_OCCURRED,
                                        data={"error": e, "callback": callback.__qualname__},
                                        source="EventBus")
                    self.publish(error_event)