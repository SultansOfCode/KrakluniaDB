cls
del *.kdb
del *.exe
gcc -o file_tests.exe -ggdb file_tests.c
file_tests.exe
