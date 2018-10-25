#!/bin/bash

./resume_vm.sh

# 1 for brownout, 2 for ours
IsVNV=1

for cap in 100 400; do
	for concurrency in 200 1000 ; do
		for preference in 3 4 5 ; do
			./exp_pattern.sh $cap $concurrency $preference 0.05 $IsVNV
		done
	done
done

actuatorPid=$(cat childpid.txt)
kill $actuatorPid
