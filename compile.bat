gcc -c -o add_basic.o cpp_code.cpp

gcc -o add_basic.dll -s -shared add_basic.o -Wl,--subsystem,windows