#!/bin/bash

####################################################
## script to read parameters from stefano's table ##
####################################################

if [ $# -lt 2 ]
then 
    echo "No arguments supplied"
    echo "Usage: /readconfig.sh [RunNumber] [Parameter name]" 
    exit
fi

COL=$(awk 'NR == 1 {for (i = 1; i <= NF; i++)  {if ($i ~ /'$2'/) {print i}}}' /afs/cern.ch/work/m/mersi/public/tables/pxltbGuiLogPixelTestBoard1.csv)

#echo $COL

awk '/'$1'/ {gsub(/,/,"");print $'"$COL"'}' /afs/cern.ch/work/m/mersi/public/tables/pxltbGuiLogPixelTestBoard1.csv
