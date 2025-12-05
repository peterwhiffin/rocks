pushd build/lin/bin
clang -g -o practice.bin ../../../src/main.c -I../../../../glad/include -I../../../../SDL/include -I../../../../openal-soft/include -I../../../inc -L../../../lib/lin -lopenal -lSDL3 -lm

popd
