REM this doesn't work yet
pushd build\win\bin
clang -g -o practice.exe ../../../src/main.c -I../../../../glad/include -I../../../../SDL/include -lSDL3 
popd

