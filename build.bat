@echo off
call emsdk_env
echo Building...
call em++ main.cpp -o sdlopenglweb.html -s USE_SDL=2 -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 -s FULL_ES3=1 -lGL -s ALLOW_MEMORY_GROWTH=1
echo Build finished.
pause