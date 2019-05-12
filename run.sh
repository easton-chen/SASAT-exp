#!/bin/bash

make clear
make delete
./resume_vm.sh

for cap in 100 200 300 400; do
	for concurrency in 100 500 1500 3000 ; do
		for thinktime in 0.05 0.1 0.2 0.4 ; do
			./exp_pattern.sh $cap $concurrency $thinktime 0
		done
	done
	make clear
done

actuatorPid=$(cat childpid.txt)
kill $actuatorPid

