#!/bin/bash

echo $1
folders=$(ls $1)



for f in $folders
do
	tail -n +2 $1/$f/STATS.txt
done

#tail -n +2 $1/STATS.txt

