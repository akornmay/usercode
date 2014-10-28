#!/bin/bash

####################################################
## This script will create the needed directories ##
## and check out the analysis code from gitHub    ##
####################################################

echo "####################################"
echo "# Setting up simuation environment #"
echo "####################################"
if [ $# -lt 1 ]
  then
    echo "No arguments supplied"
    echo "Usage: /createenv.sh [target dir]"
    exit
fi


cd $1
ls
#######
if [ ! -d "DataFlow_LCIO" ]
then
    echo "Creating LCIO file directory"
    mkdir DataFlow_LCIO
else
    echo "LCIO directory already exists"
fi

########
if [ ! -d "DataFlow_input" ]
then
    echo "Creating DataFlow input file directory"
    mkdir DataFlow_input
else
    echo "DataFlow input directory already exists"
fi

#########
if [ ! -d "DataFlow_output" ]
then
    echo "Creating DataFlow output file directory"
    mkdir DataFlow_output
else
    echo "DataFlow output directory already exists"
fi

##########
if [ ! -d "QIEdata" ]
then
    echo "Creating QIE file directory"
    mkdir QIEdata
else
    echo "QIE directory already exists"
fi

###########
if [ ! -d "cmspxltb-submission" ]
then
    echo "Checking out the cmspxltb submisson code"
    git clone https://github.com/simonspa/cmspxltb-submission.git
else
    echo "Cmspxltb submission already exists"
fi

############
if [ ! -d "cmspxltb-ana" ]
then
    echo "Checking out the cmspxltb analysis code"
    git clone https://github.com/grundler/cmspxltb-ana.git
else 
    echo "Cmspxltb-ana already exists"
fi

