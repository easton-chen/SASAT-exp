#!/bin/bash

./resume_vm.sh

IsAttr=1

for cap in 25 50 75 100 125 150 175 200 225 250 275 300 325 350 375 400; do
	for concurrency in 100 200 500 1000 1500 2000 2500 3000 3500 4000 ; do
		for preference in 3 4 5 ; do
			./exp_pattern.sh $cap $concurrency $preference 0.05 $IsAttr
		done
	done
	make clear
done

actuatorPid=$(cat childpid.txt)
kill $actuatorPid
