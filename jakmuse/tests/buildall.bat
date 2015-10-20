@echo off
setlocal

for %%i in (*.song) do (
    echo %%i
    ..\jakmuse.exe -i "%%i" -w "%%~ni.wav"
)
