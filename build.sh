pushd build/lin/bin
clang -g -o practice.bin ../../../src/main.c -I../../../../glad/include -I../../../../SDL/include -lSDL3 -lm

popd
