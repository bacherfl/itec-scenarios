#! /usr/bin/python

import time
import numpy
import os
import glob
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

def getAVGLevel(f):
	return float(extract(f, "AVG Level = "))

def getAVGUnsmoothSeconds(f):
	return float(extract(f, "Unsmooth Seconds = "))

def getVideoName(f):
	return extract(f, "Video Downloaded = ")

def getAVGBuffer(f):
	return float(extract(f, "AVG Buffer size (seconds) = "))

def getAVGGoodput(f):
	return float(extract(f, "AVG Goodput (kbit/s) = "))

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

curdir = os.getcwd()
print "Curring working dir = " + curdir + "\n"
files = glob.glob(curdir + "/../output/*.txt" );


###statistic over all clients

print "Statistic over all Clients:"

avg_level_per_client = 0.0
avg_unsmooth_seconds_per_client = 0.0
avg_buffer_per_client = 0.0
avg_goodput_per_client = 0.0

for f in  files:
	avg_level_per_client += getAVGLevel(f)
	avg_unsmooth_seconds_per_client += getAVGUnsmoothSeconds(f)
	avg_buffer_per_client += getAVGBuffer(f)
	avg_goodput_per_client += getAVGGoodput(f)

avg_level_per_client /= len(files)
avg_unsmooth_seconds_per_client /= len(files) 
avg_buffer_per_client /= len(files)
avg_goodput_per_client /= len(files)

print "AVG Level per Client: " + str(avg_level_per_client)
print "AVG Unsmooth per Client: " + str(avg_unsmooth_seconds_per_client) + "s"
print "AVG Buffer per Client: " + str(avg_buffer_per_client) + "s"
print "AVG Goodput per Client: " + str(avg_goodput_per_client) + "kbit/s"
print

###statistics per downloaded video

groupedVideos = groupByVideo(files)

for key in groupedVideos:
	print "Video: " + key[:len(key)-1] + " has been downloaded " + str(len(groupedVideos[key])) + " times:" 

	avg_level = 0.0
	avg_unsmooth_seconds= 0.0
	avg_buffer = 0.0
	avg_goodput = 0.0

	for f in groupedVideos[key]:
		avg_level += getAVGLevel(f)
		avg_unsmooth_seconds += getAVGUnsmoothSeconds(f)
		avg_buffer += getAVGBuffer(f)
		avg_goodput += getAVGGoodput(f)

	avg_level /= len(groupedVideos[key])
	avg_unsmooth_seconds /= len(groupedVideos[key]) 
	avg_buffer /= len(groupedVideos[key])
	avg_goodput /= len(groupedVideos[key])

	print "AVG Level: " + str(avg_level)
	print "AVG Unsmooth: " + str(avg_unsmooth_seconds) + "s"
	print "AVG Buffer: " + str(avg_buffer) + "s"
	print "AVG Goodput: " + str(avg_goodput) + "kbit/s"
	print

print str(len(files)) + " files processed."
print "Done!"
