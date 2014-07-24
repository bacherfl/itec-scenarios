#! /usr/bin/python

import time
import numpy
import os
import glob
import sys
import collections



        

###functions####################################################################
def extract(f, string):
	result = "not found"
	file = open(f, "r")
	for line in file:
			if(line.startswith(string)):
				result = line[len(string):]
				break
	file.close()
	return result
	
	
def getQualitySwitches(f):
	return int(extract(f, "# Quality Switches = "))

def getAVGLevel(f):
	return float(extract(f, "# AVG Level = "))

def getAVGUnsmoothSeconds(f):
	return float(extract(f, "# Unsmooth Seconds = "))

def getVideoName(f):
	return extract(f, "# Video Downloaded = ")

def getAVGBuffer(f):
	return float(extract(f, "# AVG Buffer size (seconds) = "))

def getAVGGoodput(f):
	return float(extract(f, "# AVG Goodput (kbit/s) = "))

def groupByVideo(files):
	groupedLists = { }

	for f in files:
		v_name = getVideoName(f);
		if(v_name in groupedLists):
			groupedLists[v_name].append(f)
		else:
			groupedLists[v_name] = [f]

	groupedLists = collections.OrderedDict(sorted(groupedLists.items()))

	return groupedLists

###programm#####################################################################

def computeStats(curdir):

	#print "Curring working dir = " + curdir + "\n"

	files = glob.glob(curdir + "/Content*.txt" );
	output_file = open(curdir+"/STATS.txt", "w")

	###statistic over all clients

	#output_file.write("Statistic over all Clients:\n")

	avg_level_per_client = 0.0
	avg_unsmooth_seconds_per_client = 0.0
	max_unsmooth_seconds = 0.0
	avg_buffer_per_client = 0.0
	avg_goodput_per_client = 0.0
	avg_quality_switches_per_client = 0.0

	for f in  files:
		#print f
		avg_level_per_client += getAVGLevel(f)
		avg_quality_switches_per_client += getQualitySwitches(f)
		
		unsmooth_sec = getAVGUnsmoothSeconds(f)
		
		avg_unsmooth_seconds_per_client += unsmooth_sec
		if unsmooth_sec > max_unsmooth_seconds:
			max_unsmooth_seconds = unsmooth_sec
			
		avg_buffer_per_client += getAVGBuffer(f)
		avg_goodput_per_client += getAVGGoodput(f)

	avg_level_per_client /= len(files)
	avg_unsmooth_seconds_per_client /= len(files) 
	avg_buffer_per_client /= len(files)
	avg_goodput_per_client /= len(files)
	avg_quality_switches_per_client /= len(files)
	
	output_file.write("VideoName,NumConcurStreamers,AVGLevel,AVGQualitySwitches,AVGUnsmooth,MAXUnsmooth,AVGBuffer,AVGGoodput\n")
	#output_file.write("100," + str(avg_level_per_client) + "," +  str(avg_quality_switches_per_client) + "," + str(avg_unsmooth_seconds_per_client) 
	#+ "," + str(max_unsmooth_seconds) + "," + str(avg_buffer_per_client) + "," + str(avg_goodput_per_client) + "\n")


	###statistics per downloaded video

	groupedVideos = groupByVideo(files)

	for key in groupedVideos:
		#output_file.write("Video: " + key[:len(key)-1] + " has been downloaded " + str(len(groupedVideos[key])) + " times:\n") 

		avg_level = 0.0 
		avg_unsmooth_seconds= 0.0
		avg_buffer = 0.0
		avg_goodput = 0.0
		max_unsmooth_seconds = 0.0
		avg_quality_switches_per_client = 0.0
		min_level = 99999
		cur_level = 0.0

		for f in groupedVideos[key]:
			cur_level = getAVGLevel(f)
			avg_level += cur_level
			avg_quality_switches_per_client += getQualitySwitches(f)
			unsmooth_sec = getAVGUnsmoothSeconds(f)
			avg_unsmooth_seconds += unsmooth_sec
			if unsmooth_sec > max_unsmooth_seconds:
				max_unsmooth_seconds = unsmooth_sec
			if cur_level < min_level:
				min_level = cur_level
			avg_buffer += getAVGBuffer(f)
			avg_goodput += getAVGGoodput(f)

		avg_level /= len(groupedVideos[key])
		avg_unsmooth_seconds /= len(groupedVideos[key]) 
		avg_buffer /= len(groupedVideos[key])
		avg_goodput /= len(groupedVideos[key])
		avg_quality_switches_per_client /= len(groupedVideos[key])
		
		output_file.write(key[:len(key)-1] + "," + str(len(groupedVideos[key])) + "," + str(avg_level) + "," +  str(avg_quality_switches_per_client) + "," + str(avg_unsmooth_seconds) 
			+ "," + str(max_unsmooth_seconds) + "," + str(avg_buffer) + "," + str(avg_goodput) + "\n")

	
if sys.argv[0] == 'calculate_stats.py':
	if sys.argv.__len__() >= 2:
		for arg in sys.argv:
			if 'calculate_stats.py' not in arg:
				print "Calculating Average via commandline for folder " + arg + ", writeing results to " + arg + "/STATS.txt:"
				computeStats(arg)		
	else:
		print "Not enough parameters ..."		
else:
	print "Loading calculate_stats library"		
# end for

