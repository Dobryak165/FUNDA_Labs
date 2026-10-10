@echo off
chcp 65001 >nul

echo Тесты для флага -q
.\Task3.exe 0.001 0 0 0 -q
echo.
.\Task3.exe 0.001 1 1 1 -q
echo.
.\Task3.exe 0.001 1 1 -2 -q
echo.
.\Task3.exe 0.001 4 2 -2 -q

echo Тесты для флага -m
.\Task3.exe 6 2 -m
echo.
.\Task3.exe 5 2 -m
echo.
.\Task3.exe 8 0 -m
echo.
.\Task3.exe 100 10 -m

echo Тесты для флага -t
.\Task3.exe 0.001 3 4 5 -t
echo.
.\Task3.exe 0.001 5 12 13 -t
echo.
.\Task3.exe 0.001 1 2 3 -t
echo.
.\Task3.exe 0.001 3 -4 5 -t
echo.
.\Task3.exe 0.001 1 1 1 -t
echo.
.\Task3.exe 0.001 -2 -2 -2 -t

echo Дополнительные проверки
.\Task3.exe lkfdgjldfkj 100 -m
echo.
.\Task3.exe 1 2 3 -v
echo.

pause