#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <TRandom3.h>
#include <TMath.h>
#include <TGraph.h>
#include <TH1.h>
#include <TProfile.h>


//to compile
/*

//depends on compiler version and slc version used. On lxplus do:
1.) source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.01/x86_64-slc5-gcc43-opt/root/bin/thisroot.sh
//or get the proper ROOT environment somewhere else

// compile the macro
2.) g++ `root-config --libs` QIEsavehits.c -I $ROOTSYS/include -o QIEsavehits

//run it
3.) run ./a.out 179540
*/


double convert_vcal_to_adc(double vcal, double p0, double p1, double p2, double p3){

  //use the tanh function defined by 3 parameters to convert the vcal to adc values
  double adc = (p3 + p2 * TMath::TanH(p0*vcal - p1));


  if(adc > 255.)
    {
      return 255;
    }
  else
    {
      return adc;
    }

}


int main(int argc, char *argv[]) {

  /*
   * Converstion factors
   */
  
  double xray_offset = -600. ;
  double xray_slope = 50. ;
  double ionisation_energy = 3.62; //[eV/e-]

  int runNumber = atoi(argv[1]);

  int roc, col, row, adc, vcal;
  double pulseHeight, energy;
  float flux;
  int eventNr = 0;
  float x , y , z;

  TRandom3 * random = new TRandom3();

  //create the output file
  char txt[256];
  sprintf(txt,"simdataTree_RUN%i.root",runNumber);
  TFile *outputFile = new TFile(txt,"UPDATE");

  //create the tree to store the data
  char title[30];
  sprintf(title,"Telescope");
  TTree *TelescopeTree = new TTree(title,title);;

  //define the output tree
  TelescopeTree->Branch("Event", &eventNr, "Event/i");
  TelescopeTree->Branch("roc",&roc,"roc/I");
  TelescopeTree->Branch("pulseHeight", &pulseHeight, "pulseHeight/D");
  TelescopeTree->Branch("energy", &energy,"energy/D");
  TelescopeTree->Branch("vcal", &vcal, "vcal/I");
  TelescopeTree->Branch("col", &col, "col/I");
  TelescopeTree->Branch("row", &row, "row/I");
  TelescopeTree->Branch("flux", &flux, "flux/F");
  TelescopeTree->Branch("x", &x, "x/F");
  TelescopeTree->Branch("y", &y, "y/F");


  //ifstream datafile ("/afs/cern.ch/work/t/threus/public/cleanout2000k.txt");
  std::ifstream datafile ("/afs/cern.ch/work/a/akornmay/public/hugeTrackSample_cleanoutfull.dat");
  
  //now also load the QIE data
  char buffer[256];  
  //  sprintf(buffer,"rootfiles/RawData_spill%i.bin.root",runNumber);
  sprintf(buffer,"/afs/cern.ch/work/a/akornmay/private/Simulation/RawData_spill%i.bin.root",runNumber);
  std::cout<<"Spill : " << runNumber << " (" << buffer << ")" << std::endl;

  TChain *tree_QIE = new TChain("tree_QIE");
  tree_QIE->Add(buffer);
  
  TChain *tree_trigger = new TChain("tree_trigger");
  tree_trigger->Add(buffer);
  
  long long int QIEDataSize = 0;
  int N_MIturn_Hi = 0;
  int N_MIturn_Lo = 0;
  int capID = 0;
  float BeamIntensity[588];
  float ADC[588];
  float AccumulatedBeamIntensity = 0;
  long long int trigger_block = 0;

  int trigger_word_number = 0;
  int trigger_counter = 0;
  int trigger_turn_onset = 0;
  int trigger_RF_onset = 0;
  int trigger_turn_release = 0;
  int trigger_RF_release = 0;
  int trigger_intensity_sum_noinhibit = 0;
  int trigger_intensity_RFbucket[32];
  for(int i=0;i<32;i++) trigger_intensity_RFbucket[i]=0;

  tree_QIE->SetBranchAddress("QIEDataSize",&QIEDataSize);
  tree_QIE->SetBranchAddress("N_MIturn_Hi",&N_MIturn_Hi);
  tree_QIE->SetBranchAddress("N_MIturn_Lo",&N_MIturn_Lo);
  tree_QIE->SetBranchAddress("capID",&capID);
  tree_QIE->SetBranchAddress("BeamIntensity",&BeamIntensity[0]);
  tree_QIE->SetBranchAddress("ADC",&ADC[0]);
  tree_QIE->SetBranchAddress("AccumulatedBeamIntensity",&AccumulatedBeamIntensity);

  tree_trigger->SetBranchAddress("trigger_block",&trigger_block);
  tree_trigger->SetBranchAddress("trigger_word_number",&trigger_word_number);
  tree_trigger->SetBranchAddress("trigger_counter",&trigger_counter);
  tree_trigger->SetBranchAddress("trigger_turn_onset",&trigger_turn_onset);
  tree_trigger->SetBranchAddress("trigger_RF_onset",&trigger_RF_onset);
  tree_trigger->SetBranchAddress("trigger_turn_relase",&trigger_turn_release);
  tree_trigger->SetBranchAddress("trigger_RF_relase",&trigger_RF_release);
  tree_trigger->SetBranchAddress("trigger_intensity_sum_noinhibit",&trigger_intensity_sum_noinhibit);
  tree_trigger->SetBranchAddress("trigger_intensity_RFbucket",&trigger_intensity_RFbucket[0]);

  TH1D * profile = new TH1D("bucketprofile", "Bucket Profile;Bucket Id;Total Count",588,0,588);
  TH1D * spectrum = new TH1D("energyspectrum","energyspectrum",2000,0,1000);
  TH1I * spectrum_adc = new TH1I("adcspectrum","adcspectrum",256,0,256);
  // TGraph * phcal = new TGraph();
  //to read track information form geant4 output
  std::string dummy_str;
  float dummy_fl;
  std::string tag;
  //float planes[8] = { 5 , 10 , 15 , 20 , 25 , 30 , 35 , 40 };
  float planes[8] = { 1.6 , 3.2 , 4.8 , 6.4 , 8.0 , 9.6 , 11.2 , 12.8 };
  float offset = 1.0;
  //int plane = 0;

  std::string line;

  int nParticles = 0;
  int nHits = 0;



  //  int graphcounter = 0;
  // Iterate over the Tree:
  for(int turn = 0; turn < tree_QIE->GetEntries(); turn++) {
    //for(int turn = 20000; turn < tree_QIE->GetEntries(); turn++) {
    //for(int turn = 0; turn < 100; turn++) {
    //when we run out of 
    
    if(turn<1000 && turn%100 == 0) printf("computing turn %i Event %i \n",turn, eventNr);
    if(turn<1000000 && turn%1000 == 0) printf("computing turn %i Event %i \n",turn, eventNr);


    if(datafile.eof()) 
      {
	outputFile->Write();

	datafile.clear();
	datafile.seekg(0, std::ios::beg);
	printf("starting again at the beginning of the data file \n");
	printf("read %i turns from QIE file\n",turn);
	//break;
      }    

    //for(int turn = 0; turn < 20; turn++) {
    //int turn = 150000;
    // Get the values for the next turn:
    tree_QIE->GetEntry(turn);

    // Run over all buckets in this turn:
    for(int bucket = 0; bucket < 588; bucket++) {
      // Fill the intensity profile:
      //printf("%i\n",BeamIntensity[bucket]);
      profile->Fill(bucket,BeamIntensity[bucket]);

      //now we have the intensity and need to pick the corresponding number of tracks from the geant4 data
      //convert BeamIntesity to number of particles


      float meanParticles = (BeamIntensity[bucket] - BeamIntensity[32] + 260.)/320.;
      int particlesInEvent = (int) meanParticles;

      //      if ( hitsInEvent == 0 )
      //	hitsInEvent = 1;

      flux = particlesInEvent * 40. / 0.64;
      int meanParticleRate = (int) flux;

      //      printf(" particles: %f  %i \n",meanParticles, particlesInEvent);

      eventNr++;
      
      //printf("event %i: %f %f %i %i\n" , eventNr , flux , meanHits , meanhitRate , hitsInEvent );


      // for now we only have filled events
      nParticles = 0;
      std::istringstream iss;
      if(particlesInEvent != 0)
	{
	  while ( nParticles < particlesInEvent && std::getline(datafile , line ) )
	    {
	      
	      if ( line.find("Hits Collection") != std::string::npos )
		{
		  
		  //get number of hits in this GEANT4 event
		  iss.str(line);
		  iss >> dummy_str >> dummy_str >> dummy_str >> dummy_str >> dummy_str >> dummy_str >> dummy_str >> nHits >> dummy_str >> dummy_str >> dummy_str >> dummy_str >> dummy_str ;
		  
		  //now read all the hits of the event and write them to the root file
		  //one hit is one line
		  
		  for(int hitcounter = 0; hitcounter < nHits; hitcounter++)
		    {
		      
		      //printf("we have %i hits. processing hit %i \n",nHits, hitcounter);
		      //read one line
		      std::getline(datafile,line);
		      
		      iss.str(line);
		      iss >> dummy_str >> dummy_fl >> dummy_str >> dummy_fl >> dummy_str >> dummy_str >> energy >> dummy_str >> dummy_str >> x >> y >> z >> dummy_str;

		      spectrum->Fill(energy);

		      vcal  = (energy / ionisation_energy * 1000. - xray_offset)/xray_slope;
		      //adc = convert_el_to_adc(energy,+2.465504e-03, +9.631035e-01, +1.126552e+02, +1.439645e+02, 50., -600.);
		      pulseHeight = convert_vcal_to_adc(vcal,+3.122759e-03, +1.085111e+00, +1.036756e+02, +1.520615e+02);
		      spectrum_adc->Fill(pulseHeight);
		      

		      		  
		      
		      //The data flow simulation differntiates the different ROCs by enlaring the number of COLs by 8 and the number of ROWs by 2
		      //All hits of the 8 ROCs in the telescope are spread over (8*52)cols X (80)rows 
		      
		      roc = (int) (z/1.6+0.5) - 1;
		      row = 80 / 2 + y / 0.01 ;
		      col = 52 / 2 + x / 0.015;
		      //		      col = 52 / 2 + x / 0.015;
		      
		      //printf("ROC %i Pixel (%i|%i)(%i) \n",(int)(z/1.6),adc,col,row);
		      
		      TelescopeTree->Fill();
		      
		      
		    }
		}
	      line.clear();      
	      nParticles++;
	      
	    }
	  
	
	}
      else
	{
	  roc = -1;
	  adc = -1;
	  row = -1;
	  col = -1;
	  TelescopeTree->Fill();

	}



    }
  }
  //  phcal->Write();
  outputFile->Write();
  outputFile->Close();

}


