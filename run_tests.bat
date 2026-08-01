@echo off
setlocal
cd /d "%~dp0"

if not exist bin mkdir bin

gcc -std=gnu90 -Wall -Wextra -Iinclude ^
 tests\test_logic.c src\input.c src\core.c src\storage.c src\transaction.c ^
 -o bin\pos_tests.exe

if errorlevel 1 (
    echo.
    echo Test build failed.
    pause
    exit /b 1
)

echo.
bin\pos_tests.exe
pause
endlocal
