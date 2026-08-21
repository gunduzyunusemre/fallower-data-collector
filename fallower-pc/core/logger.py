import logging
import threading

class Logger:
    _instance = None
    _lock = threading.Lock()

    def __new__(cls, log_level=logging.INFO, log_file=None):
        if cls._instance is None:
            with cls._lock:
                if cls._instance is None:
                    cls._instance = super(Logger, cls).__new__(cls)
                    cls._instance._setup_logger(log_level, log_file)
        return cls._instance

    def _setup_logger(self, log_level, log_file):
        self.logger = logging.getLogger("fallower")
        if self.logger.hasHandlers():
            self.logger.handlers.clear()

        self.logger.setLevel(log_level)

        console_handler = logging.StreamHandler()
        console_handler.setLevel(log_level)
        console_format = logging.Formatter(
            '%(asctime)s - %(levelname)-8s - %(threadName)-15s - %(name)s - %(message)s'
        )
        console_handler.setFormatter(console_format)
        self.logger.addHandler(console_handler)

        if log_file:
            try:
                file_handler = logging.FileHandler(log_file, mode='a', encoding='utf-8')  # ← Bu satırı değiştirin
                file_handler.setLevel(log_level)
                file_format = logging.Formatter(
                    '%(asctime)s - %(levelname)-8s - %(threadName)-15s - %(name)s - %(message)s'
                )
                file_handler.setFormatter(file_format)
                self.logger.addHandler(file_handler)
            except Exception as e:
                 # Logger henüz tam kurulmamış olabilir, print kullan
                print(f"ERROR: Failed to set up file handler for {log_file}: {e}")
                # self.logger.error(f"Failed to set up file handler for {log_file}: {e}")


    def get_logger(self):
        return self.logger

    def debug(self, message): self.logger.debug(message)
    def info(self, message): self.logger.info(message)
    def warning(self, message): self.logger.warning(message)
    def error(self, message, exc_info=False): self.logger.error(message, exc_info=exc_info)
    def critical(self, message, exc_info=False): self.logger.critical(message, exc_info=exc_info)