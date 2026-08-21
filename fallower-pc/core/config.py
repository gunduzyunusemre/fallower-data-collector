import logging
import threading
import os
from enum import Enum

# core.logger'dan Logger'ı import et
from .logger import Logger # Changed back to relative import

class PreprocessingPipeline(Enum):
    """Preprocessing pipeline types"""
    RAW = "raw"                           # No processing
    RAW_NORMALIZE = "raw_normalize"       # Only normalization  
    BANDPASS = "bandpass"                 # Only bandpass filter
    BANDPASS_NORMALIZE = "bandpass_normalize"  # Bandpass + normalization

class ModelConfig:
    """Model configuration class"""
    def __init__(self, name: str, filename: str, pipeline: PreprocessingPipeline, 
                 class_labels: list, confidence_threshold: float = 0.7):
        self.name = name
        self.filename = filename
        self.pipeline = pipeline
        self.class_labels = class_labels
        self.confidence_threshold = confidence_threshold

class Configuration:

    _instance = None
    _lock = threading.Lock()

    def __new__(cls):
        if cls._instance is None:
            with cls._lock:
                if cls._instance is None:
                    cls._instance = super(Configuration, cls).__new__(cls)
                    cls._instance._load_defaults()
        return cls._instance

    def _load_defaults(self):
        """Varsayılan yapılandırma değerlerini yükle"""
        # Model okuma/yazma kilidi (set_current_model / get_current_model thread-safe olması için)
        self._model_lock = threading.Lock()

        # Model yolu için proje kök dizinini kullan
        project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) # Bu dosyanın 2 üst dizini (proje kökü)
        models_dir = os.path.join(project_root, 'models')  # Modeller models/ klasöründe
        
        # Define available models with their preprocessing pipelines
        self.available_models = {
            'bandpass': ModelConfig(
                name='MobileNet v3 Small - Bandpass',
                filename='mobilenetv3Small_bandpass_fallF1_95_mAcc_87.onnx',
                pipeline=PreprocessingPipeline.BANDPASS,
                class_labels=["FALL", "GETUP", "GETUPGR", "SIT", "STILLMOVE", "STILLNOACT", "WALK"],
                confidence_threshold=0.7
            ),
            'bandpass_normalize': ModelConfig(
                name='MobileNet v3 Small - Bandpass + Normalize',
                filename='mobilenetv3Small_bPassFltr-norm_fallF1_89_mAcc_85.onnx',
                pipeline=PreprocessingPipeline.BANDPASS_NORMALIZE,
                class_labels=["FALL", "GETUP", "GETUPGR", "SIT", "STILLMOVE", "STILLNOACT", "WALK"],
                confidence_threshold=0.7
            ),
            'raw': ModelConfig(
                name='MobileNet v3 Small - Raw Signal',
                filename='mobilenetv3Small_rawsignal_fallF1_88_mAcc_84.onnx',
                pipeline=PreprocessingPipeline.RAW,
                class_labels=["FALL", "GETUP", "GETUPGR", "SIT", "STILLMOVE", "STILLNOACT", "WALK"],
                confidence_threshold=0.7
            ),
            'raw_normalize': ModelConfig(
                name='MobileNet v3 Small - Raw + Normalize',
                filename='mobilenetv3Small_rawsignal_norm_fallF1_88_mAcc_81.onnx',
                pipeline=PreprocessingPipeline.RAW_NORMALIZE,
                class_labels=["FALL", "GETUP", "GETUPGR", "SIT", "STILLMOVE", "STILLNOACT", "WALK"],
                confidence_threshold=0.7
            ),
            'embedded_1ch': ModelConfig(
                name='MobileNet v3 Small - 1-Channel + Normalize (EMBEDDED)',
                filename='mobilenetv3Small_1ch__norm_fallF1_88_mAcc_81_opt13_constFold.onnx',
                pipeline=PreprocessingPipeline.RAW_NORMALIZE,
                class_labels=["FALL", "GETUP", "GETUPGR", "SIT", "STILLMOVE", "STILLNOACT", "WALK"],
                confidence_threshold=0.7
            )
        }

        # Set default model - USE EMBEDDED MODEL!
        self.current_model_key = 'embedded_1ch'  # Match embedded preprocessing!
        
        # Build full path for current model
        current_model = self.available_models[self.current_model_key]
        default_model_path = os.path.join(models_dir, current_model.filename)

        self.config = {
            'system': {
                'output_dir': 'output_data',
                'log_level': logging.DEBUG,
                'log_file': 'fallower.log'
            },
            'serial': {
                'port': 'COM8', # İşletim sistemine ve porta göre ayarlayın
                'baudrate': 115200,
                'timeout': 0.1,
                'read_chunk_size': 1024,  # DOUBLED for faster reading (512 → 1024)
            },
            'model': {
                'path': default_model_path,
                'current_key': self.current_model_key,
                'class_labels': current_model.class_labels,
                'confidence_threshold': current_model.confidence_threshold,
                'input_shape': (1, 30, 688),  # 3D format (batch, frames, features) - real shape from ONNX
                'models_directory': models_dir
            },
            
            'processing': {
                'window_size': 30,
                'stride': 14,  # Match embedded INFERENCE_STRIDE_3TASK (53% overlap)
                'frame_feature_count': 688,  # Update to match target length
                'save_processed_data': True,  # ← Bu TRUE olmalı
                'save_predictions': True,
                'filter_nfft': 1024,
                'filter_center_freq': 8e9,
                'filter_bandwidth': 1.9e9,
                'filter_sampling_rate': 10000,
                # Normalization parameters
                'normalize_target_min': -1000,
                'normalize_target_max': 1000,
            },
            'protocol': {
                'start_marker': b"<STR>",
                'data_marker': b"<DTA",
                'stop_marker': b"<STP>",
                'end_marker': b"<END>", 
                'ack_marker': b"<ACK>",
                'error_marker': b"<ERR>",
                'command_start': "START",
                'command_stop': "STOP"
            }
        }

    def get_available_models(self):
        """Get list of available models"""
        return self.available_models

    def get_current_model(self) -> ModelConfig:
        """Get current model configuration"""
        with self._model_lock:
            return self.available_models[self.current_model_key]

    def set_current_model(self, model_key: str) -> bool:
        """Set current model by key"""
        if model_key not in self.available_models:
            self.get_logger().error(f"Unknown model key: {model_key}")
            return False

        with self._model_lock:
            old_model = self.current_model_key
            new_model = self.available_models[model_key]

            self.current_model_key = model_key
            self.config['model']['current_key'] = model_key

            models_dir = self.config['model']['models_directory']
            new_model_path = os.path.join(models_dir, new_model.filename)
            self.config['model']['path'] = new_model_path
            self.config['model']['class_labels'] = new_model.class_labels
            self.config['model']['confidence_threshold'] = new_model.confidence_threshold

        self.get_logger().info(f"Model changed: {old_model} -> {model_key}")
        self.get_logger().info(f"New model: {new_model.name}")
        self.get_logger().info(f"Pipeline: {new_model.pipeline.value}")
        self.get_logger().info(f"Model file: {new_model.filename}")

        return True

    def get_preprocessing_pipeline(self) -> PreprocessingPipeline:
        """Get preprocessing pipeline for current model"""
        with self._model_lock:
            return self.available_models[self.current_model_key].pipeline

    def load_from_file(self, config_file):
        self.get_logger().warning(f"Loading configuration from file ({config_file}) is not yet implemented.")
        pass

    def get(self, section, key=None, default=None):
        # Model section'ı thread-safe okuma (set_current_model ile yarış koşulu önlenir)
        if section == 'model':
            with self._model_lock:
                if key is None:
                    return self.config.get(section, default if default is not None else {})
                return self.config.get(section, {}).get(key, default)
        if key is None:
            return self.config.get(section, default if default is not None else {})
        return self.config.get(section, {}).get(key, default)

    def set(self, section, key, value):
        if section not in self.config:
            self.config[section] = {}
        self.config[section][key] = value

    def get_logger(self):
        log_level = self.get('system', 'log_level', logging.INFO)
        log_file = self.get('system', 'log_file', 'fallower.log')
        return Logger(log_level, log_file)