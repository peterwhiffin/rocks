pushd build\win\bin
cl /Fepractice /EHsc /Zi /DEBUG /MDd user32.lib gdi32.lib shell32.lib ../../../src/main.c -IX:/repos/glad/include -IX:/repos/SDL/include /link /libpath:../../../lib/win SDL3.lib /NODEFAULTLIB:libcmt
popd
