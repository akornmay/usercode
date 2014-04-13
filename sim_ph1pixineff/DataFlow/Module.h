#ifndef MODULE_H_
#define MODULE_H_

#include "ROC.h"
#include "TBM.h"
#include "CommonDefs.h"
#include "Statistics.h"
#include "Event.h"
#include <vector>
#include <list>

typedef std::vector<ROC> roc_vector;
typedef roc_vector::iterator roc_iter;
typedef std::vector<TBM> tbm_vector;
typedef tbm_vector::iterator tbm_iter;

/** \class Module
 * Class implementing a BPIX (half-)module or a FPIX pannel
 * @author Hans-Christian Kaestli
 */ 
class Module{
public:
	/** @brief Constructor */
	Module();
	
	/** @brief Destructor */
	~Module();
	
	/** @brief Adding hits
	 * 
	 * @param &event : reference to event structure
	 * 
	 * Sends a trigger to the TBMs and ROCs. Loops over all hits in the event and 
	 * calls AddHit() of the corresponding ROC. 
	 */
	void AddHits(Event &event);
	
	/** @brief Updating the status of the module
	 * 
	 * Calls Clock() for all ROCs and TBMs. Needs to be called exactly once per main event loop.
	 */	
	void Clock();
	
	/** @brief Initializing the module
	 * 
	 * @param id : Module Id
	 *
	 * Set up of TBMs and ROCs. One Module is corresponding to one data link.
	 * Calls Init() method of TBMs and ROCs.
	 */		
	void Init(int id);
	
	/** @brief Statistics output
	 * 
	 * Collects statistics of all ROCs and writes a summary to stdout.
	 */
	void StatOut();
	

private:
	 roc_vector ROCs;                          ///< Vector of all ROCs per module
	 tbm_vector TBMs;                          ///< Vector of all TBM cores per module
    long int bx_counter;                      ///< Bunch crossing counter
    int Id;                                   ///< Id of module in layer/blade
};

#endif /*MODULE_H_*/
