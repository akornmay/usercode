#include "TBM.h"
#include "CommonDefs.h"
#include "TH1I.h"
extern TH1I *rodelay, *rotime, *eventsize;


TBM::TBM() : READOUT(0), gap(0), header(0), trailer(0), ro_pix(0), ro_clocks(0)
{
	triggerStack.clear();
}


void TBM::Init(roc_iter first, roc_iter last, int id, long int *bx)
{
   Id=id;
   bx_counter=bx;
   first_token=first;
   last_token=last;
   iWrite=iRead=0;
   for(int i=0; i<TBM_STACK_SIZE; i++) timeStamps[i]=-10000;
}


void TBM::AddTS(long TS)
{
   timeStamps[iWrite++]=TS;
   if(iWrite==TBM_STACK_SIZE) iWrite=0;
}


void TBM::Clock()
{
	int n_lost=0;
	long TS=timeStamps[iRead];
	if(*bx_counter==(WBC+5+TS)){                            // sets TS = timeStamp if expired, 0 otherwise
	  timeStamps[iRead++]=-10000;
	  if(iRead==TBM_STACK_SIZE) iRead=0;
	} else TS=0;

	if(TS>0) {
		triggerStack.push_back(TS);                          // have to wait, readout ongoing
	}
	if(header>0){
		header--;
		return;
	}
	if(trailer>0){
		trailer--;
		if(trailer==0) {
			READOUT=false;
			long ro_length=*bx_counter-RO_start;
			ro_clocks+=ro_length;
         rotime->Fill(ro_length);
			gap=TBM_READOUT_GAP;
		}
		return;
	}
	if(gap>0){
		gap--;
		return;
	}

   pxhit hit;
   bool finished = true;
   if(READOUT){                                            // token scan running
      if(n_transfer>0){                                    // last action not yet finished
         n_transfer--;
         if(n_transfer>0) return;
    	}
      if(!token->Readout(RO_TS)){                          // r/o finished for this ROC
         token++;                                          // go to next ROC
         if(token==last_token) 	{                         // end of readout
           trailer=TBM_TRAILER_LENGTH;                     // start TBM trailer
           eventsize->Fill(evtsize);
           return;
         }
         n_transfer = ROC_HEADER_LENGTH;                   // start header for next ROC
      } else {
         n_transfer = CLOCKS_PER_HIT;                      // a new hit to be read out
	      ro_pix++;
         evtsize++;
      }
   } else if(triggerStack.size()>0){                       // initiate new readout token
      RO_TS=triggerStack.front();
      triggerStack.pop_front();
      header=TBM_HEADER_LENGTH;
      n_transfer = ROC_HEADER_LENGTH;
      READOUT=true;
      token=first_token;
      RO_start=*bx_counter;
      evtsize=0;
      rodelay->Fill(*bx_counter-RO_TS-WBC);
   }
}
