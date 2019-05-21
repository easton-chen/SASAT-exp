#!/bin/bash

make clear
make delete
./resume_vm.sh

# change concurrency

#cat configFile.txt | while read myline
#do
#	echo "Config:"$myline
#	array=(${myline// / }) 
#	./exp_pred.sh ${array[0]} ${array[1]} ${array[2]} ${array[3]} 
#done
./run.py

actuatorPid=$(cat childpid.txt)
kill $actuatorPid

