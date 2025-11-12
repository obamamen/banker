@echo off
setlocal

pushd "%~dp0"

if not exist "builder" mkdir "builder"

echo =========================================================
echo Generating build systems in "builder"
echo =========================================================
echo:

echo ---------------------------------------------------------
echo [1/3] generating visual studio solution files
echo ---------------------------------------------------------
cmake -S "%CD%" -B "builder\vs" -G "Visual Studio 17 2022" -A x64
echo:

echo ---------------------------------------------------------
echo [2/3] generating ninja files
echo ---------------------------------------------------------
cmake -S "%CD%" -B "builder\ninja" -G "Ninja"
echo:

echo ---------------------------------------------------------
echo [3/3] generating GNU make files
echo ---------------------------------------------------------
cmake -S "%CD%" -B "builder\make" -G "MinGW Makefiles"
echo:

echo =========================================================
pause
popd