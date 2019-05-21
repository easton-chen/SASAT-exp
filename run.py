#!/usr/bin/env python
import os
configFile = open("configFile.txt","r")
for line in configFile:
	print line
	array = line.split(" ")
	cap = array[0]
	concurrency = array[1]
	thinktime = array[2]
	dimmer = array[3]
	cmd = "./exp_pred.sh " + str(cap) + " " + str(concurrency) + " " + str(thinktime) \
		+ " " + str(dimmer)
	os.system(cmd)
