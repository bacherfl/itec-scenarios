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
		
		
###NOTE Start this script FROM itec-scenarios MAIN-FOLDER!!!

SIMULATION_DIR=os.getcwd()

SIMULATION_RUNS = 30
SIMULATION_OUTPUT = SIMULATION_DIR + "/output/"

SCENARIOS = {
	"FWT_DASH_LARGE_20M_CWND":     { "executeable": "fwt_dash_large", "numRuns": SIMULATION_RUNS, "params": ["--top=congavoid_100clients_bottleneck20.top"] },
	"FWT_DASH_LARGE_25M_CWND":     { "executeable": "fwt_dash_large", "numRuns": SIMULATION_RUNS, "params": ["--top=congavoid_100clients_bottleneck25.top"] },
	"FWT_DASH_LARGE_30M_CWND":     { "executeable": "fwt_dash_large", "numRuns": SIMULATION_RUNS, "params": ["--top=congavoid_100clients_bottleneck30.top"] },
}


#build project before
call([SIMULATION_DIR + "/waf"])

###script start
print "\nCurring working dir = " + SIMULATION_DIR + "\n"



for scenarioName in SCENARIOS.keys():
	runs = SCENARIOS[scenarioName]['numRuns']
	executeable = SCENARIOS[scenarioName]['executeable']
	
	executeable = "build/" + executeable
	print "------------------------------------------------------------------------"
	print "Starting", runs , "simulations of", scenarioName
	
	for i in range(0, runs):
		print "----------"
		print "Simulation run " + str(i) + " in progress..." 
		tmp = [SIMULATION_DIR+"/" + executeable] +  SCENARIOS[scenarioName]['params'] + ["--RngRun=" + str(i)]
		print tmp
		call(tmp)
		
		# move results
	
		dst = SIMULATION_OUTPUT+scenarioName + "/output_run"+str(i)
		copyResults(SIMULATION_OUTPUT, dst)
		print "Results copied to", dst

		#compute statistics
		calculate_average.computeStats(dst)
	# end for
	
# end for

exit();





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
