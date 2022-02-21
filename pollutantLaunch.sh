#!/bin/bash
#~ declare -a files=("dormant" "dormant_inflow" "quiescent" "active" "active_inflow" "active_diffusion")

make
declare -a files=("active")
dir="config"
base=$dir"/pollute_base.config"
for i in "${files[@]}"
do
	file="pollute_"$i".config"
	loc=$dir"/"$file
	out=$dir"/pollutant_composite_"$i".config"
	cat $base $loc > $out
	./RAMICES_II -config $out
	#~ echo $i
	echo $out
	rm $out
done
