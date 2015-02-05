#include "Testboard.h"
#include "CommonDefs.h"
#include "TH1I.h"
#include "TF1.h"
extern TH1I *rodelay, *rotime, *eventsize;


Testboard::Testboard() : READOUT(0), gap(0), delay(0), ro_pix(0), ro_clocks(0)
{

  ROCs.resize(CHIPS_PER_TELESCOPE);
  clkDist = new double[2501];

  TF1 * clkFunc = new TF1("clkFunc","0.5*(TMath::Erf((x-[0])/(sqrt(2)*[1]))-TMath::Erf((x-[2])/(sqrt(2)*[1])))",0,25);
  clkFunc->SetParameters(-12.5,PIX_SIGMA,12.5);
  for(int i=0; i<2501; i++)
    {
      clkDist[i]=(*clkFunc)(i/100.);
    }
  rndPhase=new TRandom3(46644);
  delete clkFunc;
  
  triggerStack.clear();
}

Testboard::~Testboard()
{
  ROCs.clear();
}




void Testboard::Init(int id)
{
   Id=id;
   bx_counter=0;
   int rocId = 0;
   for(roc_iter iRoc=ROCs.begin(); iRoc != ROCs.end(); iRoc++)
     {
       iRoc->Init(rocId++, &bx_counter);
     }
   printf("with %i ROCs\n",rocId);

   first_token=ROCs.begin();
   last_token=ROCs.end();
   iWrite=iRead=0;
   for(int i=0; i<TELESCOPE_STACK_SIZE; i++) timeStamps[i]=-10000;
  
   ntrig = 0;
   ntoken = 0;
   evtnr = 0;
}

void Testboard::Reset()
{

  iWrite=iRead=0;                                         //reset read and write pointer
  for(int i=0; i<TELESCOPE_STACK_SIZE; i++)               //clear all timestamps in triggerstack 
    {
      timeStamps[i]=-10000;
    }

  for(roc_iter iRoc = ROCs.begin(); iRoc != ROCs.end(); iRoc++)
    {
      iRoc->Reset();
    }

}

void Testboard::AddHits(Event &event)
{
  if (event.trigger && triggerStack.size() < TELESCOPE_STACK_SIZE)
    {
      ntrig++;
      evtnr++;
      AddTS(event.clock);
      for(roc_iter iRoc=ROCs.begin(); iRoc!=ROCs.end(); iRoc++)
	{
	  iRoc->Trigger(event.clock);
	}      
      if(event.hits[0].size()==0)
	{
	  pxhit phit;
	  phit.event_number = evtnr;
	  phit.timeStamp=event.clock;
	  phit.pulseHeight=-1;
	  phit.vcal=-1;
	  phit.roc=-1;
	  phit.row=-1;
	  phit.dcol=-1;
	  phit.myrow = -1;
	  phit.mycol = -1;
	  phit.flux = 0;	  
	  phit.triggers_stacked = triggerStack.size();
	  phit.trigger_number = ntrig;
	  phit.token_number = ntoken;

	  //phit.printhit();
	  saveHit(&phit);
	}
      else
	{
	  hit_iterator hit;
	  for(hit=event.hits[0].begin(); hit!=event.hits[0].end(); hit++)
	    {
	      hit->event_number = evtnr;
	      hit->triggers_stacked = triggerStack.size();
	      hit->trigger_number = ntrig;
	      hit->token_number = ntoken;
	      saveTransparentHit(*hit);
	    }
	}
    }
  
  hit_iterator hit;
  for(hit=event.hits[0].begin(); hit!=event.hits[0].end(); hit++)
    {
      hit->event_number = evtnr;
      hit->triggers_stacked = triggerStack.size();
      hit->trigger_number = ntrig;
      hit->token_number = ntoken;
      ROCs[hit->roc].AddHit(*hit);
      // hit->printhit();
    }
}

void Testboard::AddTS(long TS)
{
   timeStamps[iWrite++]=TS;
   if(iWrite==TELESCOPE_STACK_SIZE) iWrite=0;
}


void Testboard::Clock()
{
  bx_counter++;
  for(roc_iter iRoc=ROCs.begin(); iRoc!=ROCs.end(); iRoc++) iRoc->Clock();

  int n_lost=0;
  long TS=timeStamps[iRead];
  if(bx_counter==(WBC+5+TS)){                       // sets TS = timeStamp if expired, 0 otherwise
    delay = TOKEN_DELAY;
    timeStamps[iRead++]=-10000;                      //expired timeStamp
    if(iRead==TELESCOPE_STACK_SIZE) 
      {
	iRead=0;                                     //circular buffer for iRead
	cout << "iRead roll over " << TS  << endl;
	++addcounter;
      } 
  } else TS=0;
  
  if(TS>0) 
    {
      triggerStack.push_back(TS);                    // have to wait, readout ongoing
    }
  
  if(delay>0)                                        //delaying the sending of a token
    {
      delay--;
      return;
    }
  
  if(gap>0)
    {
      gap--;
      return;
    }
  
  pxhit hit;
  bool finished = true;
  if(READOUT){                                            // token scan running, we are reading out the data
    if(n_transfer>0)
      {                                                  // last action not yet finished
	n_transfer--;
	if(n_transfer>0) return;
      }
    if(!token->Readout(RO_TS))                           // ROC->Readout(timeStamp TS): returns false if ROC readout buffer is empty or first timestamp in readout buffer != TS; returns true otherwise
      {                                                  // r/o finished for this ROC
	token++;                                         // go to next ROC
	if(token==last_token) 	
	  {                                              // end of readout
	    eventsize->Fill(evtsize);
	    
	    READOUT=false;
	    long ro_length=bx_counter-RO_start;
	    ro_clocks+=ro_length;
	    rotime->Fill(ro_length);
	    gap=TELESCOPE_READOUT_GAP;
	    
	    return;
	  }
	n_transfer = ROC_HEADER_LENGTH;                  // start header for next ROC
      } 
    else 
      {
	n_transfer = CLOCKS_PER_HIT;                     // a new hit to be read out
	ro_pix++;
	evtsize++;
      }
  } 
  else if(triggerStack.size()>0)
    {                       // initiate new readout token
      RO_TS=triggerStack.front();
      triggerStack.pop_front();
      n_transfer = ROC_HEADER_LENGTH;
      READOUT=true;
      ntoken++;
      token=first_token;
      RO_start=bx_counter;
      //      cout << "RO_start " << RO_start << endl;
      evtsize=0;
      rodelay->Fill(bx_counter-RO_TS-WBC);
    }
}

void Testboard::StatOut()
{
  cout << "addcounter " << addcounter << endl;
	char txt[50];
	std::vector<statistics> stat;
	stat.resize(CHIPS_PER_TELESCOPE);
	//	statistics stat[8];
	statistics mod_stat;
	mod_stat.Reset();
	int i=0;
	for(int i=0; i<CHIPS_PER_TELESCOPE; i++) {
	   cout << endl<<"**********************************************************"<<endl<<endl;
		ROCs[i].StatOut(stat[i]);
		sprintf(txt, "Roc number %d",i);
		stat[i].PrintStat(txt);
		mod_stat+=stat[i];
	}
	cout << endl<<endl<<"**********************************************************"<<endl<<endl;
	sprintf(txt, "Telescope %d",Id);
	mod_stat.PrintStat(txt);

   i=1;
   cout <<endl;

   
   
   cout << "     Pixel readout rate = "
	<<(double)ro_pix/((double) MAX_EVENT*25e-3)<<" MPix/s"<<endl;
   cout << "     Occupancy =          "
	<<((double)ro_clocks/(double) MAX_EVENT)*100<<" %"<<endl;

cout << endl;


}

int Testboard::GetBC(double phase)
{
    int toReturn=0;
    if(phase<0){
	phase+=25;
	toReturn--;
    }
    if(phase>25){
	phase-=25;
	toReturn++;
    }
    if(phase>25 || phase <0){std::cerr<<"wrong phase ("<<phase<<") !"<<endl; return(0);}
    double p=clkDist[(int)(phase*100)];
    if(p>1-1e-6){return(toReturn);}
    if(p<1e-6){return(toReturn+1);}
    if(rndPhase->Rndm()>p){return(toReturn+1);}
    else{return(toReturn);}
}
