#!/bin/bash

./resume_vm.sh

for cap in 25 100 200 400; do
	for concurrency in 200 800 3200; do
		for serviceLevel in 0 0.5 1; do
			for preference in 0 2 4; do
				./exp_pattern.sh $cap $concurrency $serviceLevel $preference 0.1
			done
		done
	done
done

actuatorPid=$(cat childpid.txt)
kill $actuatorPid
