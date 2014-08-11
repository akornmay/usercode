#include "Telescope.h"
#include "HRTB.h"
#include "Statistics.h"
#include "CommonDefs.h"
#include "TH1I.h"
#include "TH2I.h"
#include "TMath.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TGraph.h"
using namespace std;

//long ro_time;

Telescope::Telescope()
{
   ROCs.resize(CHIPS_PER_TELESCOPE);
   HRTBs.resize(LINKS_PER_TELESCOPE);
   clkDist = new double[2501];
   
   TF1 * clkFunc = new TF1("clkFunc","0.5*(TMath::Erf((x-[0])/(sqrt(2)*[1]))-TMath::Erf((x-[2])/(sqrt(2)*[1])))",0,25);
   clkFunc->SetParameters(-12.5,PIX_SIGMA,12.5);
   for(int i=0; i<2501; i++){
       clkDist[i]=(*clkFunc)(i/100.);
   }
   rndPhase=new TRandom3(46644);
   delete clkFunc;

}


void Telescope::Init(int id) {
   Id=id;
	bx_counter=0;
	int rocId=0;
	for(roc_iter iRoc=ROCs.begin(); iRoc!=ROCs.end(); iRoc++){
		iRoc->Init(rocId++, &bx_counter);
	}
   hrtb_iter iHrtb=HRTBs.begin();
   iHrtb->Init(ROCs.begin(), ROCs.begin()+CHIPS_PER_TELESCOPE, 0, &bx_counter);
}


Telescope::~Telescope()
{
	ROCs.clear();
	HRTBs.clear();
}


void Telescope::AddHits(Event &event)                         // called once per event
{
   if(event.trigger){                                      // this event will be triggered
      for(hrtb_iter iHRTB=HRTBs.begin(); iHRTB!=HRTBs.end(); iHRTB++) iHRTB->AddTS(event.clock);
      for (roc_iter iRoc=ROCs.begin(); iRoc!=ROCs.end(); iRoc++) iRoc->Trigger(event.clock);
   }
	hit_iterator hit;
	for(hit=event.hits[Id].begin(); hit!=event.hits[Id].end(); hit++){
	   ROCs[hit->roc].AddHit(*hit);
	}
}


void Telescope::Clock()
{
   bx_counter++;
   for(hrtb_iter iHRTB=HRTBs.begin(); iHRTB!=HRTBs.end(); iHRTB++) iHRTB->Clock();
   for(roc_iter iRoc=ROCs.begin(); iRoc!=ROCs.end(); iRoc++) iRoc->Clock();
}


void Telescope::StatOut()
{
	char txt[50];
	//	statistics stat[CHIPS_PER_MODULE]; 
	statistics stat[8];
	statistics mod_stat;
	mod_stat.Reset();
	int i=0;
	for(int i=0; i<CHIPS_PER_TELESCOPE; i++) {
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

   for(hrtb_iter iHRTB=HRTBs.begin(); iHRTB!=HRTBs.end(); iHRTB++){
      cout << "Link number "<<i++<<":"<<endl;
      cout << "     Pixel readout rate = "
                           <<(double)iHRTB->ro_pix/((double) MAX_EVENT*25e-3)<<" MPix/s"<<endl;
      cout << "     Occupancy =          "
                           <<((double)iHRTB->ro_clocks/(double) MAX_EVENT)*100<<" %"<<endl;
   }

   cout << endl;

}

int Telescope::GetBC(double phase)
{
    int toReturn=0;
    if(phase<0){
	phase+=25;
	toReturn--;
    }
    if(phase>25){
	phase-=25;
	toReturn++;
    }
    if(phase>25 || phase <0){cerr<<"wrong phase ("<<phase<<") !"<<endl; return(0);}
    double p=clkDist[(int)(phase*100)];
    if(p>1-1e-6){return(toReturn);}
    if(p<1e-6){return(toReturn+1);}
    if(rndPhase->Rndm()>p){return(toReturn+1);}
    else{return(toReturn);}
}
