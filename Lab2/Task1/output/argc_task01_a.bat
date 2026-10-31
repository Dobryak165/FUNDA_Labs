@echo off
chcp 65001 >nul

echo Тесты для флага -l
.\Task1.exe -l "Лаба по фунде"
echo.
.\Task1.exe -l "Привет"
echo.
.\Task1.exe -l ""
echo.

echo Тесты для флага -r
.\Task1.exe -r "Здрасте"
echo.
.\Task1.exe -r "Привет"
echo.

echo Тесты для флага -u
.\Task1.exe -u "zdraste"
echo.
.\Task1.exe -u "abcdefg"
echo.

echo Тесты для флага -n 
.\Task1.exe -n "a1b2c3!@# 4D5E"
echo.
.\Task1.exe -n "Hello 123 !!!"
echo.


echo Тесты для флага -c 
.\Task1.exe -c 42 "A" "B" "C" "D"
echo.
.\Task1.exe -c 100 "Moon" "Banana" "Sun" "Battery"
echo.

echo Дополнительные проверки
.\Task1.exe "Just a string"
echo.
.\Task1.exe -x "test"
echo.
.\Task1.exe -c 42
echo.
.\Task1.exe -c -5 "A" "B" " "
echo.

pause