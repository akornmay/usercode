#ifndef ROC_H_
#define ROC_H_

#include "DoubleColumn.h"
#include "CommonDefs.h"
#include "Statistics.h"

/** \class ROC
 * Class implementing a readout chip
 * @author Hans-Christian Kaestli
 */ 
class ROC
{
	
typedef std::vector<DoubleColumn> dcol_vector;			///< Vector of double columns
typedef dcol_vector::iterator dcol_iter;				///< Double column iterator


public:
	/** @brief Constructor */
	ROC();
	
	/** @brief Initializing the ROC
	 * 
	 * Sets the token iterator to one past the end of the double column. 
	 * This indicates the end of the last readout.
	 */	
	void Init(int Id, long *clk);
	
	/** @brief Updating the status of the ROC
	 * 
	 * Calls Clock() for all double columns. Needs to be called once per clock cycle. 
	 * Checks for pixels in double columns to be transfered to the readout buffer. 
	 */	
	void Clock();
	
	/** @brief Adding a hit
	 * 
	 * @param &hit : reference to hit structure
	 * 
	 * Calls AddHit() of the corresponding double column. 
	 */
	void AddHit(pxhit &hit);  

   /** @brief Readout a hit from the ROC.
    *
    * @param TS : time stamp
    * @param &hit : reference to pixel to be read out
    * @return bool : indicates whether r/o in this ROC has finished
    *
    * The TBM class asks the ROC containing the internal token
    * (taken care of by the TBM class) for a pixel to be read out, corresponding to the
    * given time stamp TS. Returns true if there are further hits to be read out.
    */
   bool Readout(long TS);

   /** @brief Getting a trigger
    *
    * @param TS : time stamp for triggered event
    *
    * Adds an entry to the internal list of triggers.
    */
   void Trigger(long TS) {triggers.push_back(TS);};

	/** @brief Statistics output
	 * 
	 * Collects statistics of all double columns and writes a summary to stdout.
	 */
	void StatOut(statistics &stat);
	int GetId(){return Id;};
	
//private:
	dcol_vector dcols;				      ///< vector of double columns
   pixlist ReadoutBuffer;              ///< Digital readout buffer in ROC periphery
   std::list<long> triggers;           ///< List of internal triggers (time stamps)

	long int *bx_counter;		         ///< Bunch crossing counter
	int header;					            ///< Remaining time for the transfer of a the ROC header
	dcol_iter token;				         ///< Iterator pointing to the double column where the internal readout token is
	unsigned long int n_hits; 		      ///< Total number of hits in this ROC. For statistics reason only.
   unsigned long int n_hits_ro;        ///< Total number of hits read out. For statistics reason only.
   int Id;							         ///< Index of this ROC
   int n_transfer;                     ///< Remaining time for the transfer of a hit to the internal readout buffer
   long RO_TS;                         ///< Time stamp of ongoing internal token scan
   bool newEvent;                      ///< Indicates the start of a new readout
   bool newDcol;
   bool newChip;
};

	
#endif /*ROC_H_*/
