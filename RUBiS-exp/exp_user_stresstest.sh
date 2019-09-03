#!/bin/bash

# Settings
vmssh=czy@192.168.122.168
vm=vm1
pole=$1
serviceLevel=$2
httpmon=./httpmon
actuator=./myactuator
lc=./Desktop/brownout-rubis-icse2014/PHP/localController.py
url="http://192.168.122.168/PHP/RandomItem.php"

# Helper functions
function setCap {
	echo [`date +%s`] cap=$1 >&8
	$actuator $vm $1 &>> actuator.log
}
function setThinkTime {
	echo [`date +%s`] thinktime=$1 >&8
	echo "thinktime=$1" >&9
}
function setConcurrency {
	echo [`date +%s`] concurrency=$1 >&8
	echo "concurrency=$1" >&9
}
function setStart {
	echo [`date +%s`] start >&8
}

#
# Experimental protocol
#

# Stop on any error
set -e

# Resolve relative paths
actuator=`readlink -f $actuator`
httpmon=`readlink -f $httpmon`

# Create results directory
resultsbase=`date +%Y-%m-%dT%H:%M:%S%z`
resultsdir=exp_$resultsbase
mkdir $resultsdir
cd $resultsdir

# make sure environment is clean
pkill -f httpmon || true
ssh $vmssh "pkill -f localController.py" || true

# start local controller
ssh $vmssh "$lc --pole $pole --serviceLevel $serviceLevel" & #> lc.log &
lcPid=$!

# start (but do not activate) http client
mkfifo httpmon.fifo
$httpmon --url $url --concurrency 0 --timeout 10 < httpmon.fifo &> httpmon.log &
httpmonPid=$!
exec 9> httpmon.fifo

# open log for experiment
mkfifo exp.fifo
tee exp.log < exp.fifo &
expLogPid=$!
exec 8> exp.fifo

# output starting parameters
( set -o posix ; set ) > params # XXX: perhaps too exhaustive
#git log -1 > version

# change parameters
#setCap 200
sleep 10 # let system settle
setStart
setThinkTime 0.1
setCap 100
for concurrency in 500 2000 5000 10000; do
	setConcurrency $concurrency
	sleep 100
done
sleep 10
#setCap 200

# stop experiment log channel
kill $expLogPid
wait $expLogPid || true

# stop http client
kill $httpmonPid
wait $httpmonPid || true

# stop local controller and copy results
# stop lc
ssh $vmssh "pkill -f localController.py"

# done
