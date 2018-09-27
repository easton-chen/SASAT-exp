#!/bin/bash

./resume_vm.sh

for cap in 25 50 100 150 180 240 300 350 400; do
	for concurrency in 200 400 800 1600 3200 6400; do
		for serviceLevel in 0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1; do
			for preference in 0 1 2 3 4 5; do
				./exp_pattern.sh $cap $concurrency $serviceLevel $preference 0.1
			done
		done
	done
done

actuatorPid=$(cat childpid.txt)
kill $actuatorPid
