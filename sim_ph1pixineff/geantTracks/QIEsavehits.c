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

double convert_el_to_adc(double energy, double p0, double p1, double p2, double p3, double xray_slope, double xray_offset){

  double electrons = energy / 3.62;

  //calculate the Vcal value for the xray calibration values 
  double vcal = (electrons*1000. - xray_offset)/xray_slope;

  //use the tanh function defined by 3 parameters to convert the vcal to adc values
  double adc = (p3 + p2 * TMath::TanH(p0*vcal - p1));

  // printf("#electrons %f: vcal %f: adc %f\n",electrons, vcal, adc);

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


  int runNumber = atoi(argv[1]);

  short int col, row, adc;
  short int ladder = 2;
  short int mod = 3;
  short int disk = 2;
  short int blade = 2;
  short int panel = 2;
  float flux;
  int eventNr = 0;
  float energy , x , y , z;

  TRandom3 * random = new TRandom3();

  TFile *outputFile = new TFile("output_geant.root","UPDATE");

  //create the tree to store the data
  TTree *bpixTree[3];

  char title[30];
  for (int i=1; i<4; i++){
    sprintf(title,"BPIX_Digis_Layer%1d",i);
    bpixTree[i-1]= new TTree(title,title);
    bpixTree[i-1]->Branch("Event", &eventNr, "Event/i");
    bpixTree[i-1]->Branch("Ladder", &ladder, "Ladder/S");
    bpixTree[i-1]->Branch("Module", &mod, "Module/S");
    bpixTree[i-1]->Branch("adc", &adc, "adc/S");
    bpixTree[i-1]->Branch("col", &col, "col/S");
    bpixTree[i-1]->Branch("row", &row, "row/S");
    bpixTree[i-1]->Branch("flux", &flux, "flux/F");
    bpixTree[i-1]->Branch("x", &x, "x/F");
    bpixTree[i-1]->Branch("y", &y, "y/F");
  }

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


		      //	      if ( TMath::Abs ( z - planes[ plane ] ) < offset ) //only one plane is written out
		      adc = (int)energy;
		      spectrum->Fill(energy);
		      
		      //adc = convert_el_to_adc(energy,+2.465504e-03, +9.631035e-01, +1.126552e+02, +1.439645e+02, 50., -600.);
		      adc = convert_el_to_adc(energy,+3.122759e-03, +1.085111e+00, +1.036756e+02, +1.520615e+02, 50., -600.);
		      spectrum_adc->Fill(adc);
		      

		      //phcal->SetPoint(graphcounter,((energy*1000.+600.)/50.),adc);
		      //graphcounter++;
		  
		      
		      //The data flow simulation differntiates the different ROCs by enlaring the number of COLs by 8 and the number of ROWs by 2
		      //All hits of the 8 ROCs in the telescope are spread over (8*52)cols X (80)rows 
		      
		      row = 80 / 2 + y / 0.01 ;
		      col = (((int)(z/1.6 +0.5) -1)*52 )+ 52 / 2 + x / 0.015;
		      //		      col = 52 / 2 + x / 0.015;
		      
		      //printf("ROC %i Pixel (%i|%i)(%i) \n",(int)(z/1.6),adc,col,row);
		      
		      bpixTree[2]->Fill();
		      //		  bpixTree[1]->Fill();
		      
		      
		    }
		}
	      line.clear();      
	      nParticles++;
	      
	    }
	  
	
	}
      else
	{
	  adc = -1;
	  row = -1;
	  col = -1;
	  bpixTree[2]->Fill();

	}



    }
  }
  //  phcal->Write();
  outputFile->Write();
  outputFile->Close();

  /*
  TFile* f = new TFile("file.root","RECREATE");
  profile->Write();

  spectrum->Write();

  spectrum_adc->Write();

  f->Close();*/
}


