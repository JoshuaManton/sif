@echo off
if not exist "bin" mkdir bin
cl /MP /Zi /Od /Fd /Iexternal main.cpp lexer.cpp parser.cpp checker.cpp c_backend.cpp basic.cpp user32.lib ole32.lib oleaut32.lib /EHsc /link /INCREMENTAL:no /OUT:bin/sif.exe /DEBUG
