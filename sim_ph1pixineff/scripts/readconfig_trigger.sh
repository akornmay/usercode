#!/bin/bash

####################################################
## script to read parameters from stefano's table ##
####################################################

if [ $# -lt 1 ]
then 
    echo "No arguments supplied"
    echo "Usage: /readconfig.sh [RunNumber]" 
    exit
fi


awk '/'$1'/ {gsub(/,/,"");print strtonum("0x"$4)}' /afs/cern.ch/work/m/mersi/public/tables/triggerData.csv
