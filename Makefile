all:
	gcc main.c library/*.c -o bin/test -Wall -Werror -Wpedantic -Wextra
	./bin/test