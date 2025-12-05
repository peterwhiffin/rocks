pushd build\win\bin
REM cl /Fepractice /EHsc /Zi /DEBUG /MDd user32.lib gdi32.lib shell32.lib ../../../src/main.c -IX:/repos/glad/include -IX:/repos/SDL/include /link /libpath:../../../lib/win SDL3.lib /NODEFAULTLIB:libcmt
clang -g -o practice.exe ../../../src/main.c -I../../../../glad/include -I../../../../SDL/include -lSDL3 -lm

popd

REM pushd build/lin/bin
REM clang -g -o practice.bin ../../../src/main.c -I../../../../glad/include -I../../../../SDL/include -lSDL3 -lm
REM
REM popd
