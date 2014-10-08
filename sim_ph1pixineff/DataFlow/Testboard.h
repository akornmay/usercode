#ifndef Testboard_H_
#define Testboard_H_

#include "ROC.h"
#include <vector>
#include "Event.h"
#include <TRandom3.h>
#include "CommonDefs.h"
#include "Statistics.h"

typedef std::vector<ROC> roc_vector;
typedef roc_vector::iterator roc_iter;


/** \class Testboard
 * Class implementing a RAL Testboard 
 * @author Andreas Kornmayer (idea by Hans-Christian Kaestli)
 */ 
class Testboard
{	
public:
   /** @brief Standard constructor */
   Testboard();

   /** @brief Destructor */
   ~Testboard();

   /** @brief Initializing a readout link 
    *  
    * @param id : Unique Id of the Testboard
    * 
    * Initializes a Testboard
    * Init creates a list of iterators of ROCs controlled by the HRTB
    */	
   void Init(int id);

   /** @brief RESET all the ROCs connected to the Testboard
    *
    * @param first : iterator of first ROC in chain
    * @param last : iterator of last ROC in chain
    * @param id: Testboard id
    *
    * Reset the entire Testboard at the same time
    *
    */
   void Reset();
	
   /** @brief Adding a time stamp 
    * 
    * @param TS : time stamp to be added
    * 
    * HERE I HAVE TO LOOK FOR THE TOKEN_DELAY
    *
    * For each trigger a time stamp is added. When it expires, the 
    * Testboard sends a readout token to the first ROC.
    */	
   void AddTS(long TS);
	
   /** @brief Adding hits
    * 
    * @param &event : reference to event structure
    * 
    * Sends a trigger to the ROCs. Loops over all hits in the event and 
    * calls AddHit() of the corresponding ROC. 
    */
   void AddHits(Event &event);
   
   /** @brief Updating the status of the Testboard
    * 
    * Checks for expired time stamps and creates a trigger. Generates header/trailer
    * and asks the ROCs for hits to be read out.
    */	
   void Clock();

   /** @brief Statistics output
    * 
    * Collects statistics of all ROCs and writes a summary to stdout.
    */
   void StatOut();
   
   /** @brief Get BC where hit occured
    * 
    * Returns 0,1 or 2 corresponding to the BC where the hit should be saved.
    * 
    */
   int GetBC(double phase);
   
	
private:
   std::list<long> triggerStack;              ///< List of pending triggers
   long timeStamps[TELESCOPE_STACK_SIZE];     ///< List of time stamps for which triggers have to be generated when expired
   roc_iter first_token;                      ///< List of ROC iterators
   roc_iter last_token;                       ///< List of ROC iterators
   roc_iter token;                            ///< ROC iterator pointing to where the r/o token is
   int Id;                                    ///< Id of TBM core inside module
   int n_transfer;                            ///< Remaining length for single hit readout
   bool READOUT;                              ///< True if readout in progress
   int gap;                                   ///< Gap in LHC clocks between trailer and next header
   int delay;
   long RO_TS;                                ///< Time stamp of running readout
   int iRead;
   int iWrite;


   TRandom3 * rndPhase;			      ///< Random number generator used to assign bunch crossing
   double * clkDist;			      ///< Distribution used to assign bunch crossing
   roc_vector ROCs;                           ///< Vector of all ROCs per module

public:
   long ro_clocks;                            ///< cumulated duration of readout in LHC clocks
   long ro_pix;                               ///< Number of hits read out
   long int bx_counter;                      ///< Bunch crossing counter
   long RO_start;                             ///< Start time of readout
   int evtsize;                               ///< Size of event for a link
};


#endif /*Testboard_H_*/
