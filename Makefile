.PHONY: ctags

all:
	gcc -g -Wall ./main.c -o main

ctags:
	ctags *.c
