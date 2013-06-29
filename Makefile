.PHONY: ctags clean

all: slist dradix

dradix:
	gcc -g -Wall ./dradix.c ./dradix-test.c -o dradix-test

slist:
	gcc -g -Wall ./slist.c ./slist-test.c -o slist-test

ctags:
	ctags *.c

clean:
	rm dradix-test
	rm slist-test 
