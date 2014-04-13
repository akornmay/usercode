#ifndef TBM_H_
#define TBM_H_

#include "ROC.h"
#include <vector>
#include "Event.h"

typedef std::vector<ROC> roc_vector;
typedef roc_vector::iterator roc_iter;


/** \class TBM
 * Class implementing a TBM core which corresponds to a readout link
 * @author Hans-Christian Kaestli
 */ 
class TBM
{	
public:
   /** @brief Standard constructor */
   TBM();

   /** @brief Initializing a readout link 
    * 
    * @param first : iterator of first ROC in chain
    * @param last : iterator of last ROC in chain
    * @param id : Unique Id of TBM core
    * @param *bx : pointer to bunch crossing counter
    * 
    * Initializes a TBM core. A TBM core corresponds to one readout link.
    * A real TBM operating in dual mode corresponds to two instances
    * of the TBM class.
    * Init creates a list of iterators of ROCs controlled by the TBM.  
    */	
   void Init(roc_iter first, roc_iter last, int id, long *bx);
	
   /** @brief Adding a time stamp 
    * 
    * @param TS : time stamp to be added
    * 
    * For each trigger a time stamp is added to the TBM. When it expires, the 
    * TBM sends a readout token to the first ROC.
    */	
   void AddTS(long TS);
	
   /** @brief Updating the status of the link
    * 
    * Checks for expired time stamps and creates a trigger. Generates header/trailer
    * and asks the ROCs for hits to be read out.
    */	
   void Clock();
	
private:
   std::list<long> triggerStack;              ///< List of pending triggers
   long timeStamps[TBM_STACK_SIZE];           ///< List of time stamps for which triggers have to be generated when expired
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


#endif /*TBM_H_*/
