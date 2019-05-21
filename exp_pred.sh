#!/bin/bash

# Settings
vmssh=czy@192.168.122.168
vmName=vm1
pole=0.5
serviceLevel=$4
cap=$1
concurrency=$2
thinktime=$3
httpmon=./httpmon
actuator=./myactuator
lc=./Desktop/brownout-rubis-icse2014/PHP/ppLocalController.py
url="192.168.122.168/PHP/RandomItem.php"

# Helper functions
function setCap {
	echo [`date +%s`] cap=$1 >&8
	$actuator $vmName $1 &>> actuator.log
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
ssh $vmssh "pkill -f ppLocalController.py" || true

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

sleep 3 # let system settle
setStart
setThinkTime $thinktime
setCap $cap
setConcurrency $concurrency

# start local controller
ssh $vmssh "$lc --pole $pole --concurrency $concurrency --serviceLevel $serviceLevel --cap $cap --thinktime $thinktime" &> lc.log 
lcPid=$!

# stop experiment log channel
kill $expLogPid
wait $expLogPid || true

# stop http client
kill $httpmonPid
wait $httpmonPid || true

# done
cd ..
poleId=`echo $pole | tr -d .`
serviceLevelId=`echo $serviceLevel | tr -d .`

./processLog.py $resultsdir




