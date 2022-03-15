#!/bin/bash

dir="config/PollutantTests/"
base="base.config"
temp="temp.config"
declare -a arr=("a0" "a1" "a2" "q0" "q1" "q2")
#~ declare -a arr=("a0")
## now loop through the above array
for i in "${arr[@]}"
do
   cat $dir$base $dir$i".config" > $dir$temp
   # or do whatever with individual element of the array
   ./Ramices_Launch.sh -config $dir$temp
   rm $dir$temp
done
