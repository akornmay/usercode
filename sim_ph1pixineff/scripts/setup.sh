#!/bin/bash

. /afs/cern.ch/sw/lcg/external/gcc/4.7.2p1/x86_64-slc6/setup.sh
. /afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.21/x86_64-slc6-gcc47-opt/root/bin/thisroot.sh

#source igprof
INSTAREA=/home/akornmay/igprof
export PATH=$INSTAREA/bin:$PATH
export LD_LIBRARY_PATH=$INSTAREA/lib:$LD_LIBRARY_PATH