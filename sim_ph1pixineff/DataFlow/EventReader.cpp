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
  if(nSignal != 0) signalTree->GetHits(event, nSignal);

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
   HitTree->SetParallelUnzip(true,1.0);
   
   HitTree->SetBranchStatus("*",false);

   HitTree->SetBranchStatus("vcal",true);
   HitTree->SetBranchStatus("roc",true);
   HitTree->SetBranchStatus("Event",true);
   HitTree->SetBranchStatus("col",true);
   HitTree->SetBranchStatus("row",true);
   HitTree->SetBranchStatus("pulseHeight",true);

   HitTree->SetBranchAddress("vcal",&vcal);
   HitTree->SetBranchAddress("roc",&roc);
   HitTree->SetBranchAddress("Event",&tree_event);
   HitTree->SetBranchAddress("col",&col);
   HitTree->SetBranchAddress("row",&row);
   HitTree->SetBranchAddress("pulseHeight",&adc);
   // HitTree->SetBranchAddress("flux",&flux);
   N_Entries=(UInt_t) HitTree->GetEntries();
   cout <<"File "<<name<<" opened."<<endl;

   //initialize the random number generator
   randomNo = new TRandom3(0);


   fillEventLibrary();
 }


TelescopeHits::~TelescopeHits()
{
	HitFile->Close();
	delete HitFile;
}


void TelescopeHits::GetHits(Event &event, int nEvents)
{
  //std::cout << "getHit" << std::endl;
  for(int i=0; i<nEvents; i++)
    {
      // we need to randomize which event we will be looking at
      // this should give me one of the entries in the tree
      
      long int randomEvent = (long int)(randomNo->Rndm() * myLibrarySize) ;
      
      //we get the event from our event library
      if(myEventLibrary[randomEvent][0].roc == -1)
	{      
	  //if we pick an empty event we set back the counter by one and pick a new random event
	  --i;
	  continue;

	}
      else
	{
	  event.hits[0] = myEventLibrary[randomEvent];
	  
	  //now we just need to add some event informations to the hits
	  for(int kk  = 0; kk < event.hits[0].size(); ++kk)
	    {
	      //    event.hits[0][kk].printhit();
	      
	      event.hits[0][kk].timeStamp=event.clock;
	      event.hits[0][kk].trigger=event.trigger;
	      event.hits[0][kk].flux = flux;
	      allhits->Fill((int)event.hits[0][kk].mycol,(int)event.hits[0][kk].myrow);
	      //hit.printhit();
	      //	  printf("Hit: Event %i adc %f roc %i (%i|%i)\n",event_nr,adc,roc,col,row);
	    }
	}
    }
  
}


void TelescopeHits::fillEventLibrary()
{

  pxhit hit;

  int tempEventNo = -1;
  int mapkey = 0;

  for(int ii = 0; ii < N_Entries; )
    {
      HitTree->GetEntry(ii);
      tempEventNo = tree_event;
      //std::cout << "ii " << ii << endl;

      hit_vector& ahitVector = myEventLibrary[mapkey];

      while(tree_event == tempEventNo)
	{
	  //create the new hit
	  //hit.timeStamp=event.clock;   these are things I have to add later to the hit
	  //hit.trigger=event.trigger;
	  hit.pulseHeight=adc;
	  hit.vcal=vcal;
	  hit.roc=roc;
	  hit.row=row;
	  hit.dcol=col/2;
	  hit.myrow = row;
	  hit.mycol = col;
	  hit.flux = flux;	  
	  //	  event.flux = flux;

	  //and push it into our hitVector
	  ahitVector.push_back(hit);
	  //and go to the next event
	  ++ii;
	  if(ii == N_Entries) break;
	  HitTree->GetEntry(ii);
	}

      ++mapkey;
      //std::cout << "mapkey " << mapkey << std::endl;
    }


  myLibrarySize = myEventLibrary.size();
  
  std::cout << "Size of map " << myEventLibrary.size() << std::endl;
  /*
  for(std::map<int,hit_vector>::iterator it = myEventLibrary.begin(); it != myEventLibrary.end(); ++it)
    {
      std::cout << "new event" << std::endl;
      for(int kk = 0; kk < it->second.size(); ++kk)
	{
	  it->second[kk].printhit();
	}

    } 

  */

}





