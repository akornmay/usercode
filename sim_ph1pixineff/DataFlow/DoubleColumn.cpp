#include "DoubleColumn.h"

#include "TH1I.h"
#include "TH2I.h"

extern TH1I *DBSize, *TSSize, *ineffhits;

using namespace std;


void DoubleColumn::Init(int i, int roc, long *bx)
{
   Id=i;
   RocId=roc;
   clk=bx;
   NewEvent = true;
   DB_Full = false;
   TS_Full=false;
   RO_Mode=false;
   CD_Active = false;
   CD_Select = CD_SELECT_B;
   CD_Status = CD_SELECT_B;
   stat.Reset();
   n_reset=0;
   TS.Reset();
}

void DoubleColumn::AddHit(pxhit &hit)
{
  //hit.ineff = false;
   stat.total_hits++;
   if(n_reset==0 && RO_Mode) {
      stat.ro_Wait++;
      printf("FILL: n_reset==0 && RO_Mode +1 \n" );
      ineffhits->Fill( hit.mycol , hit.myrow );
      hit.ineff = true;
      return;
   }
   pixiter iHit;
   for(iHit=hits.begin(); iHit!=hits.end(); iHit++){
      if(iHit->row==hit.row) {                   // pixel overwrite
	stat.px_overwrite++;
	ineffhits->Fill( hit.mycol , hit.myrow );
	printf("FILL: iHit->row==hit.row \n");
	hit.ineff = true;
	return;
      }
   }
   for(iHit=pendinghits.begin(); iHit!=pendinghits.end(); iHit++){
      if(iHit->row==hit.row) 
	{
	  //ineffhits->Fill( hit.mycol , hit.myrow );
	  //hit.ineff = true;
	  //printf("FILL: iHit=pendinghits.begin \n");
	  return;             // pixel overwrite
	}
   }
   if(DB_Full) {                                 // book keeping only
     stat.DB_overflow++;
      ineffhits->Fill( hit.mycol , hit.myrow );
      hit.ineff = true;
      printf("FILL: DB_Full \n");
      return;
   }
   if(n_reset>0) {                               // book keeping only
      stat.ro_Reset++;
      ineffhits->Fill( hit.mycol , hit.myrow );
      hit.ineff = true;
      printf("FILL: stat.ro_Reset +1 [%i,%i]\n" , hit.mycol , hit.myrow );
      return;
   }
   if(TS_Full) {                                 // book keeping
      stat.TS_overflow++;
      pendinghits.push_back(hit);                // buffer full, cannot set timestamp yet
      hit.ineff = true;
      ineffhits->Fill( hit.mycol , hit.myrow );
      printf("FILL: TS_Full \n");
      return;
   }

   if(NewEvent) {
      NewEvent = false;
      wtg=hit.trigger;
      TrueTS=hit.timeStamp;
      TS_OK=SetTS(TrueTS, wtg, TrueCD);          // returns 0=set, -1=busy
      pixiter iHit=pendinghits.begin();
      while(iHit!=pendinghits.end()){
         iHit->timeStamp=TrueTS;
         iHit->trigger=wtg;
         iHit->wrongTS=true;
         iHit->CD_Select=CD_Select;
         hits.push_back((*iHit));
         iHit=pendinghits.erase(iHit);
      }
   }

   if(TS_OK<0) {                                 // dcol busy, pixel gets assigned
      stat.dcol_busy++;                          // to wrong column drain
      hit.timeStamp=TrueTS;
      hit.trigger=wtg;
      hit.wrongTS=true;
      hit.CD_Select=TrueCD;
      hit.ineff = true;
      ineffhits->Fill( hit.mycol , hit.myrow );
      printf("FILL: dcol_busy \n");
   } else {
      hit.wrongTS=false;                         // everything correct
      hit.CD_Select=CD_Select;                   // finally insert hit into pixel array
   }
   hits.push_back(hit);
}


void DoubleColumn::Clock()
{
   TSSize->Fill(TS.GetNEvents());
   DBSize->Fill(DB.GetSize());

   if(n_reset>0){                                // reset ongoing, takes 'n_reset' clocks to complete
      n_reset--;
      if(n_reset==0) Reset();
      return;
   } 
   if(RO_Mode) return;                           // do nothing, column is blocked
   NewEvent = true;
   if(CD_Active){                                // intimeStamps.Initserts hit into DB and deletes it from px array
      if(CD_Status>CD_SELECT_B) CD_Active=GetNextPxHit();
      CD_Status ^= 0x10;
   } else if(hits.size()>0) {
      CD_Active=true;
      CD_Status^=0x01;
      pixiter iHit;                              // send out LOAD to dcol
      for(iHit=hits.begin(); iHit!=hits.end(); iHit++){
         if(iHit->CD_Select==CD_Status) iHit->CD_Select+=0x10;
      }
   }
   long timeStamp=TS.Expiration(*clk);           // returns TS if expired, 0 otherwise
   if(timeStamp>0){
      if(DB.L1_verify(timeStamp)){               // clear hits if no trigger, mark for read out otherwise
         RO_Mode=true;                           // set readout mode and store time stamp
         RO_TS = timeStamp;
      }
   }
   DB_Full = DB.IsFull();                        // initiate reset if data buffer full
   if(DB_Full) n_reset=3;
   TS_Full = TS.IsFull();                        // block column if TS buffer full
}


void DoubleColumn::Reset()
{
   TS.Reset();
   int nPix=DB.Reset();                          // number of lost hits in data buffer and pixel array
   for(pixlist::iterator iHit=hits.begin(); iHit!=hits.end(); iHit++) {
      if(!iHit->wrongTS) 
	{
	  iHit->ineff = true;
	  ineffhits->Fill( iHit->mycol , iHit->myrow );
	  printf("FILL: DoubleColumn::Reset !iHit->wrongTS , [%i,%i]\n" , iHit->mycol , iHit->myrow );
	  nPix++;
	}
   }
   hits.clear();
   pendinghits.clear();
   if(RO_Mode) {
     stat.ro_Reset += nPix;
     printf("FILL: RO_Mode stat.ro_Reset + %i , total now = %i \n" , nPix , stat.ro_Reset);
   } else {
     stat.DB_overflow+=nPix;
     printf("FILL: stat.DB_overflow \n");
   }
   NewEvent = true;

   DB_Full = false;
   TS_Full=false;
   RO_Mode=false;
   CD_Active = false;
   CD_Select=CD_SELECT_B;
   CD_Status=CD_SELECT_B;
}

bool DoubleColumn::Readout(long timeStamp, pxhit &hit)
{
   if(RO_Mode && (RO_TS==timeStamp)){
      if(DB.Readout(timeStamp, hit)) return true;
      else {
         n_reset=3;
         return false;
      }
   }
   return false;
}


bool DoubleColumn::GetNextPxHit()                // returns false for last hit in event, true otherwise
{
   pixiter iHit;
   int jj=0;
   for(iHit=hits.begin(); iHit!=hits.end(); iHit++){
      if(iHit->CD_Select==CD_Status){
	     DB.InsertHit((*iHit));
	     iHit=hits.erase(iHit);
	     while(iHit!=hits.end()){
	        if(iHit->CD_Select==CD_Status) return true;
	        iHit++;
	     }
      }
   }
   return false;
}


int DoubleColumn::SetTS(long &timeStamp, bool &tg, int &cd)          // returns 0=ok, -1=busy
{
   int CD_new=CD_Select^0x01;
   pixlist::iterator iHit;
   for(iHit=hits.begin(); iHit!=hits.end(); iHit++){       // check for 3rd hit
       if(iHit->CD_Select==CD_new) {
    	   tg=iHit->trigger;
    	   timeStamp=iHit->timeStamp;                        // set new time stamp
    	   cd=CD_new;
    	   return -1;                                        // return -1 if dcol busy
       }
   }

   TS.InsertTS(timeStamp);                                 // insert time stamp
   CD_Select^=0x01;                                        // switch channel for column drain
   return 0;                                               // return 0 if TS set
}


/**************************************************************************************
 **************************************************************************************
 **************************************************************************************/


DataBuffer::DataBuffer()
{
   for(int i=0; i<DATA_BUFFER_SIZE; i++) {
      hits[i].clear();
   }
   iWrite=iRead=entries=0;
}


int DataBuffer::Reset()	                         // clears DataBuffer and returns number of hits discarded
{
   int ntot=0;
   for(int i=0; i<entries; i++){
       if(!hits[iRead++].wrongTS) {
	 hits[iRead-1].ineff = true;
	 ineffhits->Fill( hits[iRead-1].mycol , hits[iRead-1].myrow );
	 printf("FILL: DataBuffer::Reset() [%i,%i] \n" , hits[iRead-1].mycol , hits[iRead-1].myrow );
	 ntot++;
       }
       if(iRead==DATA_BUFFER_SIZE) iRead=0;
   }
   for(int i=0; i<DATA_BUFFER_SIZE; i++) {
      hits[i].clear();
   }
   iWrite=iRead=entries=0;
   return ntot;
}


bool DataBuffer::L1_verify(long TS)              // checks trigger and marks them for r/o
{                                                // or clears them from the data buffer
   while(hits[iRead].timeStamp==TS){
	   if(hits[iRead].trigger==1) return true;
	   hits[iRead++].clear();
	   entries--;
	   if(iRead==DATA_BUFFER_SIZE) iRead=0;
   }
   return false;
}

bool DataBuffer::Readout(long TS, pxhit &hit)
{
   if(hits[iRead].timeStamp!=TS) return false;
   hit=hits[iRead++];
   if(iRead==DATA_BUFFER_SIZE) iRead=0;
   return true;
}
