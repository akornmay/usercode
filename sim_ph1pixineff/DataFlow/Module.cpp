#include "Module.h"
#include "TBM.h"
#include "Statistics.h"
#include "CommonDefs.h"
#include "TH1I.h"
#include "TH2I.h"

using namespace std;

long ro_time;

Module::Module()
{
   ROCs.resize(CHIPS_PER_MODULE);
	TBMs.resize(LINKS_PER_MODULE);
}


void Module::Init(int id) {
   Id=id;
	bx_counter=0;
	int rocId=0;
	for(roc_iter iRoc=ROCs.begin(); iRoc!=ROCs.end(); iRoc++){
		iRoc->Init(rocId++, &bx_counter);
	}
   tbm_iter iTbm=TBMs.begin();
   iTbm->Init(ROCs.begin(), ROCs.begin()+CHIPS_PER_LINK[0], 0, &bx_counter);
   if(CHIPS_PER_LINK[1]>0) {
      iTbm++;
      iTbm->Init(ROCs.begin()+CHIPS_PER_LINK[0], ROCs.end(), 1, &bx_counter);
   }
}


Module::~Module()
{
	ROCs.clear();
	TBMs.clear();
}


void Module::AddHits(Event &event)                         // called once per event
{
   if(event.trigger){                                      // this event will be triggered
      for(tbm_iter iTBM=TBMs.begin(); iTBM!=TBMs.end(); iTBM++) iTBM->AddTS(event.clock);
      for (roc_iter iRoc=ROCs.begin(); iRoc!=ROCs.end(); iRoc++) iRoc->Trigger(event.clock);
   }
	hit_iterator hit;
	for(hit=event.hits[Id].begin(); hit!=event.hits[Id].end(); hit++){
	   ROCs[hit->roc].AddHit(*hit);
	}
}


void Module::Clock()
{
   bx_counter++;
   for(tbm_iter iTBM=TBMs.begin(); iTBM!=TBMs.end(); iTBM++) iTBM->Clock();
   for(roc_iter iRoc=ROCs.begin(); iRoc!=ROCs.end(); iRoc++) iRoc->Clock();
}


void Module::StatOut()
{
	char txt[50];
	statistics stat[CHIPS_PER_MODULE]; 
	statistics mod_stat;
	mod_stat.Reset();
	int i=0;
	for(int i=0; i<CHIPS_PER_MODULE; i++) {
	   cout << endl<<"**********************************************************"<<endl<<endl;
		ROCs[i].StatOut(stat[i]);
		sprintf(txt, "Roc number %d",i);
		stat[i].PrintStat(txt);
		mod_stat+=stat[i];
	}
	cout << endl<<endl<<"**********************************************************"<<endl<<endl;
	sprintf(txt, "Module %d",Id);
	mod_stat.PrintStat(txt);

   i=1;
   cout <<endl;

   for(tbm_iter iTBM=TBMs.begin(); iTBM!=TBMs.end(); iTBM++){
      cout << "Link number "<<i++<<":"<<endl;
      cout << "     Pixel readout rate = "
                           <<(double)iTBM->ro_pix/((double) MAX_EVENT*25e-3)<<" MPix/s"<<endl;
      cout << "     Occupancy =          "
                           <<((double)iTBM->ro_clocks/(double) MAX_EVENT)*100<<" %"<<endl;
   }

   cout << endl;

}
