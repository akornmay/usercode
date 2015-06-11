#!/bin/bash
echo "###########################"
echo "# Running full simulation #"
echo "###########################"
if [ $# -lt 3 ]
  then
    echo "No arguments supplied"
    echo "Usage: /pixsim.sh [RunNumber] [Telescope] [DataFlow steer file] [optional:WBC] [optional:TOKEN_DELAY] [optional:TRIGGER_BUCKET]"
    exit
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
#    trackfile="../../../DataFlow_input/Try7_PixelTestBoard1_TrackCollection.root"
#    trackfile="../../../DataFlow_input/Try11_PixelTestBoard1_TrackCollection.root"
#    trackfile="../../../DataFlow_input/Try12_PixelTestBoard1_TrackCollection.root"
#    trackfile="../../../DataFlow_input/Try13_PixelTestBoard1_TrackCollection.root"
#    trackfile="../../../DataFlow_input/Try14_PixelTestBoard1_TrackCollection.root"
    trackfile="../../../DataFlow_input/Try16_PixelTestBoard1_TrackCollection.root"
elif [ $2 -eq 2 ]
then 
#    trackfile="../../../DataFlow_input/Try7_PixelTestBoard2_TrackCollection.root"
#    trackfile="../../../DataFlow_input/Try11_PixelTestBoard2_TrackCollection.root"
#    trackfile="../../../DataFlow_input/Try12_PixelTestBoard2_TrackCollection.root"
#    trackfile="../../../DataFlow_input/Try13_PixelTestBoard2_TrackCollection.root"
#    trackfile="../../../DataFlow_input/Try14_PixelTestBoard2_TrackCollection.root"
    trackfile="../../../DataFlow_input/Try16_PixelTestBoard2_TrackCollection.root"
else 
    echo INVALID telescope number!!!  
    exit
fi
#looking for file
if [ -f "$trackfile" ]
then
	echo "Track file found.($trackfile)"
else
	echo "Track file not found."
	exit
fi

#QIE file to run number

export qiefile="../../../QIEdata/RawData_spill$(printf %06d $1).bin.root"
#looking for file
if [ -f "$qiefile" ]
then
	echo "QIEfile found.($qiefile)"
else
	echo "QIEfile not found."
	exit
fi

echo "-------------------------"

echo "Config files:"
steerfile="$3"

if [ -f "$steerfile" ]
then
	echo "Steer file found.($steerfile)"
else
	echo "Steer file not found."
	exit
fi

echo
echo

#retrieving run parameters from spread sheet
echo "Retreiving original run parameters"
if [ $2 -eq 1 ]
then 
    origWBC=$(./readconfig_tel1.sh $1 WBC_0)
    echo "WBC: "$origWBC
    
    origTokLat=$(./readconfig_tel1.sh $1 trig_patt.token_latency)
    echo "Token latency: " $origTokLat
elif [ $2 -eq 2 ]
then 
    origWBC=$(./readconfig_tel2.sh $1 WBC_0)
    echo "WBC: "$origWBC
    
    origTokLat=$(./readconfig_tel2.sh $1 trig_patt.token_latency)
    echo "Token latency: " $origTokLat
else 
    echo INVALID telescope number!!!  
    exit
fi

origTriggerBucket=$(./readconfig_trigger.sh $1)
echo "Trigger bucket: " $origTriggerBucket


#here comes the DataFlow part

simtree=$trackfile


#modify the steering file to accept additional parameters for WBC and token delay
tempfile=$steerfile
tempfile2=$steerfile
tempfile3=$steerfile
if [ $# -gt 3 ]
then
    WBC="_WBC$(printf %03d $4).steer"
    #new file name
    tempfile=$(echo $tempfile | sed 's/\(.*\)\..*/\1/')$WBC
    echo $tempfile
    cp $3 $tempfile
    tempfile3=$tempfile
    echo "Setting WBC to $4" 
    sed -i "/^[^#]/ s%.*WBC.*%        WBC = $4 %" $tempfile

else 
    WBC="_WBC$(printf %03d $origWBC).steer"
    #new file name
    tempfile=$(echo $tempfile | sed 's/\(.*\)\..*/\1/')$WBC
    echo $tempfile
    cp $3 $tempfile
    tempfile3=$tempfile
    echo "Setting WBC to $origWBC" 
    sed -i "/^[^#]/ s%.*WBC.*%        WBC = $origWBC %" $tempfile

    TKDEL="_TKDEL$(printf %03d $origTokLat).steer"
    #new file name
    tempfile2=$(echo $tempfile | sed 's/\(.*\)\..*/\1/')$TKDEL
    echo $tempfile2
    mv $tempfile $tempfile2
    tempfile3=$tempfile2
    echo "Setting TOKEN_DELAY to $origTokLat"
    sed -i "/^[^#]/ s%.*TOKEN_DELAY.*%        TOKEN_DELAY = $origTokLat %" $tempfile2

    TRBUCK="_TRBUCK$(printf %03d $origTriggerBucket).steer"
    #new file name
    tempfile3=$(echo $tempfile2 | sed 's/\(.*\)\..*/\1/')$TRBUCK
    echo $tempfile3
    mv $tempfile2 $tempfile3    
    echo "Setting TRIGGER_BUCKET to $origTriggerBucket"
    sed -i "/^[^#]/ s%.*TRIGGER_BUCKET.*%        TRIGGER_BUCKET = $origTriggerBucket %" $tempfile3
fi

if [ $# -gt 4 ]
then
    TKDEL="_TKDEL$(printf %03d $5).steer"
    #new file name
    tempfile2=$(echo $tempfile | sed 's/\(.*\)\..*/\1/')$TKDEL
    echo $tempfile2
    mv $tempfile $tempfile2
    tempfile3=$tempfile2
    echo "Setting TOKEN_DELAY to $5"
    sed -i "/^[^#]/ s%.*TOKEN_DELAY.*%        TOKEN_DELAY = $5 %" $tempfile2
fi

if [ $# -gt 5 ]
then
    TRBUCK="_TRBUCK$(printf %03d $6).steer"
    #new file name
    tempfile3=$(echo $tempfile2 | sed 's/\(.*\)\..*/\1/')$TRBUCK
    echo $tempfile3
    mv $tempfile2 $tempfile3    
    echo "Setting TRIGGER_BUCKET to $6"
    sed -i "/^[^#]/ s%.*TRIGGER_BUCKET.*%        TRIGGER_BUCKET = $6 %" $tempfile3
fi

#we need to pick up the right QIE file

echo "Setting QIE path to $qiefile"
sed -i "/^[^#]/ s%.*QIE_FILENAME.*%        QIE_FILENAME = $qiefile %" $tempfile3




#readback WBC, TKDEL and TRBUCK from file
WBC=$(awk '!/^#/ && / WBC /{print $3}' $tempfile3)
echo "WBC from file:" $WBC
TKDEL=$(awk '!/^#/ && /TOKEN_DELAY/{print $3}' $tempfile3)
echo "Token delay from file:" $TKDEL
TRBUCK=$(awk '!/^#/ && /TRIGGER_BUCKET/{print $3}' $tempfile3)
echo "Trigger bucket from file:" $TRBUCK




outfile="../../../DataFlow_output/dataflowSUMMARY_PixelTestBoard$(printf %01d $2)_RUN$(printf %05d $1)_WBC$(printf %03d $WBC)_TKDEL$(printf %03d $TKDEL)_TRBUCK$(printf %03d $TRBUCK)_noPhase.root"
echo $outfile
treefile="../../../DataFlow_output/dataflowPIXTREE_PixelTestBoard$(printf %01d $2)_RUN$(printf %05d $1)_WBC$(printf %03d $WBC)_TKDEL$(printf %03d $TKDEL)_TRBUCK$(printf %03d $TRBUCK)_noPhase.root"
echo $treefile
transparenttreefile="../../../DataFlow_output/dataflowPIXTREE_PixelTestBoard$(printf %01d $2)_RUN$(printf %05d $1)_WBC$(printf %03d $WBC)_TKDEL$(printf %03d $TKDEL)_TRBUCK$(printf %03d $TRBUCK)_noPhase_TRANSPARENT.root"
echo $transparenttreefile



#modify the steering file so the right inputfile is picked up
sed -i "/^[^#]/ s%.*SIGNAL_FILENAME.*%        SIGNAL_FILENAME = $simtree %" $tempfile3
#modify the steering file so the right output summary file is put out
sed -i "/^[^#]/ s%.*OUTPUT_FILENAME.*%        OUTPUT_FILENAME = $outfile %" $tempfile3
#modify the steering file so the right pixel tree file is put out
sed -i "/^[^#]/ s%.*PIX_TREE_FILE.*%        PIX_TREE_FILE = $treefile %" $tempfile3


../DataFlow/DataFlow $tempfile3


#now the conversion to LCIO format
#source LCIO environment
echo "Sourcing LCIO environment"
source ../../../dataflow2lcio/env.sh


output="../../../DataFlow_LCIO/dataflowPIXTREE_PixelTestBoard$(printf %01d $2)_RUN$(printf %05d $1)_WBC$(printf %03d $WBC)_TKDEL$(printf %03d $TKDEL)_TRBUCK$(printf %03d $TRBUCK)_noPhase.slcio"
numberRocs=8

../../../dataflow2lcio/dataflow2lcio $treefile $output $numberRocs
q

transparentoutput="../../../DataFlow_LCIO/dataflowPIXTREE_PixelTestBoard$(printf %01d $2)_RUN$(printf %05d $1)_WBC$(printf %03d $WBC)_TKDEL$(printf %03d $TKDEL)_TRBUCK$(printf %03d $TRBUCK)_TRANSPARENT_noPhase.slcio"

../../../dataflow2lcio/dataflow2lcio $transparenttreefile $transparentoutput $numberRocs

