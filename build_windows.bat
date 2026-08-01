@echo off
setlocal
cd /d "%~dp0"

if not exist bin mkdir bin

gcc -std=gnu90 -Wall -Wextra -Iinclude ^
 src\main.c src\input.c src\core.c src\storage.c ^
 src\catalog.c src\transaction.c src\checkout.c src\reports.c ^
 -o bin\ResumePOS.exe

if errorlevel 1 (
    echo.
    echo Build failed.
    pause
    exit /b 1
)

echo.
echo Build successful: bin\ResumePOS.exe
echo.
bin\ResumePOS.exe
endlocal
