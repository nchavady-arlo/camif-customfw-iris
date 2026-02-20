#!/bin/sh

file1=$1
file2=$2
str=$3

if diff -q "$file1" "$file2" > /dev/null; then
	echo "$3: success"
else
	echo "$3: fail"
fi

