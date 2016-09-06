#!/bin/bash

let count1=0;
let count2=0;
let count3=0;
for f in $(ls ./tests/*.txt); do 
	./a.out 1  < $f > ./tests/`basename $f .txt`.output1;
	./a.out 2  < $f > ./tests/`basename $f .txt`.output2;
	./a.out 3  < $f > ./tests/`basename $f .txt`.output3;
	diff -Bw  ./tests/`basename $f .txt`.output1  ${f}.expected1 > ./tests/`basename $f .txt`.diff1;
	diff -Bw  ./tests/`basename $f .txt`.output2  ${f}.expected2 > ./tests/`basename $f .txt`.diff2;
	diff -Bw  ./tests/`basename $f .txt`.output3  ${f}.expected3 > ./tests/`basename $f .txt`.diff3;
done;

for f in $(ls tests/*.txt); do
	echo "========================================================";
	echo "TEST CASE:" `basename $f .txt`;
	echo "========================================================";
	d1=./tests/`basename $f .txt`.diff1;
	d2=./tests/`basename $f .txt`.diff2;
	d3=./tests/`basename $f .txt`.diff3;
	if [ -s $d1 ]; then
		echo "For task 1, there is an output missmatch:"
		cat $d1
	else
		count1=$((count1 + 1));
		echo "Task 1: Passed";
	fi
	echo "-----------------------------------------------";
	if [ -s $d2 ]; then
		echo "For task 2, there is an output missmatch:"
		cat $d2
	else
		count2=$((count2 + 1));
		echo "Task 2: Passed";
	fi
	echo "-----------------------------------------------";
	if [ -s $d3 ]; then
		echo "For task 3, there is an output missmatch:"
		cat $d3
	else
		count3=$((count3 + 1));
		echo "Task 3: Passed";
	fi
done

echo
echo "Task 1 correct count:" $count1;
echo "Task 2 correct count:" $count2;
echo "Task 3 correct count:" $count3;

rm tests/*.output?
rm tests/*.diff?

