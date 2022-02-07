# /bin/sh -f
g++ -g -o main src/acceptance.c src/err_handler.c src/file_handler.c src/io_handler.c -L libClas12Root.so `root-config --cflags --libs`

# NOTE. This should be made with makefile you bozo.
