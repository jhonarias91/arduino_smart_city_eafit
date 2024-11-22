@echo off
:: Cambiar al directorio donde está el archivo app.py
cd /d "%~dp0"

:: Ejecutar el script Python
python app.py

:: Pausar la terminal para mantenerla abierta
pause
