#! /usr/bin/python

import time
import numpy
import os
import glob
import collections
import shutil
import re
import calculate_average as calculate_average
from subprocess import call

###NOTE Start this script FROM itec-scenarios MAIN-FOLDER!!!

SIMULATION_DIR=os.getcwd()

SIMULATION_RUNS = 10
SIMULATION_OUTPUT = SIMULATION_DIR + "/output/"
SCENARIO = "congavoid"
MODE = "dash-svc"
TOPOLOGY="congavoid_100clients.top"

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
	print "Simulation run " + str(i) + " in progress.. lauchning scenario: " + SCENARIO
	call([SIMULATION_DIR+"/build/" + SCENARIO, 
				"--top=" + SIMULATION_DIR+"/topologies/" + TOPOLOGY,
				"--mode=" + MODE,
				"--RngRun=" + str(i)])

	# move results
	
	dst = SIMULATION_OUTPUT+"output_run"+str(i)
	copyResults(SIMULATION_OUTPUT, dst)
	print "Results copied"

	#compute statistics
	calculate_average.computeStats(dst)

print "Finished."
