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

def generateStats(rootdir):

	print "RootDir = " + rootdir

	ratio = 0
	counter = 0

	for subdir, dirs, files in os.walk(rootdir):
		files = glob.glob(subdir + "/*aggregate-trace_*.txt" );

		stats = {}

		TIME_INDEX = 0
		NODE_INDEX = 1
		FACE_INDEX = 2 
		FACE_DESCRIPTION_INDEX = 3
		TYPE_INDEX = 4
		PACKET_NR_INDEX = 5

		total_number_of_requests = 0.0
		total_number_of_statisfied_requests = 0.0

		for file in files:
			#print  "processing file " + file

			f = open(file,"r");	

			#create first lvl of dictionary and init second level with 0
			for line in f:
				l = line.split('\t')
				if(len(l) < PACKET_NR_INDEX+1):
					continue

				if("dev=local" in l[FACE_DESCRIPTION_INDEX]):
					if(l[NODE_INDEX] not in stats):
						stats[l[NODE_INDEX]] = {}

					if(l[TYPE_INDEX] not in stats[l[NODE_INDEX]]):
						stats[l[NODE_INDEX]].update({l[TYPE_INDEX]: 0})

			#fill second levle with data
			f.seek(0)
			for line in f:
				l = line.split('\t')		
				if(len(l) < PACKET_NR_INDEX+1):
					continue

				if("dev=local" in l[FACE_DESCRIPTION_INDEX]):
					stats[l[NODE_INDEX]][l[TYPE_INDEX]] += int(l[PACKET_NR_INDEX])
		

		for key in stats:
			total_number_of_requests += stats[key]['InInterests']
			total_number_of_statisfied_requests += stats[key]['OutData']

		print "total_number_of_requests = " + str(total_number_of_requests)
		print "total_number_of_statisfied_requests = " + str(total_number_of_statisfied_requests)

		if total_number_of_requests > 0:
			output_file = open(subdir+"/STATS.txt", "w")
			output_file.write("total_number_of_requests: " + str(total_number_of_requests)+"\n")
			output_file.write("total_number_of_statisfied_requests: " + str(total_number_of_statisfied_requests)+"\n")
			output_file.write("ratio: " + str(total_number_of_statisfied_requests/total_number_of_requests)+"\n")

			ratio += (total_number_of_statisfied_requests/total_number_of_requests)
			counter += 1

	output_file = open(rootdir+"/STATS.txt", "w")
	output_file.write("ratio: " + str(ratio/counter) +"\n")


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

SIMULATION_RUNS = 2
SIMULATION_OUTPUT = SIMULATION_DIR + "/output/"

#brite config file
scenario="brite_example"

briteConfig="--briteConfFile=brite_daniel.conf"

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
		#call(tmp)
		
		# move results
	
		dst = SIMULATION_OUTPUT+scenarioName + "/output_run"+str(i)
		copyResults(SIMULATION_OUTPUT, dst)
		print "Results copied to", dst

		#compute statistics
		#generateStats(SIMULATION_OUTPUT+scenarioName);
		consumer_stats.generateStatsPerSimulation(SIMULATION_OUTPUT+scenarioName);
	# end for
	
# end for
print "Finished."
