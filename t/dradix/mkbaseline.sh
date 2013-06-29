#!/bin/sh
fn="t/dradix/$2-baseline"
./dradix-test $1 | sed -E 's/(#[[:digit:]]+l)/#PTR#/g' | sed 's/[ \t]*$//' > $fn

