#!/bin/bash
make clean
make -f Testing/makefile-profiler
./RAMICES_II_Profile "$@"
gprof RAMICES_II_Profile gmon.out > Testing/profile.txt
rm gmon.out
echo "A profile file has been written to Testing/profile.txt"
geany Testing/profile.txt

