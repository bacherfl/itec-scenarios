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

SIMULATION_RUNS = 5
SIMULATION_OUTPUT = SIMULATION_DIR + "/output/"

#brite config file
scenario="brite_example"

briteConfig="--briteConfFile=brite_low_bw.conf"

bestRoute="--fw-strategy=bestRoute"
smartFlooding="--fw-strategy=smartflooding"
perContentBased="--fw-strategy=perContentBased"

allRoutes="--route=single"
singleRoute="--route=all"

# parameter search range
alpha_min = 0.05;
alpha_max = 0.1;
#alpha_max = 0.5;
alpha = []

while alpha_min <= alpha_max:
	alpha.append(alpha_min)
	alpha_min += 0.05

reliability_t_min = 0.95
#reliability_t_min = 0.75
reliability_t_max = 1.0
reliability_t = []

while reliability_t_min <= reliability_t_max:
	reliability_t.append(reliability_t_min)
	reliability_t_min += 0.05

probing_traffic_min = 0.0
probing_traffic_max = 0.1
#probing_traffic_max = 0.5
probing_traffic = []

while probing_traffic_min <= probing_traffic_max:
	probing_traffic.append(probing_traffic_min)
	probing_traffic_min += 0.1

update_intervall_min = 0.1
#update_intervall_max = 1.0
update_intervall_max = 0.3
update_intervall = []

while update_intervall_min <= update_intervall_max:
	update_intervall.append(update_intervall_min)
	update_intervall_min += 0.2

shift_traffic_min = 0.1
#shift_traffic_max = 0.5
shift_traffic_max = 0.2
shift_traffic = []

while shift_traffic_min <= shift_traffic_max:
	shift_traffic.append(shift_traffic_min)
	shift_traffic_min += 0.1

SCENARIOS= { }

for straffic in shift_traffic:
	for uinterval in update_intervall:
		for ptraffic in probing_traffic:
			for reliability in reliability_t:
				for a in alpha:
					scenario_name = "PerContentBased_AllRoutes_ST(" + "{:1.2f}".format(straffic) + ")_UI(" + "{:1.2f}".format(uinterval) + ")_PT(" + "{:1.2f}".format(ptraffic) + ")_R(" + "{:1.2f}".format(reliability) + ")_Alpha(" + "{:1.2f}".format(a) + ")"

					straffic_parm="--SHIFT_TRAFFIC={:1.2f}".format(straffic)
					uinterval_parm="--UPDATE_INTERVALL={:1.2f}".format(uinterval)
					ptraffic_parm="--PROBING_TRAFFIC={:1.2f}".format(ptraffic)
					reliability_parm="--RELIABILITY_THRESHOLD={:1.2f}".format(reliability)
					a_parm="--ALPHA={:1.2f}".format(a)

					entry = {scenario_name: { "executeable": scenario, "numRuns": SIMULATION_RUNS, "params": [briteConfig, perContentBased, allRoutes, straffic_parm,uinterval_parm,ptraffic_parm,reliability_parm,a_parm ] }}
					SCENARIOS.update(entry)

settings = 0
for entry in SCENARIOS:
	#print entry + ":"
	#print SCENARIOS[entry]
	#print "\n"
	settings +=1

print "total settings = " + str(settings)
exit(0)

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
