@echo off
REM Build script for theWheel with Python bindings on Windows
REM Usage: build.bat [Debug|Release]

setlocal

REM Default to Release build
set BUILD_TYPE=Release
if not "%1"=="" set BUILD_TYPE=%1

echo ========================================
echo Building theWheel
echo Build Type: %BUILD_TYPE%
echo ========================================
echo.

REM Check if CMake is available
where cmake >nul 2>nul
if errorlevel 1 (
    echo Error: CMake not found in PATH
    echo Please install CMake from https://cmake.org/download/
    exit /b 1
)

REM Check if Python is available
where python >nul 2>nul
if errorlevel 1 (
    echo Warning: Python not found in PATH
    echo Python bindings will not be usable without Python
    echo.
)

REM Create build directory
if not exist build mkdir build
cd build

echo Configuring with CMake...
cmake .. -DBUILD_PYTHON_BINDINGS=ON
if errorlevel 1 (
    echo.
    echo Error: CMake configuration failed
    cd ..
    exit /b 1
)

echo.
echo Building (this may take a few minutes)...
cmake --build . --config %BUILD_TYPE%
if errorlevel 1 (
    echo.
    echo Error: Build failed
    cd ..
    exit /b 1
)

cd ..

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Python module location:
echo   build\python_bindings\%BUILD_TYPE%\thewheel.pyd
echo.
echo To test the Python bindings, run:
echo   cd build\python_bindings
echo   python ..\..\python_bindings\example.py
echo.
echo See BUILD.md for more information.
echo.

endlocal
