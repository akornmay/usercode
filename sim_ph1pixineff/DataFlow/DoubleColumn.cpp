#include "DoubleColumn.h"

#include <map>

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
   lastPixelReadoutRow = -1;
   TS.Reset();
   hits.clear();
   pendinghits.clear();
}

void DoubleColumn::AddHit(pxhit &hit)
{
   stat.total_hits++;
   if(n_reset==0 && RO_Mode) {
     //printf("FILL: n_reset==0 && RO_Mode +1 \n" );
     ineffhits->Fill( hit.mycol , hit.myrow );
     hit.ineff = true;
     hit.inefftype = 1; // 1 = ro_Wait
     stat.inefficient_hits++;
     stat.ro_Wait++;
     return;
   }

   if (!phaseOK(hit.phase)){
     stat.badPhase++;
     stat.inefficient_hits++;
     return;
   }

   pixiter iHit;
   for(iHit=hits.begin(); iHit!=hits.end(); iHit++){
      if(iHit->row==hit.row) {                   // pixel overwrite
	stat.px_overwrite++;
	ineffhits->Fill( hit.mycol , hit.myrow );
	//printf("FILL: iHit->row==hit.row \n");
	hit.ineff = true;
	hit.inefftype = 2; // 1 = ro_Wait, 2 = px_overwrite
	stat.inefficient_hits++;
	return;
      }
   }
   for(iHit=pendinghits.begin(); iHit!=pendinghits.end(); iHit++){
      if(iHit->row==hit.row) {
	//	stat.px_overwrite++;                    // pixel overwrite
	ineffhits->Fill( hit.mycol , hit.myrow );
	hit.ineff = true;
	//printf("FILL: iHit=pendinghits.begin \n");
	return;             
      }
   }
   if(DB_Full) {                                 // book keeping only
     stat.DB_overflow++;
     ineffhits->Fill( hit.mycol , hit.myrow );
     hit.ineff = true;
     hit.inefftype = 3; // 1 = ro_Wait, 2 = px_overwrite , 3 = DB_overflow
     //printf("FILL: DB_Full \n");
     stat.inefficient_hits++;
     return;
   }
   if(n_reset>0) {                               // book keeping only
      stat.ro_Reset++;
      ineffhits->Fill( hit.mycol , hit.myrow );
      hit.ineff = true;
      hit.inefftype = 4; // 1 = ro_Wait, 2 = px_overwrite , 3 = DB_overflow, 4 = ro_Reset
      //printf("FILL: stat.ro_Reset +1 [%i,%i]\n" , hit.mycol , hit.myrow );
      stat.inefficient_hits++;      
      return;
   }
   if(TS_Full) {                                 // book keeping
      stat.TS_overflow++;
      pendinghits.push_back(hit);                // buffer full, cannot set timestamp yet
      hit.ineff = true;
      hit.inefftype = 5; // 1 = ro_Wait, 2 = px_overwrite , 3 = DB_overflow, 4 = ro_Reset, 5 = TS_overflow
      ineffhits->Fill( hit.mycol , hit.myrow );
      //printf("FILL: TS_Full \n");
      stat.inefficient_hits++;      
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
	 stat.hits_seen++;
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
      hit.inefftype = 6; // 1 = ro_Wait, 2 = px_overwrite , 3 = DB_overflow, 4 = ro_Reset, 5 = TS_overflow, 6 = dcol_busy
      ineffhits->Fill( hit.mycol , hit.myrow );
      stat.inefficient_hits++;
      //     printf("FILL: dcol_busy \n");
   } else {
     hit.wrongTS=false;                         // everything correct
     hit.CD_Select=CD_Select;                   // finally insert hit into pixel array
   }
   hits.push_back(hit);
   stat.hits_seen++;
   sorthitsbyrow();

}


void DoubleColumn::sorthitsbyrow(){

  std::multimap<int,pxhit> sortedhits;
  for(pixiter iHit = hits.begin(); iHit != hits.end(); ++iHit)
    {
      sortedhits.insert(std::pair<int,pxhit>(iHit->row,*iHit));
    }
  hits.clear();
  for(std::multimap<int,pxhit>::iterator it = sortedhits.begin(); it != sortedhits.end(); ++it)
    {
      hits.push_back(it->second);
    }
}


int DoubleColumn::getPixelReadoutDelay() {

  int pixelWait = 2;

  if(lastPixelReadoutRow == -1)
    {
      if(nextPixelReadoutRow < cd_token_pix_offset){ pixelWait += 0;}
      else if(nextPixelReadoutRow < cd_token_pix_offset + cd_token_pix_per_clk){ pixelWait += 1;}
      else { pixelWait += 2;}

      lastPixelReadoutRow = nextPixelReadoutRow;
      return pixelWait;
    } 
  else
    {
      int distance = nextPixelReadoutRow - lastPixelReadoutRow;
      if(distance > cd_token_pix_per_clk)
	{
	  pixelWait += distance/cd_token_pix_per_clk ;
	}
      else
	{
	  pixelWait +=0;
	}
      lastPixelReadoutRow = nextPixelReadoutRow;
      return pixelWait;
    }

  std::cerr << "WAIT... WHAT?!?!?!" << std::endl;
  return -1;
}

void DoubleColumn::performReadoutDelay() {
  pixelReadoutTimer--;
  if (pixelReadoutTimer<=0) {
    GetNextPxHit();
    if (SpyNextPxHit()) 
      {
	pixelReadoutTimer = getPixelReadoutDelay();
      } 
    else
      {    
	if(nextPixelReadoutRow < (rowsPerDC - cd_token_pix_per_clk))
	  {
	    pixelReadoutTimer = (rowsPerDC - nextPixelReadoutRow)/cd_token_pix_per_clk;
	    nextPixelReadoutRow = rowsPerDC;
	  }
	else
	  {
	    nextPixelReadoutRow = 0;
	    CD_Active = false;
	  }
      }
  }
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

   if(RO_Mode){ return; }                          // do nothing, column is blocked
   NewEvent = true;
   if(CD_Active){                                // intimeStamps.Initserts hit into DB and deletes it from px array
     performReadoutDelay();
   } else if(hits.size()>0) {
      CD_Active=true;
      CD_Status^=0x01;
      pixelReadoutTimer = 0;
      lastPixelReadoutRow = -1;
      
      for(pixiter iHit=hits.begin(); iHit!=hits.end(); iHit++){
	if(iHit->CD_Select==CD_Status) iHit->CD_Select|=0x10;
      }
      if (SpyNextPxHit()) {
	pixelReadoutTimer = getPixelReadoutDelay();
	//performReadoutDelay();
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
   if(DB_Full)
     {
       n_reset=reset_bx;
     }
   TS_Full = TS.IsFull();                        // block column if TS buffer full
}


void DoubleColumn::Reset()
{
   TS.Reset();
   int nPix=DB.Reset();                          // number of lost hits in data buffer and pixel array
   stat.inefficient_hits+=nPix;
   for(pixlist::iterator iHit=hits.begin(); iHit!=hits.end(); iHit++) {
     if(!iHit->wrongTS)
       {
	 iHit->ineff = true;
	 ineffhits->Fill( iHit->mycol , iHit->myrow );
	 //	  printf("FILL: DoubleColumn::Reset !iHit->wrongTS , [%i,%i]\n" , iHit->mycol , iHit->myrow );
	 nPix++;
       }
   }
   hits.clear();
   pendinghits.clear();
   if(RO_Mode) {
     stat.ro_Reset += nPix;
   } else {
     stat.DB_overflow+=nPix;
   }

   lastPixelReadoutRow = -1;

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
      if(DB.Readout(timeStamp, hit))
	{
	  saveHit(&hit);
	  return true;
	}
      else {
	n_reset= reset_bx;
	return false;
      }
   }
   return false;
}


// returns true if there is any pixel to be read
bool DoubleColumn::SpyNextPxHit()
{
  pix_const_iter iHit;
  int jj=0;
  int CD_Status_high= CD_Status|0x10;

  for(iHit=hits.begin(); iHit!=hits.end(); iHit++){
    if(iHit->CD_Select==CD_Status_high){
      nextPixelReadoutRow = iHit->row;
      return true;
    }
  }
  //  nextPixelReadoutRow = 0;
  return false;
}

bool DoubleColumn::GetNextPxHit()                // returns false for last hit in event, true otherwise
{
   pixiter iHit;
   int jj=0;
   int CD_Status_high=CD_Status|0x10;
   for(iHit=hits.begin(); iHit!=hits.end(); iHit++){
     if(iHit->CD_Select==CD_Status_high){
       //   cout << "moving hit to DB " ; iHit->printhit();
       DB.InsertHit((*iHit));
       iHit=hits.erase(iHit);
       return true;
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
   TS_Full=TS.IsFull();
   CD_Select^=0x01;                                        // switch channel for column drain
   return 0;                                               // return 0 if TS set
}


/**************************************************************************************
 **************************************************************************************
 **************************************************************************************/


DataBuffer::DataBuffer()
{
   for(int i=0; i<DATA_BUFFER_SIZE; i++) {
      hits[i].init();
   }
   iWrite=iRead=entries=0;
}


int DataBuffer::Reset()	                         // clears DataBuffer and returns number of hits discarded
{
   int ntot=0;
   for(int i=0; i<entries; i++){
     if(!hits[iRead].wrongTS && hits[iRead].timeStamp != 0) {                // counts the number of hits with correct TS in the data buffer 
       hits[iRead].ineff = true;
       ineffhits->Fill(hits[iRead].mycol , hits[iRead].myrow );
       ntot++;
     }
     //     cout << "RESET" ;hits[iRead].printhit();
     iRead++;
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


bool DoubleColumn::phaseOK(double phase){
  return(1);
    
  //return(phase>9.5 && phase<14);
  
}
