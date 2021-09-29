@echo off
if not defined VCPKG_PATH goto :env_failed

set version=Debug
if /i "%1" == "release" set version=Release

rd /S/Q .\build
md build\bin\%version%
md .\logs
copy ..\.local\bin\*.dll .\build\bin\%version%\ /Y

cd build

cmake -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH% -DCMAKE_BUILD_TYPE=%version% -Wno-dev ../
if "%errorlevel%" NEQ "0" goto :cmake_failed

cmake --build . --config %version%
if "%errorlevel%" NEQ "0" goto :build_failed

ctest.exe --force-new-ctest-process -C %version%
if "%errorlevel%" NEQ "0" goto :test_failed

goto :end

REM Error Cases:
:env_failed
echo environment check failed, make sure VCPKG_PATH points to currect vcpkg path
exit /B

:cmake_failed
echo cmake configuration error !
goto :end

:test_failed
echo Failed to pass test(s) !
goto :end

:build_failed
echo Failed to build target(s) !
goto :end

:end
cd ..