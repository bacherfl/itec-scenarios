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

SIMULATION_RUNS = 2
SIMULATION_OUTPUT = SIMULATION_DIR + "/output/"

#xLargeCache="--cacheSize=75000"
#largeCache="--cacheSize=15000"
#mediumCache="--cacheSize=5000"
#smallCache="--cacheSize=1000"
MB256Cache="--cacheSize=64000"


bottleneck20="--top=dasheval_100clients_bottleneck20.top"
bottleneck25="--top=dasheval_100clients_bottleneck25.top"
bottleneck30="--top=dasheval_100clients_bottleneck30.top"
bottleneck35="--top=dasheval_100clients_bottleneck35.top"
bottleneck40="--top=dasheval_100clients_bottleneck40.top"
bottleneck50="--top=dasheval_100clients_bottleneck50.top"
bottleneck60="--top=dasheval_100clients_bottleneck60.top"

noBgTraffic="--bgTrafficClients=0"
oneBgTraffic="--bgTrafficClients=1"
twoBgTraffic="--bgTrafficClients=2"
threeBgTraffic="--bgTrafficClients=3"
fourBgTraffic="--bgTrafficClients=4"

SCENARIOS = {
	"DASH_20M_XLARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck20, MB256Cache, noBgTraffic] },
	"DASH_20M_XLARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck25, MB256Cache, noBgTraffic] },
	"DASH_30M_XLARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck30, MB256Cache, noBgTraffic] },
	"DASH_20M_XLARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck35, MB256Cache, noBgTraffic] },
	"DASH_40M_XLARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck40, MB256Cache, noBgTraffic] },
	
	
#### NO BG Traffic
# DASH, Small Cache, NO Background Traffic
#	"DASH_20M_SMALL_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck20, smallCache, noBgTraffic] },
#	"DASH_30M_SMALL_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck30, smallCache, noBgTraffic] },
#	"DASH_40M_SMALL_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck40, smallCache, noBgTraffic] },
#	"DASH_50M_SMALL_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck50, smallCache, noBgTraffic] },
#	"DASH_60M_SMALL_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck60, smallCache, noBgTraffic] },
# DASH, Medium Cache, NO Background Traffic
#	"DASH_20M_MEDIUM_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck20, mediumCache, noBgTraffic] },
#	"DASH_30M_MEDIUM_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck30, mediumCache, noBgTraffic] },
#	"DASH_40M_MEDIUM_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck40, mediumCache, noBgTraffic] },
#	"DASH_50M_MEDIUM_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck50, mediumCache, noBgTraffic] },
#	"DASH_60M_MEDIUM_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck60, mediumCache, noBgTraffic] },
# DASH, Large Cache, NO Background Traffic
#	"DASH_20M_LARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck20, largeCache, noBgTraffic] },
#	"DASH_30M_LARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck30, largeCache, noBgTraffic] },
#	"DASH_40M_LARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck40, largeCache, noBgTraffic] },
#	"DASH_50M_LARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck50, largeCache, noBgTraffic] },
#	"DASH_60M_LARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck60, largeCache, noBgTraffic] },
# DASH, XLarge Cache, NO Background Traffic
#	"DASH_20M_XLARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck20, xLargeCache, noBgTraffic] },
#	"DASH_30M_XLARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck30, xLargeCache, noBgTraffic] },
#	"DASH_40M_XLARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck40, xLargeCache, noBgTraffic] },
#	"DASH_50M_XLARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck50, xLargeCache, noBgTraffic] },
#	"DASH_60M_XLARGE_NOBG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck60, xLargeCache, noBgTraffic] },
	
#### 1 BG Traffic
# DASH, Small Cache, 1 BG Traffic
#	"DASH_20M_SMALL_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck20, smallCache, oneBgTraffic] },
#	"DASH_30M_SMALL_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck30, smallCache, oneBgTraffic] },
#	"DASH_40M_SMALL_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck40, smallCache, oneBgTraffic] },
#	"DASH_50M_SMALL_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck50, smallCache, oneBgTraffic] },
#	"DASH_60M_SMALL_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck60, smallCache, oneBgTraffic] },
# DASH, Medium Cache, 1 BG Traffic
#	"DASH_20M_MEDIUM_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck20, mediumCache, oneBgTraffic] },
#	"DASH_30M_MEDIUM_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck30, mediumCache, oneBgTraffic] },
#	"DASH_40M_MEDIUM_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck40, mediumCache, oneBgTraffic] },
#	"DASH_50M_MEDIUM_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck50, mediumCache, oneBgTraffic] },
#	"DASH_60M_MEDIUM_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck60, mediumCache, oneBgTraffic] },
# DASH, Large Cache, 1 BG Traffic
#	"DASH_20M_LARGE_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck20, largeCache, oneBgTraffic] },
#	"DASH_30M_LARGE_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck30, largeCache, oneBgTraffic] },
#	"DASH_40M_LARGE_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck40, largeCache, oneBgTraffic] },
#	"DASH_50M_LARGE_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck50, largeCache, oneBgTraffic] },
#	"DASH_60M_LARGE_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck60, largeCache, oneBgTraffic] },
# DASH, XLarge Cache, 1 BG Traffic
#	"DASH_20M_XLARGE_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck20, xLargeCache, oneBgTraffic] },
#	"DASH_30M_XLARGE_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck30, xLargeCache, oneBgTraffic] },
#	"DASH_40M_XLARGE_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck40, xLargeCache, oneBgTraffic] },
#	"DASH_50M_XLARGE_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck50, xLargeCache, oneBgTraffic] },
#	"DASH_60M_XLARGE_1BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck60, xLargeCache, oneBgTraffic] },
	
#### 2 BG Traffic
# DASH, Small Cache, 2 BG Traffic
#	"DASH_20M_SMALL_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck20, smallCache, twoBgTraffic] },
#	"DASH_30M_SMALL_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck30, smallCache, twoBgTraffic] },
#	"DASH_40M_SMALL_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck40, smallCache, twoBgTraffic] },
#	"DASH_50M_SMALL_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck50, smallCache, twoBgTraffic] },
#	"DASH_60M_SMALL_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck60, smallCache, twoBgTraffic] },
## DASH, Medium Cache, 2 BG Traffic
#	"DASH_20M_MEDIUM_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck20, mediumCache, twoBgTraffic] },
#	"DASH_30M_MEDIUM_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck30, mediumCache, twoBgTraffic] },
#	"DASH_40M_MEDIUM_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck40, mediumCache, twoBgTraffic] },
#	"DASH_50M_MEDIUM_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck50, mediumCache, twoBgTraffic] },
#	"DASH_60M_MEDIUM_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck60, mediumCache, twoBgTraffic] },
# DASH, Large Cache, 2 BG Traffic
#	"DASH_20M_LARGE_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck20, largeCache, twoBgTraffic] },
#	"DASH_30M_LARGE_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck30, largeCache, twoBgTraffic] },
#	"DASH_40M_LARGE_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck40, largeCache, twoBgTraffic] },
#	"DASH_50M_LARGE_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck50, largeCache, twoBgTraffic] },
#	"DASH_60M_LARGE_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck60, largeCache, twoBgTraffic] },
# DASH, XLarge Cache, 2 BG Traffic
#	"DASH_20M_XLARGE_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck20, xLargeCache, twoBgTraffic] },
#	"DASH_30M_XLARGE_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck30, xLargeCache, twoBgTraffic] },
#	"DASH_40M_XLARGE_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck40, xLargeCache, twoBgTraffic] },
#	"DASH_50M_XLARGE_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck50, xLargeCache, twoBgTraffic] },
#	"DASH_60M_XLARGE_2BG":    { "executeable": "evaldash", "numRuns": SIMULATION_RUNS, "params": [bottleneck60, xLargeCache, twoBgTraffic] },
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
