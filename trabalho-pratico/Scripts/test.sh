#!/bin/bash

make -j$(Scripts/get_threads.sh) testes &>/dev/null &&
if [ $# -ne 6 ]
then
	echo -e "Not enough args\nExpected 2x Dataset, input file, outputs directory for a total of 6 args"
else
	if [[ $(pwd) == "Scripts" ]]
	then
		echo "Please do not run in Scripts directory"
	else
		echo "Running with $1 $2 $3"
		./programa-testes $1 $2 $3 &>/dev/null
		cat testes_final_output.txt

		echo "Running with $4 $5 $6"
		./programa-testes $4 $5 $6 &>/dev/null
		cat testes_final_output.txt
	fi
fi
#cat testes_output.txt | grep -Ev "^CPU"
