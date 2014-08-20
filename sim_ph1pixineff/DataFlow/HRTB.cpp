#include "HRTB.h"
#include "CommonDefs.h"
#include "TH1I.h"
extern TH1I *rodelay, *rotime, *eventsize;


HRTB::HRTB() : READOUT(0), gap(0), ro_pix(0), ro_clocks(0)
{
	triggerStack.clear();
}


void HRTB::Init(roc_iter first, roc_iter last, int id, long int *bx)
{
   Id=id;
   bx_counter=bx;
   first_token=first;
   last_token=last;
   iWrite=iRead=0;
   for(int i=0; i<TELESCOPE_STACK_SIZE; i++) timeStamps[i]=-10000;
}


void HRTB::AddTS(long TS)
{
   timeStamps[iWrite++]=TS;
   if(iWrite==TELESCOPE_STACK_SIZE) iWrite=0;
}


void HRTB::Clock()
{
	int n_lost=0;
	long TS=timeStamps[iRead];
	if(*bx_counter==(WBC+5+TS)){                            // sets TS = timeStamp if expired, 0 otherwise
	  timeStamps[iRead++]=-10000;                           //expired timeStamp
	  if(iRead==TELESCOPE_STACK_SIZE) iRead=0;              //circular buffer for iRead
	} else TS=0;
	
	if(TS>0) {
	  triggerStack.push_back(TS);                           // have to wait, readout ongoing
	}

	if(gap>0){
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
	      long ro_length=*bx_counter-RO_start;
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
       token=first_token;
       RO_start=*bx_counter;
       evtsize=0;
       rodelay->Fill(*bx_counter-RO_TS-WBC);
     }
}
