#!/usr/bin/env python
import os
import re
from optparse import OptionParser

parser = OptionParser()
parser.add_option("--file", type="string", help="use this input file (default: %default)", default = "testdata.txt")
(options, args) = parser.parse_args()
testFile = options.file

expArgs = open(testFile, "r")
lines = expArgs.readlines()
thinktime = '0.01 '
isVNV = '1'
for line in lines:
	os.system('./resume_vm.sh')
	line = line.replace('[','')
	line = line.replace(']','')
	args = line.split(',', line.count(','))
	pre = args[0] + ' '
	cap = args[1] + ' '
	concurrency = args[2] + ' '
	responseTime = args[3]
	w1 = args[4] + ' '	 
	w2 = args[5] + ' '
	w3 = args[6] + ' '
	cmd = './exp_pattern.sh ' + cap + concurrency + pre + thinktime + isVNV
	print cmd
	os.system(cmd)
	childpid = open('childpid.txt', 'r')
	pid = childpid.read()
	if pid:
		cmd = 'kill ' + pid
		print cmd
		os.system(cmd)
	
