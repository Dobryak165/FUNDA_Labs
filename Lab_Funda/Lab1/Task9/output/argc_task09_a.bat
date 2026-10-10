@echo off
chcp 65001 >nul

.\Task9.exe -50 50
echo.

timeout /t 1 >nul

.\Task9.exe 0 100
echo.

.\Task9.exe abc 50
echo.
.\Task9.exe 100 -100
echo.
.\Task9.exe 5
echo.

pause