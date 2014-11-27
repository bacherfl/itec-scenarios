#! /usr/bin/python

import time
import numpy
import os
import glob
import sys
import collections

def process_app_delay_trace(wokring_dir, app_file_name):
	print("app-delays-trace(" + app_file_name + ")")

	f = open(wokring_dir + "/" + app_file_name,"r");

	stats = {}

	TIME_INDEX = 0
	NODE_INDEX = 1
	APP_ID_INDEX = 2 
	SEQ_NUM_INDEX = 3
	TYPE_INDEX = 4
	DELAY_S_INDEX = 5
	DELAY_US_INDEX = 6
	RETX_COUNT_INDEX = 7
	HOP_COUNT_INDEX = 8

	#create first lvl of dictionary and init second level with 0
	for line in f:
		l = line.split('\t')
		if(len(l) < HOP_COUNT_INDEX+1):
			continue

		if("FullDelay" in l[TYPE_INDEX]):
			if(l[NODE_INDEX] not in stats):
				stats[l[NODE_INDEX]] = {}
				stats[l[NODE_INDEX]].update({'DelayS': 0.0})
				stats[l[NODE_INDEX]].update({'RtxCount': 0})
				stats[l[NODE_INDEX]].update({'HopCount': 0})
	
	f.seek(0)
	for line in f:
			l = line.split('\t')
			if(len(l) < HOP_COUNT_INDEX+1):
				continue
			
			if("FullDelay" in l[TYPE_INDEX]):
				stats[l[NODE_INDEX]]['DelayS'] += float(l[DELAY_S_INDEX])
				stats[l[NODE_INDEX]]['RtxCount'] += float(l[RETX_COUNT_INDEX])
				stats[l[NODE_INDEX]]['HopCount'] += float(l[HOP_COUNT_INDEX])
		
	#print stats

	return stats


def process_aggregate_trace(wokring_dir, ag_file_name):
	print("process-aggregate-trace(" + ag_file_name + ")")

	f = open(wokring_dir + "/" + ag_file_name,"r");

	stats = {}

	TIME_INDEX = 0
	NODE_INDEX = 1
	FACE_INDEX = 2 
	FACE_DESCRIPTION_INDEX = 3
	TYPE_INDEX = 4
	PACKET_NR_INDEX = 5

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

	#print stats

	#fill second levle with data
	f.seek(0)
	for line in f:
		l = line.split('\t')		
		if(len(l) < PACKET_NR_INDEX+1):
			continue

		if("dev=local" in l[FACE_DESCRIPTION_INDEX]):
			stats[l[NODE_INDEX]][l[TYPE_INDEX]] += int(l[PACKET_NR_INDEX])
	
	#print stats

	return stats

#main

AG_TRACE_FILE = "aggregate-trace.txt"
APP_TRACE_FILE = "app-delays-trace.txt"

working_dir = os.getcwd()
print ("Curring working dir = " + working_dir + "\n")

ag_stats = process_aggregate_trace(working_dir, AG_TRACE_FILE)
app_stats = process_app_delay_trace(working_dir, APP_TRACE_FILE)

total_number_of_requests = 0.0
total_number_of_statisfied_requests = 0.0
avg_number_of_hops = 0.0
avg_number_of_rtx = 0.0
avg_delay_of_request = 0.0

for key in app_stats:
	avg_delay_of_request += app_stats[key]['DelayS']
	avg_number_of_hops += app_stats[key]['HopCount']
	avg_number_of_rtx += app_stats[key]['RtxCount']

for key in ag_stats:
	total_number_of_requests += ag_stats[key]['InInterests']
	total_number_of_statisfied_requests += ag_stats[key]['OutData']

avg_number_of_hops /= total_number_of_statisfied_requests
avg_number_of_rtx /= total_number_of_statisfied_requests
avg_delay_of_request /= total_number_of_statisfied_requests

print "Total Requests: " + str(total_number_of_requests)
print "Statisfied Requests: " + str(total_number_of_statisfied_requests) + " Ratio:" + str(total_number_of_statisfied_requests/total_number_of_requests)
print "Avg Delay[s]: " + str(avg_delay_of_request) + "[sec]"
print "Avg Hops: " + str(avg_number_of_hops)
print "Avg Rtx : " + str(avg_number_of_rtx - 1)
