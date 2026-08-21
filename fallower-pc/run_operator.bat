@echo off
title FALLOWER - Radar Veri Toplama Operatoru
cd /d "%~dp0"
python collect_operator.py
if errorlevel 1 pause
