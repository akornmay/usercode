#include "ROC.h"
#include "TH2I.h"

extern TH2I *ExtTokenWait;
extern TH2I *IntTokenWait, *DcolTokenWait, *DCTokenWait;

ROC::ROC():
   n_hits(0), newEvent(true), n_transfer(0), n_hits_ro(0), newDcol(true),
   newChip(true), RO_TS(-1),
   return1(0),   return2(0),   return3(0),   return4(0),   return5(0)
{
   dcols.resize(DCOLS_PER_ROC);
   header=ROC_HEADER_LENGTH;
   token=dcols.begin();
}


void ROC::Init(int id, long int *bx)
{ 
   Id=id;
   bx_counter=bx;
   for(int i=0; i<DCOLS_PER_ROC; i++) {
     dcols[i].Init(i, Id, bx);
   }
   header=ROC_HEADER_LENGTH;
   token=dcols.begin();
   ReadoutBuffer.clear();
   triggers.clear();
   ++return5;
}

void ROC::Reset()
{
  
  for(int i=0; i<DCOLS_PER_ROC; i++) {
    dcols[i].Reset();
  }

  token=dcols.begin();
  ReadoutBuffer.clear();
  triggers.clear();
  ++return6;
  
}

void ROC::AddHit(pxhit &hit)
{
   n_hits++;
   dcols[hit.dcol].AddHit(hit);	                // insert hit into double column
}

void ROC::Clock(){
   for(dcol_iter iDcol=dcols.begin(); iDcol!=dcols.end(); iDcol++) iDcol->Clock();

   if(n_transfer>0) {
      n_transfer--;
      ++total_transfer;
      return;
   }
   if(token==dcols.end()){
      if(triggers.empty())
	{
	  ++return1;
	  return;
	}
      RO_TS = triggers.front();
      if(*bx_counter<RO_TS+WBC)
	{
	  ++return2;
	  return;
	}
      triggers.pop_front();
      token=dcols.begin();
      newChip=true;
   }
   if(ReadoutBuffer.size()==ROC_BUFFER_SIZE) {
     ++return3; 
     return;
   }
   if(newChip){
      IntTokenWait->Fill(Id,*bx_counter-(RO_TS+WBC));
      newChip=false;
   }
   pxhit hit;
   while(token!=dcols.end() && !token->Readout(RO_TS, hit)) {
      newDcol=true;
      token++;
      if(token==dcols.end())
	{
	  ++return4; 
	  return;
	}
   }

   if(newDcol) {
      DcolTokenWait->Fill(token->GetId(),*bx_counter-(RO_TS+WBC));
      DCTokenWait->Fill(Id*26+token->GetId(),*bx_counter-(RO_TS+WBC));
      newDcol=false;
   }
   ReadoutBuffer.push_back(hit);
   n_hits_ro++;
   n_transfer=CONVERSION_TIME;
   return;
}

bool ROC::Readout(long TS)
{
   if(ReadoutBuffer.empty()) return false;
   if(TS!=ReadoutBuffer.front().timeStamp) return false;
   ReadoutBuffer.pop_front();
   return true;
}


void ROC::StatOut(statistics &stat)
{
  cout <<  "n_hits_ro " << n_hits_ro << endl;
  cout <<  "return1 " << return1 << endl;
  cout <<  "return2 " << return2 << endl;
  cout <<  "return3 " << return3 << endl;
  cout <<  "return4 " << return4 << endl;
  cout <<  "return5 " << return5 << endl;
  cout <<  "return6 " << return6 << endl;
  cout <<  "total_transfer " << total_transfer << endl;
	stat.Reset();
	dcol_iter iDcol;
	for(iDcol=dcols.begin(); iDcol!=dcols.end(); iDcol++)
		stat += iDcol->StatOut();
	stat.px_fluence=(double)n_hits/((double)MAX_EVENT*25E-3)/0.6561;
}
