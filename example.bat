@echo off
REM Example usage of TS3 Plugin Packager

REM Build path (adjust if needed)
set PACKAGER=build\bin\Release\ts3_packager.exe

REM Check if executable exists
if not exist "%PACKAGER%" (
    echo Error: ts3_packager.exe not found!
    echo Please build the project first using build.bat
    pause
    exit /b 1
)

REM Example 1: Create a test plugin package
echo Creating test plugin package...
%PACKAGER% ^
    --name "Test Plugin" ^
    --author "TeamSpeak Systems GmbH" ^
    --version "1.0" ^
    --platforms "win32, win64, mac" ^
    --description "This is a test plugin." ^
    --input "path\to\your\plugin.dll" ^
    --output "TestPlugin"

echo.
echo Done! Check for TestPlugin.ts3_plugin in the current directory.
pause
