#!/bin/bash

./resume_vm.sh

IsAttr=0

for cap in 400; do
	for concurrency in 1000 ; do
		for preference in 3 4 5 ; do
			./exp_pattern.sh $cap $concurrency $preference 0.05 $IsAttr
		done
	done
done

actuatorPid=$(cat childpid.txt)
kill $actuatorPid
