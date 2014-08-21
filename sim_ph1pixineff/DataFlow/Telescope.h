#ifndef TELESCOPE_H_
#define TELESCOPE_H_

#include "ROC.h"
#include "HRTB.h"
#include "CommonDefs.h"
#include "Statistics.h"
#include "Event.h"
#include <vector>
#include <list>
#include <TRandom3.h>


typedef std::vector<ROC> roc_vector;
typedef roc_vector::iterator roc_iter;
typedef std::vector<HRTB> hrtb_vector;
typedef hrtb_vector::iterator hrtb_iter;

/** \class Telescope
 * Class implementing a Telescope
 * @author Andreas Kornmayer (idea by Hans-Christian Kaestli)
 */ 
class Telescope{
public:
	/** @brief Constructor */
	Telescope();
	
	/** @brief Destructor */
	~Telescope();
	
	/** @brief Adding hits
	 * 
	 * @param &event : reference to event structure
	 * 
	 * Sends a trigger to the HRTBs and ROCs. Loops over all hits in the event and 
	 * calls AddHit() of the corresponding ROC. 
	 */
	void AddHits(Event &event);
	
	/** @brief Updating the status of the Telescope
	 * 
	 * Calls Clock() for all ROCs and HRTBs. Needs to be called exactly once per main event loop.
	 */	
	void Clock();
	
	/** @brief Initializing the Telescope
	 * 
	 * @param id : Telescope Id
	 *
	 * Set up of TBMs and ROCs. One Telescope is corresponding to one data link.
	 * Calls Init() method of TBMs and ROCs.
	 */		
	void Init(int id);
	

	/** @brief RESET the telescope
	 *
	 * Reset the entire Telescope at the same time
	 *
	 */
	void Reset();




	/** @brief Statistics output
	 * 
	 * Collects statistics of all ROCs and writes a summary to stdout.
	 */
	void StatOut();


	

	/**
	 * @brief Get BC where hit occured
	 * 
	 * Returns 0,1 or 2 corresponding to the BC where the hit should be saved.
	 * 
	 */
	int GetBC(double phase);
	


private:
	TRandom3 * rndPhase;			  ///< Random number generator used to assign bunch crossing
	double * clkDist;			  ///< Distribution used to assign bunch crossing
	roc_vector ROCs;                          ///< Vector of all ROCs per module
	hrtb_vector HRTBs;                          ///< Vector of all TBM cores per module
	long int bx_counter;                      ///< Bunch crossing counter
	int Id;                                   ///< Id of module in layer/blade
};

#endif /*TELESCOPE_H_*/
