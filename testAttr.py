import os
import re

expArgs = open("testdata.txt", "r")
lines = expArgs.readlines()
thinktime = '0.05 '
isVNV = '2'
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
	serviceLevel = args[7] + ''
	cmd = './exp_pattern.sh ' + cap + concurrency + pre + thinktime + isVNV + serviceLevel + w1 + w2 + w3 
	print cmd
	os.system(cmd)
	childpid = open('childpid.txt', 'r')
	pid = childpid.read()
	if pid:
		cmd = 'kill ' + pid
		print cmd
		os.system(cmd)
	
