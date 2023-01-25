#!/bin/bash

make -j$(Scripts/get_threads.sh) testes &>/dev/null &&
if [ $# -ne 3 ]
then
	echo -e "Not enough args\nExpected Dataset, input file, outputs directory"
else
	if [[ $(pwd) == "Scripts" ]]
	then
		echo "Please do not run in Scripts directory"
	else
		echo "Testing time of each query"
		echo "Running with $1 $2 $3"
		./programa-testes $1 $2 $3 &> /dev/null
		cat testes_final_output.txt

		echo "Testing time of load/free"
		for ((i = 0; i < 10; i++))
		do
			echo "Iteration $i"
			./programa-testes $1 &>/dev/null
		done
		./Scripts/parse_load_times.py

		echo "Testing max memory, in kbytes (once)"
		/usr/bin/time -f "%M" -o testes_final_output.txt ./programa-principal $1 $2 &> /dev/null
		cat testes_final_output.txt

		rm testes_final_output.txt
	fi
fi
#cat testes_output.txt | grep -Ev "^CPU"
