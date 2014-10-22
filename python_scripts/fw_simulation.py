#! /usr/bin/python

import time
import numpy
import os
import glob
import collections
import shutil
import re
from subprocess import call
import consumer_stats as consumer_stats

def copyResults(src,dst):
	files = glob.glob(src + "/*.txt" );

	print src
	print dst

	if not os.path.exists(dst):
		os.makedirs(dst)
	#else:
		#shutil.rmtree(dst)
		#os.makedirs(dst)
	
	for f in files:
		os.rename(f, dst+"/"+os.path.basename(f))
		
		
###NOTE Start this script FROM itec-scenarios MAIN-FOLDER!!!

SIMULATION_DIR=os.getcwd()

SIMULATION_RUNS = 10
SIMULATION_OUTPUT = SIMULATION_DIR + "/output/"

#brite config file
scenario="brite_example"

briteConfig="--briteConfFile=brite_low_bw.conf"

bestRoute="--fw-strategy=bestRoute"
smartFlooding="--fw-strategy=smartflooding"
perContentBased="--fw-strategy=perContentBased"

allRoutes="--route=single"
singleRoute="--route=all"

SCENARIOS = {
"BestRoute_AllRoutes":     { "executeable": scenario, "numRuns": SIMULATION_RUNS, "params": [briteConfig, bestRoute, allRoutes] },
"SmartFlooding_AllRoutes": { "executeable": scenario, "numRuns": SIMULATION_RUNS, "params": [briteConfig, smartFlooding, allRoutes] },
"PerContentBased_AllRoutes": { "executeable": scenario, "numRuns": SIMULATION_RUNS, "params": [briteConfig, perContentBased, allRoutes] },
"BestRoute_SingleRoute":     { "executeable": scenario, "numRuns": SIMULATION_RUNS, "params": [briteConfig, bestRoute, singleRoute] },
"SmartFlooding_SingleRoute": { "executeable": scenario, "numRuns": SIMULATION_RUNS, "params": [briteConfig, smartFlooding, singleRoute] },
"PerContentBased_SingleRoute": { "executeable": scenario, "numRuns": SIMULATION_RUNS, "params": [briteConfig, perContentBased, singleRoute] },
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
	# end for

	#compute statistics
	consumer_stats.generateStatsPerSimulation(SIMULATION_OUTPUT+scenarioName);
	
# end for
print "Finished."
