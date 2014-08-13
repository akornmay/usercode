#include "EventReader.h"
#include "TString.h"
#include "CommonDefs.h"
#include "TH2I.h"
#include <cstdlib>

using namespace std;

extern TH2I *allhits;

void ASCIIReader::Init(int layer)
{
	int l=4;
	if(layer==2) l=7;
	else if(layer==3) l=11;
    for(int module=MIN_MOD-1; module<MAX_MOD; module++){
	    TString FileName(Form("/Users/kaestli/Software/workspace/PSI46_dataloss/data/tt_mb17_r%1d_z%1d.dat",l,module+1));
	    cout <<"Opening file "<<FileName<<endl;
	    is[module].open(FileName,std::ios::in);
	    if(!is[module]) {
		    std::cout << "Error: File \""<<FileName<<"\" doesn't exist."<<std::endl;
		    exit(0);
	    }
    }
}


void ASCIIReader::Terminate()
{
	for(int module=MIN_MOD-1; module<MAX_MOD; module++) is[module].close();
}


void ASCIIReader::ReadEvent(Event &event)
{	
	pxhit hit;
	int clk, nhits, lay, phi, z;
	for(int module=MIN_MOD-1; module<MAX_MOD; module++){
		is[module] >> clk >> nhits >> lay >> phi >> z;
		if(is[module].eof()){
			is[module].clear();
			is[module].seekg(0, std::ios::beg);
			is[module] >> clk >> nhits >> lay >> phi >> z;
		}
		int row, col, itof, in1, in2;
		for(int i=0; i<nhits; i++){
			is[module] >> row >> col >> itof >> hit.pulseHeight >> in1 >> in2;
	 	   hit.roc=row<80 ? (int)(col/52) : (int)(15-(col/52));
	      if(hit.roc>=CHIPS_PER_MODULE) continue;
	 	   hit.row=row<80 ? row+(col%2)*80 : 159-row+(1-col%2)*80;
	 	   hit.dcol=row<80 ? (int)((col % 52) / 2) : (int)(((415-col) % 52)/2);
			hit.timeStamp=event.clock;
			hit.trigger=event.trigger;
			event.hits[module].push_back(hit);
			allhits->Fill(col, row);
		}
	}
}


void RootReader::Init(int layer)
{
   rndm.SetSeed(93429);                          // random number generator
	
	E_PileUp=25./80.*TOTAL_XSECTION*PEAK_LUMI;    // expectation value for number of pile up events
	E_Signal=25./80.*SIGNAL_XSECTION*PEAK_LUMI;   // expectation value for number of signal events
	E_PileUp*=(double)BUNCH_SPACING/25.0;
	E_Signal*=(double)BUNCH_SPACING/25.0;
	if(CreatePileUp)  cout <<endl<< endl<<"Mean number of pileup events= "<<E_Signal+E_PileUp<<endl<<endl;
	std::list<std::string>::const_iterator iName;
	for(iName=SignalFileNames.begin(); iName!=SignalFileNames.end(); iName++){
		    RootHits *tree = new RootHits();
		    std::string FileName=*iName;
		    tree->Init(layer, FileName);
		    signalTrees.push_back(tree);
	}
	iSignal=signalTrees.begin();
	if(CreatePileUp){
 	   for(iName=SignalFileNames.begin(); iName!=SignalFileNames.end(); iName++){
		    RootHits *tree = new RootHits();
		    std::string FileName=*iName;
		    tree->Init(layer, FileName);
		    MB_Trees.push_back(tree);
 	   }
 	   iMinBias=MB_Trees.begin();
	}
}


void RootReader::ReadEvent(Event &event)
{	
   int nSignal=1;   
   int nPileUp=0;

   if(CreatePileUp){
      nSignal=rndm.Poisson(E_Signal);            // number of Signal events per BC (poisson distributed)
      nPileUp=rndm.Poisson(E_PileUp)-nSignal;    // number of MinBias events per BC (poisson distributed)
      (*iMinBias++)->GetHits(event, nPileUp);
      if(iMinBias==MB_Trees.end()) iMinBias=MB_Trees.begin();
   }	
   (*iSignal++)->GetHits(event, nSignal);
   if(iSignal==signalTrees.end()) iSignal=signalTrees.begin(); 
}


RootHits::~RootHits()
{
	HitFile->Close();
	delete HitFile;
}


void RootHits::GetHits(Event &event, int nEvents)
{
  pxhit hit;
  HitTree->GetEntry(rPointer++);
  if(rPointer==N_Entries) rPointer=0;
  int event_nr=tree_event;
  for(int i=0; i<nEvents; i++){
    do {
      if(DETECTOR==BPIX){
	if(tree_ladder==ladder && tree_module>=MIN_MOD && tree_module<=MAX_MOD) {
	  hit.timeStamp=event.clock;
	  hit.trigger=event.trigger;
	  hit.pulseHeight=adc;
	  hit.vcal=vcal;
	  hit.roc=row<80 ? (int)(col/52) : (int)(15-(col/52));
	  if(hit.roc>=CHIPS_PER_MODULE) continue;
	  hit.row=row<80 ? row+(col%2)*80 : 159-row+(1-col%2)*80;
	  hit.dcol=row<80 ? (int)((col % 52) / 2) : (int)(((415-col) % 52)/2);
	  hit.myrow = row;
	  hit.mycol = col;
	  hit.flux = flux;	  
	  event.flux = flux;
	  event.hits[tree_module-1].push_back(hit);
	  allhits->Fill((int)col,(int) row);
	}
	HitTree->GetEntry(rPointer++);
	if(rPointer==N_Entries) rPointer=0;
      } else {
            if(tree_ladder==ladder) {
               hit.timeStamp=event.clock;
               hit.trigger=event.trigger;
               hit.pulseHeight=adc;
               if(panel==1){
                  switch(tree_module) {
                  case 1:
                     hit.roc=(int)(col/52);
                     hit.row=row+(col%2)*80;
                     hit.dcol=(int)((col % 52) / 2);
                     break;
                  case 2:
                     hit.roc=2+(row<80 ? (int)(col/52) : (int)(5-(col/52)));
                     hit.row=row<80 ? row+(col%2)*80 : 159-row+(1-col%2)*80;
                     hit.dcol=row<80 ? (int)((col % 52) / 2) : (int)(((155-col) % 52)/2);
                     break;
                  case 3:
                     hit.roc=8+(row<80 ? (int)(col/52) : (int)(7-(col/52)));
                     hit.row=row<80 ? row+(col%2)*80 : 159-row+(1-col%2)*80;
                     hit.dcol=row<80 ? (int)((col % 52) / 2) : (int)(((207-col) % 52)/2);
                     break;
                  case 4:
                     hit.roc=16+(int)(col/52);
                     hit.row=row+(col%2)*80;
                     hit.dcol=(int)((col % 52) / 2);
                     break;
                  }
                  if(hit.roc>=CHIPS_PER_LINK[0]) cout <<"Error: roc number for panel 1 too high: "<<hit.roc<<endl;
               } else {
                  switch(tree_module) {
                  case 1:
                     hit.roc=row<80 ? (int)(col/52) : (int)(5-(col/52));
                     hit.row=row<80 ? row+(col%2)*80 : 159-row+(1-col%2)*80;
                     hit.dcol=row<80 ? (int)((col % 52) / 2) : (int)(((155-col) % 52)/2);
                     break;
                  case 2:
                     hit.roc=6+(row<80 ? (int)(col/52) : (int)(7-(col/52)));
                     hit.row=row<80 ? row+(col%2)*80 : 159-row+(1-col%2)*80;
                     hit.dcol=row<80 ? (int)((col % 52) / 2) : (int)(((207-col) % 52)/2);
                     break;
                  case 3:
                     hit.roc=14+(row<80 ? (int)(col/52) : (int)(9-(col/52)));
                     hit.row=row<80 ? row+(col%2)*80 : 159-row+(1-col%2)*80;
                     hit.dcol=row<80 ? (int)((col % 52) / 2) : (int)(((259-col) % 52)/2);
                     break;
                  }
                  if(hit.roc>=CHIPS_PER_LINK[1]) cout <<"Error: roc number for panel 2 too high: "<<hit.roc<<endl;
                  hit.roc+=CHIPS_PER_LINK[0];
               }
               event.hits[0].push_back(hit);
               allhits->Fill((int)col,(int) row);
            }
            HitTree->GetEntry(rPointer++);
            if(rPointer==N_Entries) rPointer=0;
         }
	   } while(tree_event==event_nr);
      event_nr=tree_event;
	}
	rPointer--;
}


void RootHits::Init(int lr, std::string &name)
{
   cout <<"Trying to open file "<<name<<endl;
   HitFile = TFile::Open(name.c_str(), "READ");
   if(!HitFile) {
      std::cout << "File \""<<name<<"\" not found."<< std::endl;
      exit(0);
   }
   ladder=LADDER;
   layer=lr;
   char title[128];
   if(DETECTOR==BPIX) {
      sprintf(title,"BPIX_Digis_Layer%1d",layer);
      HitTree=(TTree*) HitFile->Get(title);
      HitTree->SetBranchAddress("Ladder",&tree_ladder);
   }
   else {
      sprintf(title,"FPIX_Digis_Side%1d_Disk%1d",(int)(layer/3)+1, layer%2+1);
      HitTree=(TTree*) HitFile->Get(title);
      HitTree->SetBranchAddress("Blade",&tree_ladder);
      HitTree->SetBranchAddress("Panel",&panel);
   }
   HitTree->SetBranchAddress("vcal",&vcal);
   HitTree->SetBranchAddress("Module",&tree_module);
   HitTree->SetBranchAddress("Event",&tree_event);
   HitTree->SetBranchAddress("col",&col);
   HitTree->SetBranchAddress("row",&row);
   HitTree->SetBranchAddress("adc",&adc);
   HitTree->SetBranchAddress("flux",&flux);
   N_Entries=(UInt_t) HitTree->GetEntries();
   UInt_t rPointer=0;
   cout <<"File "<<name<<" opened."<<endl;
}

