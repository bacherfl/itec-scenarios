#! /usr/bin/python

import time
import numpy
import os
import glob
import collections
import shutil
import re
import subprocess
from subprocess import call
import threading
import time
import consumer_stats as consumer_stats

curActiveThreads = 0
invalid_runs = 0

class Thread(threading.Thread):
    # init
  def __init__(self,job_number, sys_cal, callback_method, src ,dst):
		super(Thread,self).__init__()
		self.sysCall = sys_cal
		self.jobNumber = job_number
		self.callback = callback_method
		self.src = src
		self.dst = dst

		print "JOBNUMMER = " + str(job_number)
		print "JOBNUMMER = " + str(src)

  # overwriting run method of threading.Thread (do not call this method, call thread.start() )
  def run(self):

		if not os.path.exists(self.src+"/traces"):
			os.makedirs(self.src+"/traces")

		fpOut = open("t_" + str(self.jobNumber) + ".stdout.txt", "w")

		# start subprocess
		proc = subprocess.Popen(self.sysCall,stdout=fpOut, cwd=self.src)
		proc.communicate() # wait until finished

		# sleep 0.5 seconds to be sure the OS really has finished the process
		time.sleep(0.5)

		fpOut.close()
		os.remove("t_" + str(self.jobNumber) + ".stdout.txt")

		# callback
		print "threadFinishedd(" + src + ")"
		self.callback(self.jobNumber,self.src,self.dst)

def threadFinished(job_number,src,dst):
	#compute statistics

	global curActiveThreads, invalid_runs

	#print "generateStatsPerSimulation(" + src + ")"
	try:
		consumer_stats.generateStatsPerSimulation(src);
	except Exception:
		invalid_runs += 1
		pass

	#copy results
	files = glob.glob(src + "/traces/*STATS*.txt");

	if not os.path.exists(dst):
		os.makedirs(dst)

	for f in files:
		os.rename(f, dst+"/"+os.path.basename(f))

	#print "DELTE FOLDER " + src
	shutil.rmtree(src)

	curActiveThreads -= 1

def copyResults(src,dst):
	files = glob.glob(src + "/*.txt" );

	if not os.path.exists(dst):
		os.makedirs(dst)
	#else:
		#shutil.rmtree(dst)
		#os.makedirs(dst)
	
	for f in files:
		os.rename(f, dst+"/"+os.path.basename(f))		
		
###NOTE Start this script FROM itec-scenarios MAIN-FOLDER!!!

SIMULATION_DIR=os.getcwd()

THREADS = 1
SIMULATION_RUNS = 5
SIMULATION_OUTPUT = SIMULATION_DIR + "/output/"

results = {}

"""for root, dirs, files in os.walk(SIMULATION_OUTPUT):
	for subdir in dirs:
		
		if "output_run" in subdir:
			continue

		#print root+subdir

		files = glob.glob(root+subdir + "/*/*STATS*.txt" );
		
		avg_ratio = 0.0
		file_count = 0		

		for file in files:

			print file

			f = open(file, "r")
			for line in f:
				if(line.startswith("Ratio:")):
					avg_ratio += float(line[len("Ratio:"):])
					file_count +=1
					break;

		if(file_count > 0):
 			avg_ratio /= file_count
	
		print avg_ratio
		results.update({"AVG_RATIO:"+ subdir : avg_ratio})

print results
		
exit(0)"""

#build project before
call([SIMULATION_DIR + "/waf"])

#brite config file
scenario="brite_example"

briteConfig="--briteConfFile=/home/dposch/ndnSIM/itec-scenarios/brite_low_bw.conf"

bestRoute="--fw-strategy=bestRoute"
smartFlooding="--fw-strategy=smartflooding"
perContentBased="--fw-strategy=perContentBased"

allRoutes="--route=single"
singleRoute="--route=all"

# parameter search range
alpha_min = 0.05;
alpha_max = 0.2;
#alpha_max = 0.5;
alpha = []

while alpha_min <= alpha_max:
	alpha.append(alpha_min)
	alpha_min += 0.05

reliability_t_min = 0.90
#reliability_t_min = 0.75
reliability_t_max = 1.0
reliability_t = []

while reliability_t_min <= reliability_t_max:
	reliability_t.append(reliability_t_min)
	reliability_t_min += 0.05

probing_traffic_min = 0.0
probing_traffic_max = 0.2
#probing_traffic_max = 0.5
probing_traffic = []

while probing_traffic_min <= probing_traffic_max:
	probing_traffic.append(probing_traffic_min)
	probing_traffic_min += 0.1

update_intervall_min = 0.1
update_intervall_max = 0.5
#update_intervall_max = 1.0
update_intervall = []

while update_intervall_min <= update_intervall_max:
	update_intervall.append(update_intervall_min)
	update_intervall_min += 0.2

shift_traffic_min = 0.1
#shift_traffic_max = 0.5
shift_traffic_max = 0.1
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

print "Total different Settings = " + str(settings)
print "Runs per Setting = " + str(SIMULATION_RUNS) 
print "All in All Runs = " + str(settings*SIMULATION_RUNS) 

###script start
print "\nCurring working dir = " + SIMULATION_DIR + "\n"

job_number = 0

for scenarioName in SCENARIOS.keys():
	runs = SCENARIOS[scenarioName]['numRuns']
	executeable = SCENARIOS[scenarioName]['executeable']
	
	executeable = "build/" + executeable
	print "------------------------------------------------------------------------"
	print "Starting", runs , "simulations of", scenarioName
	
	for i in range(0, runs):
  	# See if we are using all available threads
		while curActiveThreads >= THREADS:
			time.sleep(1) # wait 1 second

		print "----------"
		print "Simulation run " + str(i) + " in progress..." 
		sysCall = [SIMULATION_DIR+"/" + executeable] +  SCENARIOS[scenarioName]['params'] + ["--RngRun=" + str(i)] + ["--outputFolder=traces"] ## working folder of subprocess is determined by Thread
		print sysCall
		#call(sysCall)

    # end while wait...

		dst = SIMULATION_OUTPUT+scenarioName + "/output_run"+str(i)
		src = SIMULATION_OUTPUT+"tmp_folder_" + str(job_number)

	   # start thread, get callback method to be called when thread is done
		thread = Thread(job_number, sysCall, threadFinished, src, dst)
		thread.start()

		job_number += 1
		curActiveThreads += 1	
# end for

while curActiveThreads != 0:
    time.sleep(100)
    print "Active Threads: " + str(curActiveThreads)
print ""

## parse results

print "We had " + str(invalid_runs) + " invalid runs"
print "Finished."