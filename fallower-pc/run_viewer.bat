@echo off
title FALLOWER - Radar Veri Goruntuleyici ve Etiketleme
cd /d "%~dp0"
python collect_viewer.py
if errorlevel 1 pause
