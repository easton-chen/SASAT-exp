#!/bin/bash

# Settings
vmssh=czy@192.168.122.168
vmName=vm1
pole=0.5
serviceLevel=0
w0=0
w1=0
w2=0
cap=$1
concurrency=$2
thinktime=$4
preference=$3
httpmon=./httpmon
actuator=./myactuator
lc=./Desktop/brownout-rubis-icse2014/PHP/localController.py
IsVNV=$5
url="192.168.122.168/PHP/RandomItem.php"

if [ $IsVNV -eq 1 ]; then
	lc=./Desktop/brownout-rubis-icse2014/PHP/brownoutLocalController.py
fi

if [ $IsVNV -eq 2 ]; then
	lc=./Desktop/brownout-rubis-icse2014/PHP/attrLocalController.py
	serviceLevel=$9
	w0=$6
	w1=$7
	w2=$8
fi

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

# if vm is paused, resume it
#../resume_vm.sh

# make sure environment is clean
pkill -f httpmon || true
ssh $vmssh "pkill -f localController.py" || true

# start local controller
#stdbuf -o0 ssh $vmssh "stdbuf -o0 $lc --pole $pole --serviceLevel $serviceLevel --cap $cap --concurrency $concurrency --thinktime #$thinktime" &> lc.log &
#lcPid=$!

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
sleep 5 # let system settle
setStart
setThinkTime $thinktime
setConcurrency $concurrency
setCap $cap
#sleep 300

# start local controller
echo $preference
if [ $IsVNV -eq 2 ]; then
	ssh $vmssh "$lc --pole $pole --serviceLevel $serviceLevel --cap $cap --concurrency $concurrency --thinktime $thinktime --preference $preference --w0 $w0 --w1 $w1 --w2 $w2 --predictLatency $predictLatency" &> lc.log
else
	ssh $vmssh "$lc --pole $pole --serviceLevel $serviceLevel --cap $cap --concurrency $concurrency --thinktime $thinktime --preference $preference" &> lc.log
fi
lcPid=$!
#setCap 200

# stop experiment log channel
kill $expLogPid
wait $expLogPid || true

# stop http client
kill $httpmonPid
wait $httpmonPid || true

# stop local controller and copy results
#kill $lcPid
#wait $lcPid || true

# stop actuator
#actuatorPid=$(cat childpid.txt)
#kill $actuatorPid

# stop lc
#ssh $vmssh "pkill -f localController.py"

# done
cd ..
poleId=`echo $pole | tr -d .`
serviceLevelId=`echo $serviceLevel | tr -d .`
if [ $IsVNV -eq 0 ];then
	echo "start process.."
	./processLog.py $resultsdir
	echo "end process.."
else 
	./vnvProcessLog.py $resultsdir
fi
