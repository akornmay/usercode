#ifndef COMMON_DEFS_H_
#define COMMON_DEFS_H_

#include "Event.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <list>
#include "TTree.h"
#include "TFile.h"

//
// general settings
//

extern long MAX_EVENT;                                  // #events to be processed
extern double TRIGGER_RATE;                             // L1 trigger rate in kHz
extern double SIGNAL_XSECTION;                          // signal (jet) X-section in mb
extern double TOTAL_XSECTION;                           // MinBias X-section in mb
extern double PEAK_LUMI;                                // in 10^34
extern int BUNCH_SPACING;                               // spacing between filled LHC bunches
extern long WBC;                                        // trigger latency
extern int LAYER;
extern int DETECTOR;
extern int LADDER;
extern int MIN_MOD, MAX_MOD;
extern std::list<std::string> SignalFileNames;
extern std::list<std::string> MinBiasFileNames;
extern bool CreatePileUp;
extern int ALL_BUNCHES_FILLED;                          // 1: all bunches are filled; 0: only certain bunches are filled
extern double PIX_SIGMA;				//Spread of pixel clock (in ns)
extern double DET_SPACING;				//distance between det for phase assignment

extern bool SAVE_TREE;
extern void saveHits(hit_vector * hits);
extern void saveHit(pxhit * hit);

extern std::string PIX_TREE_FILE;

const int MINIMAL_TRIGGER_GAP(3);                       // minimal gap between triggers (3 BX)

//
// module and ROC settings
//

extern int CHIPS_PER_LINK[2];
extern int CHIPS_PER_MODULE;
extern int LINKS_PER_MODULE;
const unsigned int TBM_STACK_SIZE(16);                  // size of trigger stack in TBM
const int DCOLS_PER_ROC(26);
const unsigned int TS_BUFFER_SIZE(24);
const unsigned int DATA_BUFFER_SIZE(80);
const unsigned int ROC_BUFFER_SIZE(64);
const int CONVERSION_TIME(4);

//
//HRBT settings
//
extern int TOKEN_DELAY;
const int NUMBER_OF_TELESCOPES(1);
const int CHIPS_PER_TELESCOPE(8);
const int LINKS_PER_TELESCOPE(1);
const unsigned int TELESCOPE_STACK_SIZE(16);

extern int TRIGGER_BUCKET;                              //Nuber of the bucket in each turn the trigger is send on

//
// readout format
//

const int TELESCOPE_HEADER_LENGTH(0);//(16);
const int TELESCOPE_TRAILER_LENGTH(0);//(15);
const int TELESCOPE_READOUT_GAP(0);


const int TBM_HEADER_LENGTH(8);                         // TBM header length in LHC clocks
const int TBM_TRAILER_LENGTH(8);                        // TBM trailer length in LHC clocks
const int TBM_READOUT_GAP(1);                           // gap between TBM trailer and next header in LHC clocks
const int ROC_HEADER_LENGTH(3);                         // ROC header length in bits
const int CLOCKS_PER_HIT(6);                            // #clocks per pixel hit

enum {BPIX=0, FPIX};

#endif /*COMMON_DEFS_H_*/
