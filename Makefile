all:
	gcc main.c library/*.c -o bin/test -Wall -Wpedantic -Wextra
	./bin/test