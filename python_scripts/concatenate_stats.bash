#!/bin/bash

#echo $1
folders=$(ls $1)

echo "VideoName,NumConcurStreamers,AVGLevel,AVGQualitySwitches,AVGUnsmooth,MAXUnsmooth,AVGBuffer,AVGGoodput"

for f in $folders
do
	tail -n +2 $1/$f/STATS.csv
done

#tail -n +2 $1/STATS.txt

