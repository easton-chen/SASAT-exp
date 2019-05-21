#!/usr/bin/env python
from __future__ import print_function

from collections import defaultdict
from optparse import OptionParser
from os.path import join as pathjoin
import re
from sys import argv, stdin, stdout, stderr

# Reads experiment files from current directory (as dumped by do_stuff)
# and outputs results to traindata.txt:


#
# Helper functions
#
def avg(a):
	return sum(a) / len(a)

def stats(a):
	return min(a), avg(a), max(a)

#
# Process command-line
#
parser = OptionParser(usage = "usage: %prog DIRECTORIES...")
(options, args) = parser.parse_args()


for directory in args:
	#
	# Read all input files
	#
	expLogLines = open(pathjoin(directory, 'exp.log')).readlines()
	clientLogLines = open(pathjoin(directory, 'httpmon.log')).readlines()
	lcLogLines = open(pathjoin(directory, 'lc.log')).readlines()
	paramLogLines = open(pathjoin(directory, 'params')).readlines()

	#
	# Process lines
	#

	concurrency = '-'
	cap = '-'
	for line in paramLogLines:
		try:
			key, value = line.strip().split('=')
			if key == 'cap':
				cap = float(value)
			elif key == 'concurrency':
				serviceLevel = float(value)
		except ValueError:
			#print("Ignoring line {0}".format(line.strip()), file = stderr)
			pass

	lastTotal = 0

	totalRequests = 0
	totalErrors = 0
	totalRecommendations = 0
	

	traindata = open("env_ss_data.txt", "a+")
	for line in lcLogLines:
		try:
			No = int(re.search("No.([0-9]+)",line).group(1))
			CPU_cap = int(re.search("cap=([0-9]+)",line).group(1))
			concurrency = int(re.search("concurrency=([0-9]+)",line).group(1))
			thinktime = float(re.search("thinktime=([0-9.]+)",line).group(1))
			response_time = int(re.search(":\(([0-9]+)",line).group(1))
			uLatency = float(re.search("uLatency=([0-9.]+)",line).group(1))
			uTimeout = float(re.search("uTimeout=([0-9.]+)",line).group(1))
			serviceLevel = int(re.search("serviceLevel=([0-9]+)",line).group(1))
			# generate output data
			data_line = ','.join([str(CPU_cap), str(concurrency), str(thinktime), str(serviceLevel), str(uLatency), str(uTimeout)]) + '\n'
			traindata.write(data_line)
		except AttributeError:
			pass
			#print("ignore")
	traindata.close()
	
	print("data writen to file")

