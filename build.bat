@echo off
echo Building TS3 Plugin Packager...
echo.

if not exist build mkdir build
cd build

echo Running CMake...
cmake ..
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Building project...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Build successful!
echo Executable location: build\bin\Release\ts3_packager.exe
echo.
pause