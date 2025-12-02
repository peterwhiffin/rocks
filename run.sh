pushd build/lin/bin 
gdb -q -ex "set debuginfod enabled off" practice.bin -ex=run -ex=quit
popd
