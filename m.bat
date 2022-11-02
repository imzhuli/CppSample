@echo off

cd build

set version=Debug
if /i "%1" == "release" set version=Release

cmake --build . --config %version%
if "%errorlevel%" NEQ "0" goto :build_failed

ctest.exe --force-new-ctest-process -C %version%
if "%errorlevel%" NEQ "0" goto :test_failed

goto :end

REM Error Cases:

:test_failed
echo Failed to pass test(s) !
goto :end

:build_failed
echo Failed to build target(s) !
goto :end

:end
cd ..
