/* 
########################################################################## 
    decoder for QIE data format
########################################################################## 

   - Author : Yeng-Ming (Jacky) Tzeng
   - e-mail : ymtzeng@cern.ch
   - create time : 03/04/2014
   - version : 2.2

   * v2.2 on 03/17/14 : add pedestal with baseline offset
   * v2.1 on 03/11/14 : Kaz updated DAQ readout to avoid byte-swap, and the decoder should be turned off affter 168126

   Usage :
        1). source root environment
        2). g++ -o dd decoder_databinary.cc `root-config --cflags` `root-config --glibs`
        2). g++ -o db debug.cc `root-config --cflags` `root-config --glibs`
        3). ./dd file.bin
                for example : ./db 166179 /data2/experiments/T1036/data/cerenkov_data/raw_QIE_data
                for example : ./db 168126 /data2/experiments/T1036/data/cerenkov_data/raw_QIE_data
                for example : ./db 168597 /data2/experiments/T1036/data/cerenkov_data/raw_QIE_data
                for example : ./db 169558 /data2/experiments/T1036/data/cerenkov_data/raw_QIE_data
                tree_trigger->Scan("18.9*(588*trigger_turn_onset+trigger_RF_onset)/pow(10,9)")
        4). output file as file.root

   === data structure ====
   [4 byte - QIE Data size, # words]
   [2 byte empty]
   297 word cycles :    
   [16 bit - MI turn #,Hi]
   [16 bit - MI turn #,Lo]
   [16 bit - cap ID, debug]

   [ 294 x 16 bit, beam intensity / RF
   each 16 bit = 2x 8-bit beam intensity 
   = 588 RF buckets ]  
   [4 byte - trigger block]
   5 word cycles :
   [16 bit - trigger#, Hi]
   [16 bit - trigger#, Lo]
   [16 bit - MI turn #, Hi]
   [16 bit - MI turn #, Lo]
   [16 bit - RF bucket #, 0-587]



   ---- format for FNAL2014 

   <Trigger block size (int)>
   <Trigger 1 word number (int)>
   <Trigger 1 counter (int)>
   <Trigger 1 turn onset (int)>
   <Trigger 1 RF onset (int)>
   <Trigger 1 turn release (int)>
   <Trigger 1 RF release (int)>
   <Trigger 1 intensity sum no inhibit (int)>
   <33 integers for trigger 1.  This has the intensity for the RF bucket where the trigger came as well as +/-16 RF buckets around it, for a total of 33 intensity values>
   <Trigger 2 word number (int)>
   ...repeat...

   <Inhibit block size (int)>
   <Inhibit1 turn onset (int)>
   <Inhibit1 RF onset (int)>
   <Inhibit1 intensity sum (int)>
   <Inhibit1 turn release (int)>
   <Inhibit1 RF release (int)>
   <Inhibit2 turn onset (int)>
   ...repeat...

   From here and below are all QIE data until the end of file.
   <port1Size > : QIE block size is 3 x port1size
   <QIE turn count HI 16 bits (short)>
   <QIE turn count LO 16 bits (short)>
   <QIE cap ID (short)>
   <294 words of unsigned short.  Each word holds 2 RF bucket worth of intensity data.  294 words = 588 RF buckets = 1 whole turn>
 */

#include <iostream>
#include <iomanip>
#include <string>
#include "TTree.h"
#include "TFile.h"
#include <fstream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

bool DebugMode = false;
FILE *dataStream;
//bool read_16bits(unsigned short &word);
bool read_16bits(unsigned short &word, bool swap);
bool read_32bits(unsigned int &word32, bool swap);
bool LoadCyclicTriggerMap();

float QIE_conversion[256];
std::map<int, int> cyclic_trigger_map;
std::map<int, int>::iterator cyclic_trigger_mapItr_;
int main(int argc, char *argv[]){

    bool IsLoadCyclicTriggerMap = LoadCyclicTriggerMap();
    char buffer[128];
    if(argv[1] == NULL || argv[2] == NULL){
        printf("Usage :\n1). source root environment\n2). g++ -o dd decoder_databinary.cc `root-config --cflags` `root-config --glibs`\n3). ./dd spill-number path\n4). output file as RawData_spillNumber.bin.root\n");
        return 0;
    }   

    ifstream inlookup;
    inlookup.open("QIE_conversion_map.dat");
    int counter=0;
    while(inlookup >> counter){
        inlookup >> QIE_conversion[counter];
    }
    inlookup.close();

    sprintf(buffer,"%s/RawData_spill%s.bin",argv[2], argv[1]);
    std::string inputfilename = argv[1];
    //dataStream = fopen(inputfilename.c_str(),"r");
    dataStream = fopen(buffer,"r");

    bool status = true;
    unsigned int word32;
    unsigned short word;
    unsigned short wordbuffer;
    counter=0;  // unit : 16 bit
    long long int port1Size = 0;
    long long int QIEDataSize = 0;
    int N_MIturn_Hi = 0;
    int N_MIturn_Lo = 0;
    int N_MIturn    = 0;
    int capID = 0;
    int RFbuckets[2] = {0,0};
    float BeamIntensity[588];
    for(int i=0;i<588;i++) BeamIntensity[i]=0;
    float BeamIntensityRaw[588];
    for(int i=0;i<588;i++) BeamIntensityRaw[i]=0;
    float ADC[588];
    for(int i=0;i<588;i++) ADC[i]=0;
    float AccumulatedBeamIntensity = 0;

    long long int trigger_block_dummy = 0;
    long long int trigger_block = 0;
    int trigger_word_number = 0;
    int trigger_counter = 0;
    int trigger_turn_onset = 0;
    int trigger_RF_onset = 0;
    int trigger_turn_release = 0;
    int trigger_RF_release = 0;
    int trigger_intensity_sum_noinhibit = 0;
    int trigger_RFbucket = 0;

    long long int inhibit_block = 0;
    int inhibit_counter = 0;
    int inhibit_turn_onset = 0;
    int inhibit_RF_onset = 0;
    int inhibit_turn_release = 0;
    int inhibit_RF_release = 0;
    int inhibit_intensity_sum = 0;
    //sprintf(buffer,"%s.root",inputfilename.c_str());
    sprintf(buffer,"%s/RawData_spill%s.bin.root",argv[2],argv[1]);
    TFile *tfile = new TFile(buffer,"recreate");

    TTree *tree_QIE = new TTree("tree_QIE","tree_QIE");
    tree_QIE->Branch("port1Size",&port1Size,"port1Size/L");
    tree_QIE->Branch("QIEDataSize",&QIEDataSize,"QIEDataSize/L");
    tree_QIE->Branch("N_MIturn_Hi",&N_MIturn_Hi,"N_MIturn_Hi/I");
    tree_QIE->Branch("N_MIturn_Lo",&N_MIturn_Lo,"N_MIturn_Lo/I");
    tree_QIE->Branch("N_MIturn",&N_MIturn,"N_MIturn/I");
    tree_QIE->Branch("capID",&capID,"capID/I");
    tree_QIE->Branch("BeamIntensity",&BeamIntensity[0],"BeamIntensity[588]/F");
    tree_QIE->Branch("BeamIntensityRaw",&BeamIntensityRaw[0],"BeamIntensityRaw[588]/F");
    tree_QIE->Branch("ADC",&ADC[0],"ADC[588]/F");
    tree_QIE->Branch("AccumulatedBeamIntensity",&AccumulatedBeamIntensity,"AccumulatedBeamIntensity/F");

    TTree *tree_trigger = new TTree("tree_trigger","tree_trigger");
    tree_trigger->Branch("trigger_block",&trigger_block,"trigger_block/L");
    tree_trigger->Branch("trigger_word_number",&trigger_word_number,"trigger_word_number/I");
    tree_trigger->Branch("trigger_counter",&trigger_counter,"trigger_counter/I");
    tree_trigger->Branch("trigger_turn_onset",&trigger_turn_onset,"trigger_turn_onset/I");
    tree_trigger->Branch("trigger_RF_onset",&trigger_RF_onset,"trigger_RF_onset/I");
    tree_trigger->Branch("trigger_turn_release",&trigger_turn_release,"trigger_turn_release/I");
    tree_trigger->Branch("trigger_RF_release",&trigger_RF_release,"trigger_RF_release/I");
    tree_trigger->Branch("trigger_intensity_sum_noinhibit",&trigger_intensity_sum_noinhibit,"trigger_intensity_sum_noinhibit/I");
    tree_trigger->Branch("trigger_RFbucket",&trigger_RFbucket,"trigger_RFbucket/I");

    TTree *tree_inhibit = new TTree("tree_inhibit","tree_inhibit");
    tree_inhibit->Branch("inhibit_block",&inhibit_block,"inhibit_block/L");
    tree_inhibit->Branch("inhibit_counter",&inhibit_counter,"inhibit_counter/I");
    tree_inhibit->Branch("inhibit_turn_onset",&inhibit_turn_onset,"inhibit_turn_onset/I");
    tree_inhibit->Branch("inhibit_RF_onset",&inhibit_RF_onset,"inhibit_RF_onset/I");
    tree_inhibit->Branch("inhibit_turn_release",&inhibit_turn_release,"inhibit_turn_release/I");
    tree_inhibit->Branch("inhibit_RF_release",&inhibit_RF_release,"inhibit_RF_release/I");
    tree_inhibit->Branch("inhibit_intensity_sum",&inhibit_intensity_sum,"inhibit_intensity_sum/I");


    TTree *tree_summary = new TTree("tree_summary","tree_summary");
    int summary_spill = 0;
    int summary_Trigger_size = 0;
    int summary_Trigger_count = 0;
    int summary_Trigger_RF_onset = 0;
    int summary_Trigger_turn_onset = 0;
    int summary_Cyclic_RF_onset = 0;
    int summary_Cyclic_RF_onset_status = 0;
    int summary_Trigger_BeamIntensity[5];       // [1 , 66, 132, 198, 264]
    double summary_onset_time = 0;
    for(int i=0;i<5;i++) summary_Trigger_BeamIntensity[i] = 0;
    tree_summary->Branch("summary_spill",&summary_spill,"summary_spill/I");
    tree_summary->Branch("summary_Trigger_size",&summary_Trigger_size,"summary_Trigger_size/I");
    tree_summary->Branch("summary_Trigger_count",&summary_Trigger_count,"summary_Trigger_count/I");
    tree_summary->Branch("summary_onset_time",&summary_onset_time,"summary_onset_time/D");
    tree_summary->Branch("summary_Trigger_RF_onset",&summary_Trigger_RF_onset,"summary_Trigger_RF_onset/I");
    tree_summary->Branch("summary_Trigger_turn_onset",&summary_Trigger_turn_onset,"summary_Trigger_turn_onset/I");
    tree_summary->Branch("summary_Cyclic_RF_onset",&summary_Cyclic_RF_onset,"summary_Cyclic_RF_onset/I");
    tree_summary->Branch("summary_Cyclic_RF_onset_status",&summary_Cyclic_RF_onset_status,"summary_Cyclic_RF_onset_status/I");
    tree_summary->Branch("summary_Trigger_BeamIntensity",&summary_Trigger_BeamIntensity[0],"summary_Trigger_BeamIntensity[5]/I");

    int checkValue = -1;
    int checkCounter = 0;

    int sizeofBeamIntensity = 0;

    status = read_32bits(word32,false); // trigger block size
    trigger_block_dummy = (word32)& 0xffffffff;
    if(!status) {
            tfile->Close();
            return 0;
    }
    bool IsFoundFirst = false;
    int counter_for_trigger_block = 0;
    for(int i=0;i<8*(int)trigger_block_dummy;i++){
            status = read_32bits(word32,false); 
            if(!status) {
                    tfile->Close();
                    return 0;
            }
            if(i%8==0) trigger_word_number             = (word32)& 0xffffffff;
            if(i%8==1) trigger_counter                 = (word32)& 0xffffffff;
            if(i%8==2) trigger_turn_onset              = (word32)& 0xffffffff;
            if(i%8==3) trigger_RF_onset                = (word32)& 0xffffffff;
            if(i%8==4) trigger_turn_release             = (word32)& 0xffffffff;
            if(i%8==5) trigger_RF_release               = (word32)& 0xffffffff;
            if(i%8==6) trigger_intensity_sum_noinhibit = (word32)& 0xffffffff;
            if(i%8==7) trigger_RFbucket                = (word32)& 0xffffffff;
            if(i%8==7){
                    trigger_block++;
                    tree_trigger->Fill();
                    trigger_word_number = 0;
                    trigger_counter = 0;
                    trigger_turn_onset = 0;
                    trigger_RF_onset = 0;
                    trigger_turn_release = 0;
                    trigger_RF_release = 0;
                    trigger_intensity_sum_noinhibit = 0;
                    trigger_RFbucket = 0;
                    IsFoundFirst = false;
            }
    }
    std::cout<<"[tree_trigger DEBUG] trigger_block :"<<trigger_block<<std::endl;
    status = read_32bits(word32,false); // inhibit block size
    inhibit_block = (word32)& 0xffffffff;
    if(!status) {
            tfile->Close();
            return 0;
    }
    for(int i=0;i<6*inhibit_block;i++){
            bool IsSwapped = true;
            if(i%6==2 || i%6==4) IsSwapped = false;
            status = read_32bits(word32,IsSwapped); 
            if(!status) {
                    tfile->Close();
                    return 0;
            }
            if(i%6==0) inhibit_counter                 = (word32)& 0xffffffff;
            if(i%6==1) inhibit_turn_onset              = (word32)& 0xffffffff;
            if(i%6==2) inhibit_RF_onset                = (word32)& 0xffffffff;
            if(i%6==3) inhibit_intensity_sum           = (word32)& 0xffffffff;
            if(i%6==4) inhibit_turn_release             = (word32)& 0xffffffff;
            if(i%6==5) inhibit_RF_release               = (word32)& 0xffffffff;
            if(i%6==5){
                    tree_inhibit->Fill();
                    inhibit_counter                 = 0;
                    inhibit_turn_onset              = 0;
                    inhibit_RF_onset                = 0;
                    inhibit_turn_release             = 0;
                    inhibit_RF_release               = 0;
                    inhibit_intensity_sum           = 0;
            }
    }
    status = read_32bits(word32,false); // port1 size
    port1Size = (word32)& 0xffffffff;
    while (status){
            if((counter/297)%10000==0&&counter%297==0) printf("Decoding QIE data for %i th event\r",counter/297);
            bool IsSwapped = false;
            if((counter)%297<2) {
                    IsSwapped = true;
            }
            status = read_16bits(word,IsSwapped);
            //std::cout<<std::hex<<std::setw(4)<<std::setfill('0')<<word<< std::dec<<std::endl;
            if(counter==0){
                    sizeofBeamIntensity = 0;
                    AccumulatedBeamIntensity = 0;
            }
            if((counter)>=0){
                    if((counter)%297==0) {
                            N_MIturn_Hi = (word & 0xffff);
                            if(DebugMode)
                                    std::cout<<"N_MIturn_Hi : "<<N_MIturn_Hi<<std::endl;
                            if(DebugMode)
                            std::cout<<std::endl<<"["<<(counter)/297<<"th QIE ] N_MIturn_Hi : "<< std::hex<<std::setw(4)<<std::setfill('0')<<word<< std::dec<<std::endl;
                            wordbuffer = word;
                    }
                    if((counter)%297==1) {
                            N_MIturn_Lo = (word & 0xffff);
                            if(DebugMode)
                                    std::cout<<"N_MIturn_Lo : "<<N_MIturn_Lo<<std::endl;
                            if(DebugMode)
                            std::cout<<"["<<(counter)/297<<"th QIE ] N_MIturn_Lo : "<< std::hex<<std::setw(4)<<std::setfill('0')<<word<< std::dec<<std::endl;
                            word32 = (wordbuffer<<16)|word;
                            N_MIturn = ( word32 ) & 0xffffffff;                 
                            if(DebugMode)
                            std::cout<<"    ["<<(counter)/297<<"th QIE ] N_MIturn : "<< std::hex<<std::setw(8)<<std::setfill('0')<<word32<< std::dec<<std::endl;
                    }
                    if((counter)%297==2) {
                            capID = (word & 0xffff);
                            if(DebugMode)
                                    std::cout<<"capID : "<<capID<<std::endl;
                    }
                    if((counter)%297>=3&&(counter)%297<297){
                            BeamIntensity[sizeofBeamIntensity] = QIE_conversion[word & 0x00ff];
                            ADC[sizeofBeamIntensity] = word & 0x00ff;
                            AccumulatedBeamIntensity+= BeamIntensity[sizeofBeamIntensity];
                            sizeofBeamIntensity++;

                            BeamIntensity[sizeofBeamIntensity] = QIE_conversion[word>>8 & 0x00ff];
                            ADC[sizeofBeamIntensity] = word>>8 & 0x00ff;
                            AccumulatedBeamIntensity+= BeamIntensity[sizeofBeamIntensity];
                            sizeofBeamIntensity++;
                            if(DebugMode)
                                    std::cout<<"RFbuckets["<<BeamIntensity[sizeofBeamIntensity-2]<<","<<BeamIntensity[sizeofBeamIntensity-1]
                                            <<"]"<<std::endl;
                            //if(sizeofBeamIntensity==2)
                            //    std::cout<<"RFbuckets["<<BeamIntensity[sizeofBeamIntensity-2]<<","<<BeamIntensity[sizeofBeamIntensity-1]
                            //        <<"]"<<std::endl;
                    }
                    if((counter)%297==297-1){
                            // Apply pedestal and baseline
                            for(int iBI=0;iBI<588;iBI++){
                                BeamIntensityRaw[iBI] = BeamIntensity[iBI];
                                if ( iBI != 32) 
                                        BeamIntensity[iBI] = BeamIntensity[iBI] - BeamIntensity[32] + 260;
                            }
                            BeamIntensity[32] = 260;

                            tree_QIE->Fill();
                            sizeofBeamIntensity = 0;
    
                            N_MIturn_Hi = 0;
                            N_MIturn_Lo = 0;
                            N_MIturn = 0;
                            capID = 0;
                            RFbuckets[0] = 0;
                            RFbuckets[1] = 0;
                            for(int iBeamIntensity=0;iBeamIntensity<588;iBeamIntensity++) BeamIntensity[iBeamIntensity]=0;
                            for(int iADC=0;iADC<588;iADC++) ADC[iADC]=0;
                    }
            }
            if(!status&&(counter)%297!=0)
                    std::cout<<"[warning] unexpected data : " <<std::hex<<std::setw(4)<<std::setfill('0')<<word<< std::dec<<std::endl;
            counter++;
    }


    summary_spill = atoi(argv[1]);
    summary_Trigger_size = tree_trigger->GetEntries();
    cyclic_trigger_mapItr_ = cyclic_trigger_map.find(summary_spill);
    if(cyclic_trigger_mapItr_==cyclic_trigger_map.end()){
            summary_Cyclic_RF_onset_status = 0;
            summary_Cyclic_RF_onset =  0;
    }else{
            summary_Cyclic_RF_onset_status = 1;
            summary_Cyclic_RF_onset =  cyclic_trigger_mapItr_->second;
    }
    int WBCrange[5] = {1,200, 233, 266, 299}; // [itself, 150, 175, 200, and 225]
/*
    for(int i=0;i<tree_trigger->GetEntries();i++){
            for(int j=0;j<5;j++) summary_Trigger_BeamIntensity[j] = 0;                
            if(i%1000==0) printf("[Processing] %1.2f \r",1.*i/tree_trigger->GetEntries());
            tree_trigger->GetEntry(i);
            summary_Trigger_count = trigger_counter;
            summary_Trigger_RF_onset  = trigger_RF_onset;
            summary_Trigger_turn_onset = trigger_turn_onset;

            summary_onset_time = (double)(18.9*(588*trigger_turn_onset + trigger_RF_onset)/pow(10,9));
            tree_QIE->GetEntry(trigger_turn_onset-1);
            for(int iera=0;iera<5;iera++){
                    for(int irf=0;irf<WBCrange[iera];irf++){
                            if(trigger_RF_onset-summary_Cyclic_RF_onset-irf >= 0 ){
                                    summary_Trigger_BeamIntensity[iera] += 
                                            (int)BeamIntensity[trigger_RF_onset-summary_Cyclic_RF_onset-irf];
                            }
                    }
            }
            tree_QIE->GetEntry(trigger_turn_onset-2);
            for(int iera=0;iera<5;iera++){
                    for(int irf=0;irf<WBCrange[iera];irf++){
                            if(trigger_RF_onset-summary_Cyclic_RF_onset-irf < 0 ){
                                    summary_Trigger_BeamIntensity[iera] += 
                                            (int)BeamIntensity[trigger_RF_onset-summary_Cyclic_RF_onset-irf + 588];
                            }
                    }
            }
            tree_summary->Fill();
    }
*/

    tree_QIE->Write();
    tree_trigger->Write();
    tree_inhibit->Write();
    //delete tree_QIE;
    //delete tree_trigger;
    //delete tree_inhibit;
    tree_summary->Write();
    tfile->Close();

    return 0;
}

bool read_16bits(unsigned short &word, bool swap){
    // unsigned short : size is 2 bytes = 16 bits
    if ( feof(dataStream) || !fread(&word,sizeof(word),1,dataStream) ){
        std::cout<<"Ending decoder"<<std::endl;
        return false;
    }
    unsigned char word8_a;
    unsigned char word8_b;
    word8_a = word;
    word8_b = word >> 8;
    if(swap)    word = (word8_a << 8) | word8_b;        
    return true;
}



bool read_32bits(unsigned int &word32, bool swap){
    // unsigned int : size is 4 bytes = 32 bits
    if ( feof(dataStream) || !fread(&word32,sizeof(word32),1,dataStream) ){
        std::cout<<"Ending decoder"<<std::endl;
        return false;
    }
    unsigned short word16_a;
    unsigned short word16_b;
    word16_a = word32;
    word16_b = word32 >> 16;
//    if(swap)    word32 = (word16_a << 16) | word16_b; // should be turned off affter 168126

    return true;
}

bool LoadCyclicTriggerMap(){
        ifstream Fcyclic("cyclic.log");
        if(!Fcyclic){
                std::cout<<"[WARNING] : No cyclic.log!"<<std::endl;
                return false;
        }
        char data[128];
        while(!Fcyclic.eof()){
                int spill = 0;
                int RFonset = 0;
                Fcyclic >> data;
                spill = atoi(data);
                Fcyclic >> data;
                RFonset = atoi(data);
                cyclic_trigger_map.insert( std::pair< int, int >(spill, RFonset) );
        }
        return true;
}
