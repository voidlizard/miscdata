#!/bin/sh
tmp=`mktemp`
./dradix-test $1 | sed -E 's/(#[[:digit:]]+l)/#PTR#/g' | sed 's/[ \t]*$//' > $tmp
diff t/dradix/$2-baseline $tmp
status=$?
echo -n "TEST " $1 $2
if [ $status -eq 0 ]; then
	echo " OK"
else
	echo " FAIL"
fi
rm $tmp
