#!/bin/bash

make clear
make delete
./resume_vm.sh

./exp_pred.sh 300 0.1 80

actuatorPid=$(cat childpid.txt)
kill $actuatorPid

