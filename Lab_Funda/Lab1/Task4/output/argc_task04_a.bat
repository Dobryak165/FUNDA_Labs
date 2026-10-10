@echo off
chcp 65001 >nul

echo Тесты для флага -d
.\Task4.exe -d input_task04.txt
echo.
type out_input_task04.txt
echo.

echo Тесты для флага -i
.\Task4.exe -i input_task04.txt
echo.
type out_input_task04.txt
echo.

echo Тесты для флага -s
.\Task4.exe -s input_task04.txt
echo.
type out_input_task04.txt
echo.

echo Тесты для флага -a
.\Task4.exe -a input_task04.txt
echo.
type out_input_task04.txt
echo.

echo Тесты для флага -nd
.\Task4.exe -nd input_task04.txt my_result.txt
echo.
type my_result.txt
echo.

echo Дополнительные проверки
.\Task4.exe -x input_task04.txt
echo.
.\Task4.exe -d ahahaha.txt
echo.
.\Task4.exe -d
echo.

pause

