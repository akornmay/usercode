#ifndef EVENTREADER_H_
#define EVENTREADER_H_

#include "TRandom3.h"
#include "TFile.h"
#include "TTree.h"
#include "CommonDefs.h"
#include <list>

class RootHits
{
public:
	~RootHits();
	void Init(int lr, std::string &name);
	void GetHits(Event &event, int nEvents);
private:
	int vcal;
	short int col, row, adc;
	int event_nr;
	float flux;
	int ladder, layer;
	unsigned int tree_event;
	short tree_ladder, tree_module, panel;
	UInt_t N_Entries;
	UInt_t rPointer;
	TFile *HitFile;
	TTree *HitTree;
};

class RootReader
{
public:	
	void Init(int layer);
	void ReadEvent(Event &event);
	
private:
	std::list<RootHits *> signalTrees, MB_Trees;
	TRandom3 rndm;

	double E_PileUp;						// expectation value for number of pile up events
	double E_Signal;						// expectation value for number of signal events
	std::list<RootHits *>::iterator iSignal, iMinBias;
	
};

class ASCIIReader
{
public:
	void Init(int layer);
	void ReadEvent(Event &event);
	void Terminate();	
private:
	ifstream is[4];
};

#endif /*EVENTREADER_H_*/
