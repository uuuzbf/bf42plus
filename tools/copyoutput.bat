@echo off
SETLOCAL ENABLEEXTENSIONS
IF NOT DEFINED BF1942_DIR (
  echo BF1942_DIR env var not set, copying output skipped
  EXIT 0
)
del "%BF1942_DIR%\d3d8_old.dll"
rename "%BF1942_DIR%\d3d8.dll" d3d8_old.dll
copy /y "%1" "%BF1942_DIR%\d3d8.dll"
