#include "EventReader.h"
#include "TString.h"
#include "CommonDefs.h"
#include "TH2I.h"
#include <cstdlib>

using namespace std;

extern TH2I *allhits;

/*
 * the telescope part
 */

//define the TelescopeReader that reads the data files


void TelescopeReader::Init()
{
  // read data file
  signalTree = new TelescopeHits();	
  signalTree->Init(SignalFileName);
  
  InitQIE(QIEfileName);

  QIEeventcounter = 0;
  Turn = QIEeventcounter/588 ;
  Bucket = QIEeventcounter%588 ;
  
}


void TelescopeReader::ReadEvent(Event &event)
{
  //here we read the event entrie from the QIE device and calculate the number of tracks
  QIETree->GetEntry(Turn);
  //cout << BeamIntensity[Bucket] << "," << BeamIntensity[32] << endl;   
  //  cout << "reading event with ";
  int nSignal = NumberOfParticles(BeamIntensity[Bucket], BeamIntensity[32]);   
  //cout << nSignal << " Tracks" << endl;
  signalTree->GetHits(event, nSignal);

  ++QIEeventcounter;
  Turn = QIEeventcounter/588 ;
  Bucket = QIEeventcounter%588 ;

}


int TelescopeReader::NumberOfParticles(double QIEintensity, double pedestal)
{
  int NumberOfP = (int)(((QIEintensity - pedestal + BEAMINTENSITY_PARAM1)/BEAMINTENSITY_PARAM2) * BEAMINTENSITY_SCALING);

  return NumberOfP;
}


void TelescopeReader::InitQIE(std::string &name)
{
   cout <<"Trying to open QIE file "<< name << endl;
   QIEFile = TFile::Open(name.c_str(), "READ");
   if(!QIEFile) {
      std::cout << "File \""<<name<<"\" not found."<< std::endl;
      exit(0);
   }

   char title[128];
   sprintf(title,"tree_QIE");
   QIETree=(TTree*) QIEFile->Get(title);
   
   QIETree->SetBranchStatus("*",false);

   QIETree->SetBranchStatus("BeamIntensity",true);
   QIETree->SetBranchAddress("BeamIntensity",BeamIntensity);

   }

void TelescopeHits::Init(std::string &name)
{
   cout <<"Trying to open file "<<name<<endl;
   HitFile = TFile::Open(name.c_str(), "READ");
   if(!HitFile) 
     {
       std::cout << "File \""<<name<<"\" not found."<< std::endl;
       exit(0);
     }
   else
     {
       cout << "Done" << endl;
     }
   char title[128];
   sprintf(title,"Telescope");
   HitTree=(TTree*) HitFile->Get(title);
   
   HitTree->SetBranchAddress("vcal",&vcal);
   HitTree->SetBranchAddress("roc",&roc);
   HitTree->SetBranchAddress("Event",&tree_event);
   HitTree->SetBranchAddress("col",&col);
   HitTree->SetBranchAddress("row",&row);
   HitTree->SetBranchAddress("pulseHeight",&adc);
   HitTree->SetBranchAddress("flux",&flux);
   N_Entries=(UInt_t) HitTree->GetEntries();
   cout <<"File "<<name<<" opened."<<endl;

   //initialize the random number generator
   randomNo = new TRandom3(0);

}


TelescopeHits::~TelescopeHits()
{
	HitFile->Close();
	delete HitFile;
}


void TelescopeHits::GetHits(Event &event, int nEvents)
{

  // we need to randomize which event we will be looking at
  // this should give me one of the entries in the tree
  long int randomEvent = (long int)(randomNo->Rndm() * N_Entries) ;
  HitTree->GetEntry(randomEvent);
  //since we picked a random hit in the tree we run over events until the eventnumber changes
  long int randomEventNo = tree_event;
  while(tree_event == randomEventNo)
    {
      ++randomEvent;
      if(randomEvent == N_Entries) randomEvent = 0;
      HitTree->GetEntry(randomEvent);
    }
  //now we use the next event to be actually read out
  pxhit hit;
  HitTree->GetEntry(randomEvent);
  if(randomEvent == N_Entries) randomEvent = 0;
  int event_nr=tree_event;
  for(int i=0; i<nEvents; i++){
    do {
      if(roc != -1)
      	{
	  
	  hit.timeStamp=event.clock;
	  hit.trigger=event.trigger;
	  hit.pulseHeight=adc;
	  hit.vcal=vcal;
	  hit.roc=roc;
	  hit.row=row;
	  hit.dcol=col/2;
	  hit.myrow = row;
	  hit.mycol = col;
	  hit.flux = flux;	  
	  event.flux = flux;
	  event.hits[0].push_back(hit);
	  allhits->Fill((int)col,(int) row);
	  //	  hit.printhit();
	  //	  printf("Hit: Event %i adc %f roc %i (%i|%i)\n",event_nr,adc,roc,col,row);
      
	}

      HitTree->GetEntry(randomEvent++);
      if(randomEvent == N_Entries) randomEvent = 0;
    } while(tree_event==event_nr);
    
    event_nr=tree_event;
  }

}








