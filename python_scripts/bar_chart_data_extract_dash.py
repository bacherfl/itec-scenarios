import os
import numpy as np
import scipy as sp
import scipy.stats
import glob
import re

def walklevel(some_dir, level=1):
    some_dir = some_dir.rstrip(os.path.sep)
    assert os.path.isdir(some_dir)
    num_sep = some_dir.count(os.path.sep)
    for root, dirs, files in os.walk(some_dir):
        yield root, dirs, files
        num_sep_this = root.count(os.path.sep)
        if num_sep + level <= num_sep_this:
            del dirs[:]

def getSubdirsForConnectivity(filter):

	simulations = []

	for root, dirs, files in walklevel(DATA_DIR, 0):
			for simulation in dirs:

				if(filter not in simulation):
					continue

				simulations.append(simulation)

	return simulations

def groupByBW(simulations, filter):

	group = []

	for sim in simulations:
				if(filter not in sim):
					continue

				group.append(sim)

	return group

def mean_confidence_interval(data, confidence=0.95):
    a = 1.0*np.array(data)
    n = len(a)
    m, se = np.mean(a), scipy.stats.sem(a)
    h = se * sp.stats.t._ppf((1+confidence)/2., n-1)
    return m, m-h, m+h

def readAttribute(filename, attrib):

	f = open(filename,"r")
	
	for line in f:
		if(line.startswith(attrib)):
			#print line
			return float(re.findall(r"[-+]?\d*\.\d+|\d+", line)[0])

	print "Could not find line: " + attrib + " in file " + filename
	return "NaN"

# note link failures a not considered yet they are just grouped.
def calculateQuartiles(simulations, alg_filter, mode):

	sims = []

	#extract runs only for algorithm
	for sim in simulations:
		if(alg_filter not in sim):
					continue

		#print sim
		sims.append(sim)

	values = []
	# take only runs of with certain link failures
	for sim in sims:

		if(LINK_FAILURE_FILTER not in sim):
				continue

		for root, dirs, files in walklevel(DATA_DIR + sim, 1):
			for subdir in dirs:
				files = glob.glob(root+"/"+subdir + "/*STATS*.txt" );
				for f in files:
					#print f
					if("stalls" in mode):
						values.append(readAttribute(f, "AVG Unsmooth per Client:"))
					elif("quality" in mode):
						values.append(readAttribute(f, "AVG Level per Client:"))
					else:
						print "Error Attribute " + mode + " unknown!"

	#print values
	return mean_confidence_interval(values, 0.95)

######################################################

SIMULATION_DIR=os.getcwd()
DATA_DIR=SIMULATION_DIR+"/output/"

CONNECTIVITY_FILTERS = ["LowConnectivity", "MediumConnectivity", "HighConnectivity" ]
BANDWIDTH_FILTERS= ["LowBW", "MediumBW", "HighBW"]
ALGORITHM_FILTERS= ["BestRoute", "SmartFlooding", "PerContentBased", "Broadcast"]
#LINK_FAILURE_FILTER = "LinkFailure_0"
LINK_FAILURE_FILTER = "LinkFailure_15"


#simulations = getSubdirsForConnectivity(FILTER)
#sims_lowBW = groupByBW(simulations, BANDWIDTH_FILTERS[0])
#sims_mediumBW = groupByBW(simulations, BANDWIDTH_FILTERS[1])
#sims_highBW = groupByBW(simulations, BANDWIDTH_FILTERS[2])

for connectivity in CONNECTIVITY_FILTERS: 

	f = open(DATA_DIR + connectivity + "_stalls.dat", "w")
	lines = ""
	
	simulations = getSubdirsForConnectivity(connectivity)
	
	for bw in BANDWIDTH_FILTERS:
		header = "descriptor bandwidth "
		line = bw + " "
		sims_bw = groupByBW(simulations, bw)

		for algorithm in ALGORITHM_FILTERS:
			header += algorithm + "_stalls,+,- "
			res = calculateQuartiles(sims_bw, algorithm, "stalls")
			line+= "%.4f" % res[0] + " %.4f" % (res[2]-res[0]) + " %.4f" % (res[1]-res[0]) + "\t"
		
		lines += line +"\n"

	f.write(header+"\n")
	f.write(lines)
	f.close()

for connectivity in CONNECTIVITY_FILTERS: 

	f = open(DATA_DIR + connectivity + "_quality.dat", "w")
	lines = ""
	
	simulations = getSubdirsForConnectivity(connectivity)
	
	for bw in BANDWIDTH_FILTERS:
		header = "descriptor bandwidth "
		line = bw + " "
		sims_bw = groupByBW(simulations, bw)

		for algorithm in ALGORITHM_FILTERS:
			header += algorithm + "_quality,+,- "
			res = calculateQuartiles(sims_bw, algorithm, "quality")
			line+= "%.4f" % res[0] + " %.4f" % (res[2]-res[0]) + " %.4f" % (res[1]-res[0]) + "\t"
		
		lines += line +"\n"

	f.write(header+"\n")
	f.write(lines)
	f.close()

