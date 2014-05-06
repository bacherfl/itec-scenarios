#! /usr/bin/python

import time
import numpy
import os
import glob
import collections
import shutil
import re
from subprocess import call

###NOTE Start this script FROM itec-scenarios MAIN-FOLDER!!!

SIMULATION_DIR=os.getcwd()

SIMULATION_RUNS=1
SIMULATION_OUTPUT=SIMULATION_DIR + "/output/"

def copyResults(src,dst):
	files = glob.glob(src + "/*.txt" );

	print src
	print dst

	if not os.path.exists(dst):
		os.makedirs(dst)
	else:
		shutil.rmtree(dst)
		os.makedirs(dst)
	
	for f in files:
		os.rename(f, dst+"/"+os.path.basename(f))

###script start
print "\nCurring working dir = " + SIMULATION_DIR + "\n"

#build project before
call([SIMULATION_DIR + "/waf"])

for i in range(0, SIMULATION_RUNS):
	call([SIMULATION_DIR+"/build/congavoid", 
				"--top="+SIMULATION_DIR+"/topologies/congavoid_100clients.top",
				"--mode=dash-avc",
				"--RngRun=" + str(i)])

	# move results
	copyResults(SIMULATION_OUTPUT, SIMULATION_OUTPUT+"output_run"+str(i))

print "Finished."
