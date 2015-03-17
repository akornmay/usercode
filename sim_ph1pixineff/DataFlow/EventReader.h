#ifndef EVENTREADER_H_
#define EVENTREADER_H_

#include "TRandom3.h"
#include "TFile.h"
#include "TTree.h"
#include "CommonDefs.h"
#include <list>

// this part is especially for the telescope simulation


class TelescopeHits
{
public:
	~TelescopeHits();
	void Init(std::string &name);
	void InitQIE(std::string &name);
	void GetHits(Event &event, int nEvents);
private:
	int nTrig;
	
	int vcal;
	int col, row;
	double adc;
	int roc;
	int event_nr;
	float flux;
	unsigned int tree_event;
	UInt_t N_Entries;
	UInt_t rPointer;
	TFile *HitFile;
	TTree *HitTree;

	TRandom3* randomNo;


};

class TelescopeReader
{
 public:
  void Init();
  void InitQIE(std::string &name);
  void ReadEvent(Event &event);
  int NumberOfParticles(double QIEintensity, double pedestal);
 private:

  TFile* QIEFile;
  TTree* QIETree;
  float BeamIntensity[588];
  int Turn;
  int Bucket;
  double MaxNoTracks;
  long int QIEeventcounter;

  TelescopeHits* signalTree;

  std::list<TelescopeHits *>::iterator iSignal;

};







#endif /*EVENTREADER_H_*/
