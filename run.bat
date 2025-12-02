@echo off
pushd build\win\bin
start x:/rad/raddbg.exe --quit_after_success --auto_run practice.exe
popd
