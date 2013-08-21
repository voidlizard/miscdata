.PHONY: ctags clean baseline check

all: slist dradix

dradix:
	gcc -g -Wall ./dradix.c ./dradix-test.c -o dradix-test

slist:
	gcc -g -Wall ./slist.c ./slist-test.c -o slist-test


hash:
	gcc -g -Wall ./slist.c ./hash-test.c -o hash-test

ctags:
	ctags *.c

clean:
	rm dradix-test
	rm slist-test

baseline: dradix
	./dradix-test list 2>&1 | xargs -L 1 t/dradix/mkbaseline.sh

check: dradix
	./dradix-test list 2>&1 | xargs -L 1 t/dradix/checkbaseline.sh

