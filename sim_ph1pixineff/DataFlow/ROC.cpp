#include "ROC.h"
#include "TH2I.h"

extern TH2I *ExtTokenWait;
extern TH2I *IntTokenWait, *DcolTokenWait, *DCTokenWait;

ROC::ROC():
   n_hits(0), newEvent(true), n_transfer(0), n_hits_ro(0), newDcol(true),
   newChip(true), RO_TS(-1)
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
      return;
   }
   if(token==dcols.end()){
      if(triggers.empty()) return;
      RO_TS = triggers.front();
      if(*bx_counter<RO_TS+WBC) return;
      triggers.pop_front();
      token=dcols.begin();
      newChip=true;
   }
   if(ReadoutBuffer.size()==ROC_BUFFER_SIZE) {
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
      if(token==dcols.end()) return;
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
	stat.Reset();
	dcol_iter iDcol;
	for(iDcol=dcols.begin(); iDcol!=dcols.end(); iDcol++)
		stat += iDcol->StatOut();
	stat.px_fluence=(double)n_hits/((double)MAX_EVENT*25E-3)/0.6561;
}
