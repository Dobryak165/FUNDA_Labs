@echo off
chcp 65001 >nul

echo Подготовка тестового файла input.txt
echo 1 John Doe 50000 > input.txt
echo 3 Alice Smith 60000 >> input.txt
echo 2 Bob Johnson 50000 >> input.txt
echo 4 Charlie Brown 40000.5 >> input.txt
echo 5 Eve Davis 70000 >> input.txt
echo Файл input.txt успешно создан!
echo.

echo Тест флага -a
.\Task2.exe input.txt -a out_a.txt
echo Файл out_a.txt создан:
type out_a.txt
echo.

echo Тест флага -d
.\Task2.exe input.txt -d out_d.txt
echo Файл out_d.txt создан:
type out_d.txt
echo.

echo Дополнительные проверки (Ошибки)
.\Task2.exe input.txt -a input.txt
echo.
.\Task2.exe input.txt -x out_x.txt
echo.
.\Task2.exe input.txt -a
echo.
.\Task2.exe no_file.txt -a out.txt
echo.
echo 1 John 123 50000 > bad_input.txt
echo 2 Alice_Smith 60000 >> bad_input.txt
.\Task2.exe bad_input.txt -a out_bad.txt
echo.

echo Очистка временных файлов
del input.txt out_a.txt out_d.txt bad_input.txt >nul 2>&1
echo Временные файлы удалены.
echo.

pause