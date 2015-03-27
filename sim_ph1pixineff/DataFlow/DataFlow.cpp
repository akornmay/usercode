#include "DataFlow.h"
#include "CommonDefs.h"
#include "TCanvas.h"
#include <cstdlib>
#include <vector>

using namespace std;

int main(int argc, char **argv)
{
   WriteHisto=false;
	
   char inName[100];			            // read name of steering file from stdin
   if (argc<2) {
     	inName[0]='\0';
   } else strcpy(inName,argv[1]); 
   ReadSettings(inName);		         // read steering file
	
   double p_trig=(double)(TRIGGER_RATE)/40000.;  // probability for trigger per clock
   p_trig*=(double)BUNCH_SPACING/25.0;
   int last_trigger=0;
   int ntrig=0;				            // count total number of triggers sent
   int blockedtriggers = 0;                      //count of triggers blocked by trigger rules
   int event_number = 0;
   TRandom3 rndm(93657);		         // random number generator

   layer = LAYER;			               // analyze layer number LAYER as specified in steering file
   bool EmptyBC[3564];			         // LHC bunch structure (true=empty, false=filled)
   Init(EmptyBC);			               // open hit files and create LHC bunch structure


   std::vector<Testboard> Testboards;
   std::vector<Testboard>::iterator iTB;
   Testboards.resize(NUMBER_OF_TELESCOPES);
   int j = 0;
   for(iTB=Testboards.begin(); iTB!=Testboards.end(); iTB++)
     {
       iTB->Init(j++);
     }
   printf("we have %i Telescopes\n",j);

   //   RootReader EventReader;
   //   EventReader.Init(layer);

   TelescopeReader EventReader;
   EventReader.Init();


   long unsigned int bucketcounter = 0;
   long unsigned int bxcounter = 0;
   long unsigned int resetcounter = 0;

   double phase=0;					//Phase between Fermi and LHC clock
   bool newBC=false;
   int old_trig=0;
   
   nextEvent = new Event;
   nextEvent->New(2,false);
   nextNextEvent = new Event;
   nextNextEvent->New(3,false);
   event.New(1,false);
   
   if(SAVE_TREE)initSave();
// *****************************************************************************************************
//                   main loop over bunch crossings
// *****************************************************************************************************	
   
   
   
   for(clk=1; clk< MAX_EVENT; clk++){
     if(clk<50000 && !(clk %5000)) cout <<"Processing event number "<< clk << " ....."<<endl;
     else if(clk<100000 && !(clk %10000)) cout <<"Processing event number "<< clk << " ....."<<endl;
     else if(!(clk %50000)) cout <<"Processing event number "<< clk <<" ("<<clk*100./MAX_EVENT<<"%) ....."<<endl;
     if(ntrig == MAX_TRIGGER)
       {
	 cout<<"Maximum number of triggers reached. Terminating Simulation!" << endl;
	 break;
       }
     if(newBC){
       bxcounter++;
       //	printf("Tick\n");
// 	cout<<"NEW BC. Total hits : "<<event.hits[MIN_MOD-1].size() <<endl;
// 	cout<<"NEW BC. Next hits : "<<(*nextEvent).hits[MIN_MOD-1].size() <<endl;
// 	cout<<"NEW BC. Next² hits : "<<(*nextNextEvent).hits[MIN_MOD-1].size() <<endl;
	/////////////////////////////////////////////////////////////////////////
	////////////PROCESSING EVENT FROM PREVIOUS BUNCH CROSSING ///////////////
	/////////////////////////////////////////////////////////////////////////


	
	//all clusters
	event.clusterize( false );
	//only efficient clusters
	//	event.clusterize( true );
	eff->Fill( event.clustersall.size() , event.getInefficiency() ); 
	effflux->Fill( event.flux , event.getInefficiency() ); 
	effflux_hits->Fill( event.flux , event.getInefficiencyHit() ); 
	effhitrate->Fill( event.clustersall.size() / 0.64 * 40. , event.getInefficiency() ); 
	//fill reasons
	int reasons[6];
	for (int x = 0; x < 6; x++ )
	    reasons[x] = 0;

	int whichvec = -1;
	
	for ( int x = 0; x < 4; x++ )
	    if ( event.hits[x].size() > 0 )
	    {
		whichvec = x;
		break;
	    }
	if (whichvec>-1){
	  for (int x = 0; x < event.hits[whichvec].size(); x++ )
	    reasons[ event.hits[whichvec][x].inefftype ]++;
	  for (int x = 0; x < 6; x++ )
	    inefftype[x]->Fill(event.flux , reasons[x] / event.hits[whichvec].size() );
	}
	
	if (clk == 102 )
	    {
	    for (int c=0; c<event.clustersall.size(); c++)
		{
		for (int px = 0; px < event.clustersall[c].size(); px++)
		    {	    
		    pxhit thishit = event.clustersall[c].at(px);
		    allclusters_ev->Fill( thishit.mycol , thishit.myrow , c+1 );
		    }
		}

	    for (int c=0; c<event.clustersafter.size(); c++)
		{
		for (int px = 0; px < event.clustersafter[c].size(); px++)
		    {	    
		    pxhit thishit = event.clustersafter[c].at(px);
		    effclusters_ev->Fill( thishit.mycol , thishit.myrow , c+1 );
		    }
		}

	    }
	if(WriteHisto){
	  for(int i=0; i<NUMBER_OF_TELESCOPES; i++)    FillHisto(event.hits[i], old_trig);
	}
		
// 	if(event.trigger && SAVE_TREE){
// 		for(int i=MIN_MOD-1; i<MAX_MOD; i++)    saveHits(&event.hits[i]);		    
// 	}
	iTB=Testboards.begin();
	for(; iTB!=Testboards.end(); iTB++) iTB->AddHits(event);  // add hits to telescope

	iTB=Testboards.begin();
	for(; iTB!=Testboards.end(); iTB++) iTB->Clock();    // advance clock in telescope

	
	////////////////////////////////////////////////////////
	///////////Generating next (empty) event////////////////
	////////////////////////////////////////////////////////
	
	
	if(last_trigger>0) last_trigger--;                                                   // two triggers cannot be within MINIMAL_TRIGGER_GAP clocks
	int trigger=0;                                                                      //we need to keep the minimal trigger gap to prevent double triggering a bucket


	if(last_trigger==0 && (bucketcounter%588==TRIGGER_BUCKET || bucketcounter%588==(TRIGGER_BUCKET+1)))           // this event will be triggered at a specific bucket number
	  {	  
	    trigger=1;
	    ntrig++;

	    if(ntrig%RESETINTERVAL == 0) //don't send a trigger, instread send a reset
	      {
	
		trigger =0;
		resetcounter++;
		
		iTB=Testboards.begin();
		for(; iTB!=Testboards.end(); iTB++) 
		  {
		    iTB->Reset();  // reset telescope
		  }
	      }

	    last_trigger=MINIMAL_TRIGGER_GAP;
	    //		    printf("Trigger #%i send\n", triggercounter);
	  }
	

	
	event.New(clk, trigger);			     // initialize new event
	
	////////////////////////////////////////////////////////
	////////Moving 'next' to evt and 'next²' to next////////
	////////////////////////////////////////////////////////
	for(int i=0; i<NUMBER_OF_TELESCOPES; i++){
	    for(int j=0; j<nextEvent->hits[i].size(); j++){
		nextEvent->hits[i][j].timeStamp=event.clock;
		nextEvent->hits[i][j].trigger=event.trigger;
		event.hits[i].push_back(nextEvent->hits[i][j]);			//Add previous "next event" to new event
	    }
	}
	   
	delete nextEvent;
	nextEvent = nextNextEvent;
	nextNextEvent = new Event;
	nextNextEvent->New(clk+2,false);					//Generating event for next clk.
      }
     bucketcounter++;
     eventToProcess.New(clk,0);
     EventReader.ReadEvent(eventToProcess);		                 // read hits from input file(s)
     //       cout<<"eventToProcess.hits[MIN_MOD-1].size()="<<eventToProcess.hits[MIN_MOD-1].size()<<endl;
     int nHits=eventToProcess.hits[0].size();
     double * hPhase= new double[nHits];
     for(int i=0; i<nHits; i++){hPhase[i]=phase+rndm.Gaus(0,1.88);}
     for (int j=0; j<NUMBER_OF_TELESCOPES; j++){					//Sort in BC from phase
       int nHitsToProcess = nHits;
       for(int i=nHits-1; i>-1; i--){
		    double hp = hPhase[i]+j*DET_SPACING*1./29.98;
		    double ep = phase+j*DET_SPACING*1./29.98;
		    int BC_sort=Testboards[0].GetBC(hp);
		    if(BC_sort==0){
			eventToProcess.hits[j][i].phase=hp+12.5;				//Save phase for hit
			eventToProcess.hits[j][i].evtPhase=ep+12.5;
			eventToProcess.hits[j][i].timeStamp=event.clock;
			eventToProcess.hits[j][i].trigger=event.trigger;
			event.hits[j].push_back(eventToProcess.hits[j][i]);			//Change phase and move to next BC
			eventToProcess.hits[j].erase(eventToProcess.hits[j].begin()+i);

		    }
		    else if(BC_sort==1){
			    eventToProcess.hits[j][i].phase=hp-12.5;
			    eventToProcess.hits[j][i].evtPhase=ep-12.5;
			    nextEvent->hits[j].push_back(eventToProcess.hits[j][i]);		//Change phase and move to next BC
			    eventToProcess.hits[j].erase(eventToProcess.hits[j].begin()+i);
		    }
		    else if(BC_sort==2){
			    eventToProcess.hits[j][i].phase=hp-37.5;
			    eventToProcess.hits[j][i].evtPhase=ep-37.5;
			    nextNextEvent->hits[j].push_back(eventToProcess.hits[j][i]);		//Change phase and move to next/next BC
			    eventToProcess.hits[j].erase(eventToProcess.hits[j].begin()+i);
		    }
		    else{
			
			    cerr<<"Bad event #BC = "<<BC_sort<<" ignored"<<endl;   
			    eventToProcess.hits[j].erase(eventToProcess.hits[j].begin()+i);
		    }
	}
      }
      delete hPhase;
     // Move clock
     phase+=18.8;
      if(phase>25){
	newBC=true;
	phase-=25;
      }
      else{
	newBC=false;
	clk--;
      }
   }					       	                                // end of main loop
   if(SAVE_TREE)endSave();
// *****************************************************************************************************
//                        Statistics output
// *****************************************************************************************************	
	
	
   cout << "**********************************************"<<endl;
   cout << "*            Statistics output               *"<<endl;
   cout << "**********************************************"<<endl<< endl;
   iTB=Testboards.begin();
   for(; iTB!=Testboards.end(); iTB++) iTB->StatOut();
   cout << "Time simulated:           "<<MAX_EVENT*25e-6<<" ms"<<endl;
   cout << "Total number of triggers: "<<ntrig <<"  ,  rate= "
	<<(double)ntrig/(double)bxcounter*40000<<" kHz"<<endl;
   cout <<"Total number of buckets " << bucketcounter << endl;
   cout <<"Total number of blocked triggers " << blockedtriggers << endl;
   cout <<"Total number of RESETS issued by test board " << resetcounter << endl;

   if(WriteHisto){
	   histoFile->cd();
	   h1->Write();
	   h2->Write();
	   h3->Write();
	   h4->Write();
	   h5->Write();
	   h6->Write();
	   h7->Write();
	   h8->Write();
	   g1->Write();
	   g2->Write();
	   g3->Write();
	   g4->Write();
	   g5->Write();
	   allhits->Write();
	   ineffhits->Write();
	   allclusters_ev->Write();
	   effclusters_ev->Write();
	   eff->Write();
	   inefftype[0]->Write();
	   inefftype[1]->Write();
	   inefftype[2]->Write();
	   inefftype[3]->Write();
	   inefftype[4]->Write();
	   inefftype[5]->Write();
	   effflux->Write();
	   effflux_hits->Write();
	   effhitrate->Write();
	   printf("ENTRIES:::: %i\n" , (int)ineffhits->GetEntries());
	   rotime->Write();
	   rodelay->Write();
	   eventsize->Write();
	   IntTokenWait->Write();
	   ExtTokenWait->Write();
	   DcolTokenWait->Write();
	   DCTokenWait->Write();

	   TSSize->Write();
	   DBSize->Write();
	   histoFile->Write();
	   delete h1;
	   delete h2;
	   delete h3;
	   delete h4;
	   delete h5;
	   delete h6;
	   delete h7;
	   delete h8;
	   delete g1;
	   delete g2;
	   delete g3;
	   delete g4;
	   delete g5;
	   delete rotime;
	   delete rodelay;
	   delete eventsize;
	   histoFile->Close();
	   delete histoFile;
   }
   return(0);
}


void Init(bool* EmptyBC)
{
	int PSbatch[]={0,80,
			190,270,350,
			460, 540, 620, 700,
			811, 891, 971,
			1081, 1161, 1241,
			1351, 1431,  1511, 1591,
			1702, 1782, 1862,
			1972, 2052, 2132,
			2242, 2322, 2402, 2482,
			2593, 2673, 2753,
			2863, 2943, 3023,
			3133, 3213, 3293, 3373
	};

	if ( ALL_BUNCHES_FILLED == 1 )
	  for(int i=1; i<3564; i++) EmptyBC[i]=false;
	else
	  {
	    for(int i=0; i<3564; i++) EmptyBC[i]=true;
	    
	    for(int j=0; j<39; j++) {
	      for(int i=0; i<72; i++) {
		EmptyBC[PSbatch[j]+i]=false;
	      }
	    }
	  }
	int j=BUNCH_SPACING/25;
	for(int i=1; i<3564; i++) if( (i%j) !=0) EmptyBC[i]=true;

   if(WriteHisto) {
      histoFile = new TFile(HistoFileName,"RECREATE");
      h1=new TH1I("px_per_mod","Pixels per hit module",401,-0.5,400.5);	
      h2=new TH1I("px_per_roc","Pixels per hit ROC",81,-0.5,80.5);	
      h3=new TH1I("px_per_dcol","Pixels per hit dcol",31,-0.5,30.5);	
      h4=new TH1I("dc_per_roc","Dcols per hit ROC",25,0.5,26.5);
      h5=new TH1I("roc_per_mod","ROCs per hit module",15,0.5,16.5);
      h6=new TH1D("hits_per_phase","Hits per phase (ns)",2000,-100,100);
      h7=new TH1D("rejected_per_phase","rejected hits per phase (ns)",260,0,26);
      h8=new TH1D("hits_relative_phase","time between hit and fermi clock (ns)",520,-26,26);
      g1=new TH1I("px_per_mod_per_tg","Pixels per hit module per trigger",401,-0.5,400.5);	
      g2=new TH1I("px_per_roc_per_tg","Pixels per hit RO per trigger",81,-0.5,80.5);
      g3=new TH1I("px_per_dcol_per_tg","Pixels per hit dcol per trigger",31,-0.5,30.5);	
      g4=new TH1I("dc_per_roc_per_tg","Dcols per hit ROC per trigger",25,0.5,26.5);
      g5=new TH1I("roc_per_mod_per_tg","ROCs per hit module per trigger",15,0.5,16.5);
      rotime=new TH1I("rotime", "Duration for readout in LHC clocks",500,-0.5,499.5);
      rodelay=new TH1I("rodelay", "Delay of readout in LHC clocks",1000,-0.5,999.5);
      eventsize=new TH1I("eventsize", "Size of event in pixels",500,-0.5,499.5);
      //allhits= new TH2I("total_hits","All hits from simulation",416,0,416,160,0,160);
      allhits= new TH2I("total_hits","All hits from simulation",52,0,52,80,0,80);
      ineffhits= new TH2I("ineff_hits","Inefficient hits from simulation",52,0,52,80,0,80);
      allclusters_ev = new TH2I("allclusters_ev","all clusters in a single event",52,0,52,80,0,80);
      effclusters_ev = new TH2I("effclusters_ev","eff clusters in a single event",52,0,52,80,0,80);
      eff = new TProfile("efficiency","efficiency", 30 , 0 , 30 );
      effflux = new TProfile("efficiency_flux","efficiency on clusters", 50 , 0 , 1000 );
      effflux_hits = new TProfile("efficiency_flux_hits","efficiency on hits", 50 , 0 , 1000 );
      inefftype[0] = new TProfile("inefftype0","efficient hits", 50 , 0 , 1000 ); //// 1 = ro_Wait, 2 = px_overwrite , 3 = DB_overflow, 4 = ro_Reset, 5 = TS_overflow
      inefftype[1] = new TProfile("inefftype1","inefficient hits: ro_Wait", 50 , 0 , 1000 ); //// 1 = ro_Wait, 2 = px_overwrite , 3 = DB_overflow, 4 = ro_Reset, 5 = TS_overflow
      inefftype[2] = new TProfile("inefftype2","inefficient hits: px_overwrite", 50 , 0 , 1000 ); //// 1 = ro_Wait, 2 = px_overwrite , 3 = DB_overflow, 4 = ro_Reset, 5 = TS_overflow
      inefftype[3] = new TProfile("inefftype3","inefficient hits: DB_overflow", 50 , 0 , 1000 ); //// 1 = ro_Wait, 2 = px_overwrite , 3 = DB_overflow, 4 = ro_Reset, 5 = TS_overflow
      inefftype[4] = new TProfile("inefftype4","inefficient hits: ro_Reset", 50 , 0 , 1000 ); //// 1 = ro_Wait, 2 = px_overwrite , 3 = DB_overflow, 4 = ro_Reset, 5 = TS_overflow
      inefftype[5] = new TProfile("inefftype5","inefficient hits: TS_overflow", 50 , 0 , 1000 ); //// 1 = ro_Wait, 2 = px_overwrite , 3 = DB_overflow, 4 = ro_Reset, 5 = TS_overflow
      effhitrate = new TProfile("efficiency_hitrate","efficiency", 50 , 0 , 1000 );
      DBSize=new TH1I("DB_occupancy","Data buffer occupancy",DATA_BUFFER_SIZE+1,
		      -0.5,DATA_BUFFER_SIZE+0.5);
      TSSize=new TH1I("TS_occupancy","TS buffer occupancy",TS_BUFFER_SIZE+1,
		      -0.5,TS_BUFFER_SIZE+0.5);
      IntTokenWait=new TH2I("IntTokenWait","Waiting time for internal r/o token",
                             16,-0.5,15.5,100,-0.5,99.5);
      ExtTokenWait=new TH2I("ExtTokenWait","Waiting time for external r/o token",
                             16,-0.5,15.5,1000,-0.5,999.5);
      DcolTokenWait=new TH2I("DcolTokenWait","Waiting time for r/o token in dcol",
                              16,-0.5,15.5,300,-0.5,299.5);
      DCTokenWait=new TH2I("DCTokenWait","Waiting time for r/o token in dcol for whole module",
                            416,0,416,300,-0.5,299.5);
   }
}

void FillHisto(hit_vector &hits, int trigger)
{
	if(hits.empty()) {
	  if(trigger) {
	    g1->Fill(0);
	    g2->Fill(0);
	    g3->Fill(0);
	    g4->Fill(0);
	    g5->Fill(0);
	  }
	  return;
	}
	
	for(int i=0; i<hits.size(); i++){
	  h6->Fill(hits[i].phase);
	  h8->Fill(hits[i].phase-hits[i].evtPhase);
// 	  if(!main_phaseOK(hits[i].phase)){h7->Fill(hits[i].phase);}
	}
	  
	int roc_counters[CHIPS_PER_TELESCOPE];
	for(int i=0; i<CHIPS_PER_TELESCOPE; i++)roc_counters[i]=0;
	int dcol_counters[CHIPS_PER_TELESCOPE][DCOLS_PER_ROC];
	for(int i=0; i<CHIPS_PER_TELESCOPE; i++){
		for(int j=0; j<DCOLS_PER_ROC; j++) dcol_counters[i][j]=0;
	}
	for(hit_iterator iHit=hits.begin(); iHit!=hits.end(); iHit++){
		roc_counters[iHit->roc]++;
		dcol_counters[iHit->roc][iHit->dcol]++;
	}
	h1->Fill(hits.size());
	if(trigger) g1->Fill(hits.size());
	int dcol_per_roc[CHIPS_PER_TELESCOPE];
	for(int j=0; j<CHIPS_PER_TELESCOPE; j++) dcol_per_roc[j]=0;
	int nrocs=0;

	for(int i=0; i<CHIPS_PER_TELESCOPE; i++){
	        if(trigger) g2->Fill(roc_counters[i]);
		if(roc_counters[i]>0) {
			nrocs++;
			h2->Fill(roc_counters[i]);
		}
	}
	if(nrocs>0) h5->Fill(nrocs);
	if(trigger) g5->Fill(nrocs);
	for(int i=0; i<CHIPS_PER_TELESCOPE; i++){
		for(int j=0; j<DCOLS_PER_ROC; j++){
		    if(trigger) g3->Fill(dcol_counters[i][j]);
		    if(dcol_counters[i][j]>0) {
		       h3->Fill(dcol_counters[i][j]);
		    	 dcol_per_roc[i]++;
		    }
		}
		if(dcol_per_roc[i]>0) h4->Fill(dcol_per_roc[i]);
		if(trigger) g4->Fill(dcol_per_roc[i]);
	}
}


void ReadSettings(char* fileName)
{
  //
  // general settings
  //
  MAX_EVENT = 100000;                 // #events to be processed
  MAX_TRIGGER = 369000;               // max triggers in one spill
  TRIGGER_RATE = 100;                 // L1 trigger rate in kHz
  BUNCH_SPACING = 25;                 // 25ns bunch mode
  DETECTOR = BPIX;                    // either 'BPIX' or 'FPIX'
  //
  // DAQ, telescope and ROC settings
  //

  WBC = 157;                      // trigger latency

  if(fileName[0]=='\0') {
    cout<<"Using default parameters"<<endl<<endl;
    SignalFileName = "InputFile.root";
  }
  else{  
    ifstream is(fileName,std::ios::in);
    if(!is) {
      cout << "Error: File \""<<fileName<<"\" doesn't exist."<<endl;
      exit(0);
    }
    char buf[255];
    std::string Parameter, Value;
    char equal;
    while(!is.eof() && !is.fail()) {
      is>>Parameter;
      if(Parameter[0]=='#'){
	is.getline(buf,255);
	continue;
      }
      is >> equal >> Value;
      is.getline(buf,255);
      if(equal!='=') {
	cout << "Error: Syntax error for parameter "<<Parameter<<endl;
	continue;
      }
      if(Parameter=="MAX_EVENT"){
	MAX_EVENT=atol(Value.c_str());
	continue;
      }
      if(Parameter=="MAX_TRIGGER"){
	MAX_TRIGGER=atol(Value.c_str());
	continue;
      }
      if(Parameter=="PIX_TREE_FILE"){
	PIX_TREE_FILE=Value;
	continue;
      }
      if(Parameter=="TRIGGER_RATE"){
	TRIGGER_RATE=atof(Value.c_str());
	continue;
      }
      if(Parameter=="PIX_SIGMA"){
	PIX_SIGMA=atof(Value.c_str());
	continue;
      }
      if(Parameter=="SAVE_TREE"){
	SAVE_TREE=atoi(Value.c_str());
	continue;
      }
      if(Parameter=="DET_SPACING"){
	DET_SPACING=atof(Value.c_str());
	continue;
      }
      if(Parameter=="TOKEN_DELAY"){
	TOKEN_DELAY=atoi(Value.c_str());
	continue;
      }
      if(Parameter=="TRIGGER_BUCKET"){
	TRIGGER_BUCKET=atoi(Value.c_str());
	continue;
      }
      if(Parameter=="RESETINTERVAL"){
	RESETINTERVAL=atoi(Value.c_str());
	continue;
      }
      if(Parameter=="BUNCH_SPACING"){
	BUNCH_SPACING=atoi(Value.c_str());
	if(BUNCH_SPACING%25 !=0){
	  cout<<"Error: Bunch spacing must be a multiple of 25."<<endl;
	  exit(0);
	}
	continue;
      }
      if(Parameter=="WBC"){
	WBC=atoi(Value.c_str());
	continue;
      }
      if(Parameter=="THRESHOLD"){
	THRESHOLD=atol(Value.c_str());
	continue;
      }
      if(Parameter=="SIGNAL_FILENAME"){
	SignalFileName = Value;
	continue;
      }
      if(Parameter=="BEAMINTENSITY_PARAM1"){
	BEAMINTENSITY_PARAM1 = atof(Value.c_str());
	continue;
      }
      if(Parameter=="BEAMINTENSITY_PARAM2"){
	BEAMINTENSITY_PARAM2 = atof(Value.c_str());
	continue;
      }
      if(Parameter=="BEAMINTENSITY_SCALING"){
	BEAMINTENSITY_SCALING = atof(Value.c_str());
	continue;
      }
      if(Parameter=="QIE_FILENAME"){
	QIEfileName = Value;
	continue;
      }
      if(Parameter=="MINBIAS_FILENAME"){
	MinBiasFileNames.push_back(Value);
	continue;
      }
      if(Parameter=="OUTPUT_FILENAME"){
	HistoFileName=Value;
	WriteHisto=true;
	continue;
      }
      else {cout<<"Error: Undefined parameter "<<Parameter<<endl;
	exit(0);
      }
    }
  }
	
  cout <<"Data loss simulation for ";
  cout << SignalFileName << endl;

  cout <<"Software version "<<SOFTWARE_VERSION<<endl<<endl;
   
  cout <<"Physics parameters"<<endl;
  cout <<"Events to be processed                  "<<MAX_EVENT<<endl;
  cout <<"Triggers to be send                     "<<MAX_TRIGGER<<endl;
  cout <<"Filename for signal                     "<< SignalFileName <<endl;

  cout <<"LHC running with "<<BUNCH_SPACING<<"ns bunch spacing"<<endl;

  cout << endl<<"Trigger parameters"<<endl;
  cout <<"L1 trigger rate                         "<< TRIGGER_RATE<<" kHz"<<endl;
  cout <<"WBC Trigger latency                         "<< WBC<<" LHC clocks"<<endl;

  cout << endl<<"Output filename                         " << HistoFileName<<endl;
  cout <<endl<<endl;
}

bool main_phaseOK(double phase){return(phase>9.5&&phase<14);}

void initSave(){
    pixFile = new TFile(PIX_TREE_FILE.c_str(),"RECREATE");
    pixTree = new TTree("hitTree","hits from simulation");
    pixTree->Branch("event_number",&pStruct.event_number,"event_number/i");
    pixTree->Branch("TS",&pStruct.TS,"TS/L");
    pixTree->Branch("roc",&pStruct.roc,"roc/I");
    pixTree->Branch("row",&pStruct.myrow,"myrow/I");
    pixTree->Branch("col",&pStruct.mycol,"mycol/I");
    pixTree->Branch("vcal",&pStruct.vcal,"vcal/I");
    pixTree->Branch("pulseHeight",&pStruct.pulseHeight,"pulseHeight/D");
    pixTree->Branch("phase",&pStruct.phase,"phase/D");
    pixTree->Branch("trigger_number",&pStruct.trigger_number,"trigger_number/i");
    pixTree->Branch("token_number",&pStruct.token_number,"token_number/i");
    pixTree->Branch("triggers_stacked",&pStruct.triggers_stacked,"triggers_stacked/B");
    pixTree->Branch("trigger_phase",&pStruct.trigger_phase,"trigger_phase/B");
    pixTree->Branch("data_phase",&pStruct.data_phase,"data_phase/B");
    pixTree->Branch("status",&pStruct.status,"status/B");

    
    std::size_t pos = PIX_TREE_FILE.find(".");
    std::string TRANSPARENT_PIX_TREE_FILE;
    TRANSPARENT_PIX_TREE_FILE.append(PIX_TREE_FILE.begin(),PIX_TREE_FILE.end() - 5);
    TRANSPARENT_PIX_TREE_FILE.append("_TRANSPARENT.root");
    pixTransparentFile = new TFile(TRANSPARENT_PIX_TREE_FILE.c_str(),"RECREATE");
    pixTransparentTree = new TTree("hitTree","hits from transparent simulation");
    pixTransparentTree->Branch("event_number",&pStruct.event_number,"event_number/i");
    pixTransparentTree->Branch("TS",&pStruct.TS,"TS/L");
    pixTransparentTree->Branch("roc",&pStruct.roc,"roc/I");
    pixTransparentTree->Branch("row",&pStruct.myrow,"myrow/I");
    pixTransparentTree->Branch("col",&pStruct.mycol,"mycol/I");
    pixTransparentTree->Branch("vcal",&pStruct.vcal,"vcal/I");
    pixTransparentTree->Branch("pulseHeight",&pStruct.pulseHeight,"pulseHeight/D");
    pixTransparentTree->Branch("phase",&pStruct.phase,"phase/D");
    pixTransparentTree->Branch("trigger_number",&pStruct.trigger_number,"trigger_number/i");
    pixTransparentTree->Branch("token_number",&pStruct.token_number,"token_number/i");
    pixTransparentTree->Branch("triggers_stacked",&pStruct.triggers_stacked,"triggers_stacked/B");
    pixTransparentTree->Branch("trigger_phase",&pStruct.trigger_phase,"trigger_phase/B");
    pixTransparentTree->Branch("data_phase",&pStruct.data_phase,"data_phase/B");
    pixTransparentTree->Branch("status",&pStruct.status,"status/B");


}

void endSave(){
  pixFile->cd();
  cout<<"Writing Tree...";
  pixTree->Write();
  pixFile->Close();
  cout<<"done !"<<endl;
  
  pixTransparentFile->cd();
  cout<<"Writing Transparent Tree...";
  pixTransparentTree->Write();
  pixTransparentFile->Close();
  cout<<"done !"<<endl;
}
void saveHit(pxhit * hit){
  pStruct.event_number = (*hit).event_number;   
  
  pStruct.TS=((*hit).timeStamp * 25)/1000;
  pStruct.roc=(*hit).roc;
  pStruct.myrow=(*hit).myrow;
  pStruct.mycol=(*hit).mycol;
  pStruct.vcal=(*hit).vcal;
  pStruct.pulseHeight=(*hit).pulseHeight;
  pStruct.phase=(*hit).phase;
  
  pStruct.trigger_number=(*hit).trigger_number;
  pStruct.token_number=(*hit).token_number;
  pStruct.triggers_stacked=(*hit).triggers_stacked;
  pStruct.trigger_phase=(*hit).trigger_phase;
  pStruct.data_phase=(*hit).data_phase;
  pStruct.status=(*hit).status;
  
  pixTree->Fill();
}
void saveHits(hit_vector * hits){
  for(int i=0; i<hits->size(); i++) saveHit(&((*hits)[i]));
}



void saveTransparentHit(pxhit hit){
  pStruct.event_number = hit.event_number;   
  
  pStruct.TS=(hit.timeStamp *25)/1000;
  pStruct.roc=hit.roc;
  pStruct.myrow=hit.myrow;
  pStruct.mycol=hit.mycol;
  pStruct.vcal=hit.vcal;
  pStruct.pulseHeight=hit.pulseHeight;
  pStruct.phase=hit.phase;
  
  pStruct.trigger_number=hit.trigger_number;
  pStruct.token_number=hit.token_number;
  pStruct.triggers_stacked=hit.triggers_stacked;
  pStruct.trigger_phase=hit.trigger_phase;
  pStruct.data_phase=hit.data_phase;
  pStruct.status=hit.status;
  
  pixTransparentTree->Fill();
}

