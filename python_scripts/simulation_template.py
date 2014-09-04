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


dashStr="--mode=dash-svc"
adapStr="--mode=adaptation"

SCENARIOS = {
# DASH
#"CongAvoid_DASH_6M_CWND":    { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck6.top", "--cwnd=tcp"] },
#"CongAvoid_DASH_9M_CWND":    { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck9.top", "--cwnd=tcp"] },
#"CongAvoid_DASH_12M_CWND":    { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--#top=congavoid_100clients_bottleneck12.top", "--cwnd=tcp"] },
#"CongAvoid_DASH_15M_CWND":    { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck15.top", "--cwnd=tcp"] },
#"CongAvoid_DASH_18M_CWND":    { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck18.top", "--cwnd=tcp"] },
#	"CongAvoid_DASH_20M_CWND":    { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck20.top", "--cwnd=tcp"] },
#	"CongAvoid_DASH_20M_STATIC":  { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck20.top", "--cwnd=static"] },
#	"CongAvoid_DASH_25M_CWND":    { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck25.top", "--cwnd=tcp"] },
#	"CongAvoid_DASH_25M_STATIC":  { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck25.top", "--cwnd=static"] },
#	"CongAvoid_DASH_30M_CWND":    { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck30.top", "--cwnd=tcp"] },
#	"CongAvoid_DASH_30M_STATIC":  { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck30.top", "--cwnd=static"] },
#	"CongAvoid_DASH_35M_CWND":    { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck35.top", "--cwnd=tcp"] },
#	"CongAvoid_DASH_35M_STATIC":  { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck35.top", "--cwnd=static"] },
#	"CongAvoid_DASH_40M_CWND":    { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck40.top", "--cwnd=tcp"] },
#	"CongAvoid_DASH_40M_STATIC":  { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [dashStr, "--top=congavoid_100clients_bottleneck40.top", "--cwnd=static"] },
# INA
"CongAvoid_INA_6M_CWND":     { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck6.top", "--cwnd=tcp"] },
"CongAvoid_INA_9M_CWND":     { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck9.top", "--cwnd=tcp"] },
"CongAvoid_INA_12M_CWND":     { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck12.top", "--cwnd=tcp"] },
"CongAvoid_INA_15M_CWND":     { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck15.top", "--cwnd=tcp"] },
	"CongAvoid_INA_18M_CWND":     { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck18.top", "--cwnd=tcp"] },
#	"CongAvoid_INA_20M_STATIC":   { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck20.top", "--cwnd=static"] },
#	"CongAvoid_INA_25M_CWND":     { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck25.top", "--cwnd=tcp"] },
#	"CongAvoid_INA_25M_STATIC":   { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck25.top", "--cwnd=static"] },
#	"CongAvoid_INA_30M_CWND":     { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck30.top", "--cwnd=tcp"] },
#	"CongAvoid_INA_30M_STATIC":   { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck30.top", "--cwnd=static"] },
#	"CongAvoid_INA_35M_CWND":     { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck35.top", "--cwnd=tcp"] },
#	"CongAvoid_INA_35M_STATIC":   { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck35.top","--cwnd=static"] },
#	"CongAvoid_INA_40M_CWND":     { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck40.top","--cwnd=tcp"] },
#	"CongAvoid_INA_40M_STATIC":   { "executeable": "congavoid", "numRuns": SIMULATION_RUNS, "params": [adapStr, "--top=congavoid_100clients_bottleneck40.top","--cwnd=static"] },
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
