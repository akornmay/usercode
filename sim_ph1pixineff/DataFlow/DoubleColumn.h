#ifndef DOUBLE_COLUMN_H_
#define DOUBLE_COLUMN_H_

#include "CommonDefs.h"
#include "Statistics.h"

typedef std::list<pxhit> pixlist;
typedef std::list<pxhit>::iterator pixiter;

enum {CD_SELECT_A=0, CD_SELECT_B};

class TSlist
{
public:

};


/** \class TimeStamp
 * Class implementing the time stamp buffer
 * @author Hans-Christian Kaestli
 */ 
class TimeStamp
{	
public:
   /** @brief Writing a time stamp 
    * 
    * @param &TS : time stamp
    * 
    * Adds a pair of time stamp & L1 expiration to the time stamp buffer. 
    */ 	
   void InsertTS(long ts){
      TS[iWrite++]=ts;
      if(iWrite==TS_BUFFER_SIZE) iWrite=0;
      entries++;
   };
	
   /** @brief Check expiration of time stamps 
    * 
    * @param long *clk : actual time in LHC clocks
    *
    * @return long : time stamp if expired, 0 otherwise
    * 
    * checks whether a time stamp has expired. 
    * Return time stamp if expired, 0 otherwise.
    */ 	
   long Expiration(long &clk){	 // checks expiration of time stamps. Return TS if expired, 0 otherwise
      long ts=TS[iRead];
      if(ts!=clk-WBC) return 0;
      TS[iRead++]=0;
      if(iRead==TS_BUFFER_SIZE) iRead=0;
      entries--;
      return ts;
   };

   /** @brief constructor */
   TimeStamp()  { Reset(); };
	
   /** @brief Clear time stamp buffer */	
   void Reset(){
      iWrite=iRead=entries=0;
      for(int i=0; i<TS_BUFFER_SIZE; i++) TS[i]=0;
   }

   /** @brief Check if time stamp buffer is full */	
   bool IsFull() {return entries==TS_BUFFER_SIZE;};
	
   /** @brief Get the number of time stamps in the double column */	
   int GetNEvents() { return entries;};

private:
   int iWrite;
   int iRead;
   int entries;
   long TS[TS_BUFFER_SIZE];
};


/** \class DataBuffer
 * Class implementing the data buffer
 * @author Hans-Christian Kaestli
 */ 
class DataBuffer
{
public:
   /** @brief constructor */
   DataBuffer();
	
   /** @brief Clear data buffer and returns number of hits discarded
    *
    * @return int : total number of discarded pixels
    *
    * Clear data buffer and returns number of hits discarded
    */
    int Reset();
       
   /** @brief Writing a hit to the data buffer 
    * 
    * @param &Hit : reference to pixel hit structure
    * 
    * Adds a pixel hit to the data buffer list. 
    */ 
   void InsertHit(pxhit &Hit) { hits[iWrite++]=Hit;
                                if(iWrite==DATA_BUFFER_SIZE) iWrite=0;
                                entries++; };
	
   /** @brief Deleting hits from the data buffer 
    * 
    * @param TimeStamp : time stamp which has expired
    * @return int : number of hits corresponding to TimeStamp with L1A
    * 
    * Clear all hits in the data buffer with given time stamp 
    * return number of hits to be read out
    */ 
   bool L1_verify(long TimeStamp);

   /** @brief Get the number of hits in the data buffer */	
   int GetSize() { return entries;};
	
   /** @brief Check if data buffer is full */	
   bool IsFull() { return entries==DATA_BUFFER_SIZE; };

   bool Readout(long timeStamp, pxhit &hit);

private:
   int iWrite;
   int iRead;
   int entries;
   pxhit hits[DATA_BUFFER_SIZE];
};


/** \class DoubleColumn
 * Class implementing a double column
 * @author Hans-Christian Kaestli
 */ 
class DoubleColumn
{
public:

	void Init(int i, int roc, long *bx);
   /** @brief Adding a hit to the pixel array
    * 
    * @param &hit : reference to pixel hit structure
    * 
    * Adds pixel hit to the array of pixels waiting for a column drain. 
    * Checks first whether DB is full, double column is in readout mode,
    * or hit FF in pixel is still active.
    * Checks for column drain busy and sets timestamp. Updates statistics. 
    */ 
   void AddHit(pxhit &hit);
	
   /** @brief Updates the status of the double column
    * 
    * Has to be called once for each clock cycle. Does the column drain, 
    * checks for expired time stamps, sets readout 
    * mode and checks for DB and TS overflows.
    */
   void Clock();
	
   /** @brief Readout a hit from the double column. 
    * 
    * @param TS : time stamp
    * @return bool : true if readout continues, false if done
    * 
    * The ROC class asks the double column whether there is something
    * to be read out for the given time stamp. Returns false if nothing.
    */
   bool Readout(long timeStamp, pxhit &hit);

   int GetId() { return Id;};
	
   /** @brief Get statistics
    * 
    * @return Structure describing the hit statistics for this DoubleColumn
    */
   statistics & StatOut() {return stat;};
	
//private:
   /** @brief Receive next hit from column drain
    * 
    * @return false for last hit in event, true otherwise
    * 
    * Do the column drain. Copy pixel hit into data buffer and delete it from pixel array
    */
   bool GetNextPxHit();			     // receive next hit from column drain
	
   /** @brief Set time stamp
    * 
    * @param &timeStamp : long time stamp
    * @return int : 0=ok, -1=TS full, -2=DB full, -3=column drain busy
    * 
    * Insert the time stamp timeStamp into TS buffer. Checks if column drain 
    * is busy (3rd hit) or double column is blocked (TS or DB full)
    */	
   int SetTS(long &timeStamp, bool &tg, int &cd);
	
   /** @brief Reset the double column
    *  
    * Send reset to TS and DB buffer. Counts number of hits lost and updates statistics. 
    * Reset double column logic.
    */	
   void Reset();

   pixlist hits;          ///< list of hits in double column waiting for column drain
   pixlist pendinghits;   ///< list of hits in double column with not yet specified time stamp
   DataBuffer DB;         ///< data buffer
   TimeStamp TS;          ///< time stamp buffer

   int Id;                ///< Unique Id of double column within ROC
   int RocId;             ///< Unique Id of ROC the double column belongs to
   int CD_Status;         ///< channel of active column drain
   int CD_Select;         ///< channel for next ColOR
   bool CD_Active;        ///< true if column drain active
   bool RO_Mode;          ///< true if double column blocked while waiting for r/o token
   bool DB_Full;          ///< true for data buffer overflow
   bool TS_Full;          ///< true for time stamp buffer overflow
   bool NewEvent;         ///< Flag for new event
   int TS_OK;             ///< Status of last time stamp (OK=1, dcol blocked = 0, cd busy=-1)
   int n_reset;           ///< Remaining number of LHC clocks to finish a reset request
   long RO_TS;            ///< Time stamp of next event to be read out
   int RO_pix;            ///< Number of pixels to be read out
   long TrueTS;
   int TrueCD;
   bool wtg;
   long *clk;             ///< Bunch crossing counter
   statistics stat;       ///< Structure doing the book keeping
};

#endif /*DOUBLE_COLUMN_H_*/
