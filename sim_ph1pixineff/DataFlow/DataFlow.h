#ifndef DATAFLOW_H_
#define DATAFLOW_H_

#include <TRandom3.h>
#include <TFile.h>
#include <TH1I.h>
#include <TH1F.h>
#include <TH2I.h>
#include <TProfile.h>
#include <TString.h>
#include "Testboard.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include "Event.h"
#include "EventReader.h"
#include <map>
#include <list>

//
// general settings
//

long MAX_EVENT;                                 // #events to be processed
long MAX_TRIGGER;
double TRIGGER_RATE;                            // L1 trigger rate in kHz
double SIGNAL_XSECTION;                         // signal (jet) X-section in mb
double TOTAL_XSECTION;                          // MinBias X-section in mb
double PEAK_LUMI;                               // in 10^34
long WBC;                                       // trigger latency
int THRESHOLD;                                  // pixelThreshold calculated in DACs and compared agains pulseHeight
int LAYER;
int MIN_MOD, MAX_MOD;
int DETECTOR;
int LADDER;
std::string SignalFileName;
std::list<std::string> MinBiasFileNames;
bool CreatePileUp;
int BUNCH_SPACING;

bool PIXELTIMING;
double PIX_SIGMA;				//Spread of pixel clock (in ns)
double DET_SPACING;				//distance between det for phase assignment

bool SAVE_TREE;

//
//HRBT settings
//
int TOKEN_DELAY;
int TRIGGER_BUCKET;
int RESETINTERVAL;
//
// QIE readings
//

double BEAMINTENSITY_PARAM1;
double BEAMINTENSITY_PARAM2;
double BEAMINTENSITY_SCALING;
std::string QIEfileName;



//
// method declarations
//

bool GetMBHit(Event &event);
bool GetSignalHit(Event &event);
void Init(bool* EmptyBC);
void ReadSettings(char* fileName);
void FillHisto(hit_vector &hits, int trigger);
void WriteHits();
bool main_phaseOK(double phase);
//
// variable declarations
//

const std::string SOFTWARE_VERSION="V2.0   23-Aug-2012   no TAG";
Event event;
Event * nextEvent;
Event * nextNextEvent;
Event eventToProcess;
int layer,module[4];
long clk;
ofstream hitFile;
TFile *histoFile;
TH1 *h1, *h2, *h3, *h4, *h5, *h6, *h7, *h8;
TH1I *g1, *g2, *g3, *g4, *g5;
TH1I *rodelay, *rotime, *eventsize;
TH2I *allhits;
TH2I *ineffhits;
TH2I *allclusters_ev , *effclusters_ev;
TH2I *ExtTokenWait, *IntTokenWait, *DcolTokenWait, *DCTokenWait;
TH1I *DBSize, *TSSize;
TProfile *eff, *effflux, *effhitrate, *effflux_hits;
TProfile *inefftype[6];
bool WriteHisto;
TString HistoFileName;




//Related to data saving :

void initSave();
void endSave();
void saveHits(hit_vector * hits);
void saveHit(pxhit * hit);
void saveTransparentHit(pxhit hit);

TTree * pixTree;					//Tree containing all detected hits
TFile * pixFile;					//File containing previous tree

TTree * pixTransparentTree;					//Tree containing all detected hits
TFile * pixTransparentFile;					//File containing previous tree

string PIX_TREE_FILE;					//Name of the file where data will be saved

/** \struct pixStruct
 *  Structure used to save hits to TTree
 */


struct pixStruct{
  unsigned int event_number;
  
  int roc;
  int mycol;
  int myrow;
  int vcal;
  double pulseHeight;
  double phase;
  
  long TS;
  unsigned int trigger_number;
  unsigned int token_number;
  char triggers_stacked;
  char trigger_phase;
  char data_phase;
  char status;
};

pixStruct pStruct;

#endif /*DATAFLOW_H_*/
