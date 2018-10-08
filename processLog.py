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
	BestY = 0
	BestLine = ''
	preference_order_list = [[0,1,2],[0,2,1],[1,0,2],[1,2,0],[2,0,1],[2,1,0]]
	
	for line in lcLogLines:
		try:
			No = int(re.search("No.([0-9]+)",line).group(1))
			Y = float(re.search("Y=([0-9.]+)",line).group(1))
			if(Y > BestY and No >= 3):
				BestY = Y
				BestLine = line
		except AttributeError:
			pass
			#print("ignore")
	
	
	# generate output data
	internal_line = ''
	external_line = ''
	CPU_cap = int(re.search("cap=([0-9]+)",BestLine).group(1))
	concurrency = int(re.search("concurrency=([0-9]+)",BestLine).group(1))
	response_time = int(re.search(":\(([0-9]+)",BestLine).group(1))
	init_latency = int(re.search("init_latency=([0-9]+)",BestLine).group(1))
	init_serviceLevel = int(re.search("init_serviceLevel=([0-9]+)",BestLine).group(1))
	new_serviceLevel = int(re.search("rr=([0-9]+)",BestLine).group(1))
	preference = int(re.search("preference=([0-9]+)",BestLine).group(1))
	preference_order = preference_order_list[preference]
	weights = re.search("weights=(\[.+\])", BestLine).group(1)

	external_line = ','.join([str(preference_order),str(CPU_cap),str(concurrency),str(init_latency),weights]) + '\n'
	internal_line = ','.join([str(init_latency), weights, str(new_serviceLevel)]) + '\n'

	# write to files
	traindata_internal = open("internal_data.txt", "a")
	traindata_internal.write(internal_line)
	traindata_internal.close()
	traindata_external = open("external_data.txt", "a")
	traindata_external.write(external_line)
	traindata_external.close()
	print("data writen to file")

