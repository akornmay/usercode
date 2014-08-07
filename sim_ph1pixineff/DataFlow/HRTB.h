#ifndef HRTB_H_
#define HRTB_H_

#include "ROC.h"
#include <vector>
#include "Event.h"

typedef std::vector<ROC> roc_vector;
typedef roc_vector::iterator roc_iter;


/** \class HRTB
 * Class implementing a HRTB 
 * @author Andreas Kornmayer (idea by Hans-Christian Kaestli)
 */ 
class HRTB
{	
public:
   /** @brief Standard constructor */
   HRTB();

   /** @brief Initializing a readout link 
    * 
    * @param first : iterator of first ROC in chain
    * @param last : iterator of last ROC in chain
    * @param id : Unique Id of HRTB
    * @param *bx : pointer to bunch crossing counter
    * 
    * Initializes a HRTB
    * Init creates a list of iterators of ROCs controlled by the HRTB
    */	
   void Init(roc_iter first, roc_iter last, int id, long *bx);
	
   /** @brief Adding a time stamp 
    * 
    * @param TS : time stamp to be added
    * 
    * HERE I HAVE TO LOOK FOR THE TOKEN_DELAY
    *
    * For each trigger a time stamp is added to the HRTB. When it expires, the 
    * HRTB sends a readout token to the first ROC.
    */	
   void AddTS(long TS);
	
   /** @brief Updating the status of the HRBT
    * 
    * Checks for expired time stamps and creates a trigger. Generates header/trailer
    * and asks the ROCs for hits to be read out.
    */	
   void Clock();
	
private:
   std::list<long> triggerStack;              ///< List of pending triggers
   long timeStamps[TELESCOPE_STACK_SIZE];           ///< List of time stamps for which triggers have to be generated when expired
   roc_iter first_token;                      ///< List of ROC iterators
   roc_iter last_token;                       ///< List of ROC iterators
   roc_iter token;                            ///< ROC iterator pointing to where the r/o token is
   int Id;                                    ///< Id of TBM core inside module
   int header;                                ///< Remaining length of TBM header in clocks
   int trailer;                               ///< Remaining length of TBM trailer in clocks
   int n_transfer;                            ///< Remaining length for single hit readout
   bool READOUT;                              ///< True if readout in progress
   int gap;                                   ///< Gap in LHC clocks between trailer and next header
   long RO_TS;                                ///< Time stamp of running readout
   int iRead;
   int iWrite;

public:
   long ro_clocks;                            ///< cumulated duration of readout in LHC clocks
   long ro_pix;                               ///< Number of hits read out
   long int *bx_counter;                      ///< Bunch crossing counter
   long RO_start;                             ///< Start time of readout
   int evtsize;                               ///< Size of event for a link
};


#endif /*HRTB_H_*/
