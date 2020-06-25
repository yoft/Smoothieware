@echo off
echo F|xcopy /Y /F LPC1768\main.bin G:\firmware.bin
timeout /t 2 > NUL