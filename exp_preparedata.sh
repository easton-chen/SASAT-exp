#!/bin/bash

./resume_vm.sh

for cap in 350; do
	for concurrency in 500 ; do
		for preference in 1; do
			./exp_pattern.sh $cap $concurrency $preference 0.05
		done
	done
	
done

actuatorPid=$(cat childpid.txt)
kill $actuatorPid
