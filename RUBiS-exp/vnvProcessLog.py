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
	lcLogLines = open(pathjoin(directory, 'lc.log')).readlines()
	
	#
	# Process lines
	#

	preference_order_list = [[0,1,2],[0,2,1],[1,0,2],[1,2,0],[2,0,1],[2,1,0]]
	
	for line in lcLogLines:
		try:
			data = re.search("data: (.+)", line).group(1)
		except AttributeError:
			pass
			#print("ignore")
	

	# write to files
	if data:
	    	data += '\n'
	    	vnv_data = open("vnv_data_new.txt", "a")
	    	vnv_data.write(data)
	    	vnv_data.close()
	    	print("data writen to file")

