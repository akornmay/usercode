#!/bin/sh
echo "###########################"
echo "# Running full simulation #"
echo "###########################"
if [ $# -ne 3 ]
  then
    echo "No arguments supplied"
    echo "Usage: /pixsim.sh [RunNumber] [Telescope] [DataFlow steer file]"
fi

echo RunNumber is $1
echo simulating Telescope $2 
#first we check if all files we need are there
echo Checking input files:
echo "-------------------------"
echo "Data files:"

#track file
if [ $2 -eq 1 ]
then 
    trackfile="/afs/cern.ch/work/a/akornmay/public/hugeTrackSample_cleanoutfull.dat"
elif [ $2 -eq 2 ]
then 
    trackfile="/afs/cern.ch/work/a/akornmay/public/hugeTrackSample_tilted_cleanoutfull.dat"
else 
    echo INVALID telescope number!!!  
fi
#looking for file
if [ -f "$trackfile" ]
then
	echo "Track file found.($trackfile)"
else
	echo "Track file not found."
fi

#QIE file to run number

qiefile="/afs/cern.ch/work/a/akornmay/public/Simulation/QIEdata/RawData_spill$1.bin.root"
#looking for file
if [ -f "$qiefile" ]
then
	echo "QIEfile found.($qiefile)"
else
	echo "QIEfile not found."
fi

echo "-------------------------"

echo "Config files:"
steerfile="$3"

if [ -f "$steerfile" ]
then
	echo "Steer file found.($steerfile)"
else
	echo "Steer file not found."
fi

echo
echo

#source ROOT environment
echo "Sourcing ROOT and compiling QIEsafehits macro" 
echo
source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.01/x86_64-slc5-gcc43-opt/root/bin/thisroot.sh
#recompiling macro
g++ `root-config --libs` ../geantTracks/QIEsavehits.c -I $ROOTSYS/include -o ../geantTracks/QIEsavehits
echo
#running program 
../geantTracks/QIEsavehits $1


#here comes the DataFlow part