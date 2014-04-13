#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <TRandom3.h>


void savehits() {

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

  TFile outputFile ("output_geant.root","RECREATE");

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

  ifstream datafile ("/afs/cern.ch/work/t/threus/public/cleanout2000k.txt");
  
  string dummy_str;
  float dummy_fl;
  string tag;
  //float planes[8] = { 5 , 10 , 15 , 20 , 25 , 30 , 35 , 40 };
  float planes[8] = { 1.6 , 3.2 , 4.8 , 6.4 , 8.0 , 9.6 , 11.2 , 12.8 };
  float offset = 1.0;
  int plane = 0;

  string line;

  int nhits = 0;

  while ( !datafile.eof() )
    {
      //float meanFlux = 1000 * random->Rndm(); //Mhz/cm2
      //float meanHits =  meanFlux / 40. * 0.64;

      float meanHits = random->Exp(2);
      float meanFlux = meanHits * 40. / 0.64;

      int meanhitRate = (int) meanFlux;
      int hitsInEvent;

      /*
      if (meanhitRate > 50)
	hitsInEvent = random->PoissonD(meanHits);
      else
	(random->Uniform(1.0) > meanhitRate / 100. ) ? hitsInEvent = 0 : hitsInEvent = 1 ;
      */
      hitsInEvent = (int) meanHits;
      if ( hitsInEvent == 0 )
	hitsInEvent = 1;
      
      //flux = hitsInEvent * 40. / 0.64;
      flux = meanFlux;

      eventNr++;
      
      printf("event %i: %f %f %i %i\n" , eventNr , meanFlux , meanHits , meanhitRate , hitsInEvent );

      nhits = 0;

      while ( nhits < hitsInEvent && std::getline(datafile , line ) )
	{
	  
	  if ( line.find("Hits Collection") != std::string::npos )
	    {
	      //nothing
	    }
	  else
	    {
	      //hits
	      std::istringstream(line,ios_base::in) >> dummy_str >> dummy_fl >> dummy_str >> dummy_fl >> dummy_str >> dummy_str >> energy >> dummy_str >> dummy_str >> x >> y >> z >> dummy_str;
	      //choose ROC
	      if ( TMath::Abs ( z - planes[ plane ] ) < offset )
		{
		  adc = (int)energy;
		  row = 80 / 2 + y / 0.01;
		  col = 52 / 2 + x / 0.015;
		  bpixTree[2]->Fill();
		  bpixTree[1]->Fill();
		  
		  nhits++;

		  //printf("- adding hit.\n");

		}
	    }
	  
	  line.clear();
	}
    }
  outputFile.Write();
  outputFile.Close();
}

