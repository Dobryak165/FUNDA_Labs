@echo off
chcp 65001 >nul

echo Тесты для флага -h
.\Task1.exe -1 -h
echo.
.\Task1.exe 2 -h
echo.
.\Task1.exe 10 -h
echo.
.\Task1.exe 25 -h
echo.
.\Task1.exe 100 -h
echo.
.\Task1.exe 150 -h
echo.

echo Тесты для флага -p
.\Task1.exe 0 -p
echo.
.\Task1.exe 1 -p
echo.
.\Task1.exe 2 -p
echo.
.\Task1.exe 13 -p
echo.
.\Task1.exe 15 -p
echo.

echo Тесты для флага -s
.\Task1.exe 12 -s
echo.
.\Task1.exe 16 -s
echo.
.\Task1.exe 4000 -s
echo.

echo Тесты для флага -e
.\Task1.exe 3 -e
echo.
.\Task1.exe 5 -e
echo.
.\Task1.exe 10 -e
echo.
.\Task1.exe 15 -e
echo.

echo Тесты для флага -a
.\Task1.exe wehkjeh -a
echo.
.\Task1.exe 5 -a
echo.
.\Task1.exe 50 -a
echo.
.\Task1.exe 100 -a
echo.
.\Task1.exe 200 -a
echo.

echo Тесты для флага -f
.\Task1.exe 2 -f
echo.
.\Task1.exe 5 -f
echo.
.\Task1.exe 15 -f
echo.
.\Task1.exe 21 -f
echo.

echo Дополнительные проверки
.\Task1.exe 093248509
echo.
.\Task1.exe 10 -q
echo.

pause