#include <TFile.h>
#include <TCanvas.h>
#include <iostream>
#include <TRandom.h>
#include <TRandom3.h>
#include <TTree.h>
#include <TF1.h>
#include <TMath.h>


Double_t langaufun(Double_t *x, Double_t *par) {

   //Fit parameters:
   //par[0]=Width (scale) parameter of Landau density
   //par[1]=Most Probable (MP, location) parameter of Landau density
   //par[2]=Total area (integral -inf to inf, normalization constant)
   //par[3]=Width (sigma) of convoluted Gaussian function
   //
   //In the Landau distribution (represented by the CERNLIB approximation), 
   //the maximum is located at x=-0.22278298 with the location parameter=0.
   //This shift is corrected within this function, so that the actual
   //maximum is identical to the MP parameter.

      // Numeric constants
      Double_t invsq2pi = 0.3989422804014;   // (2 pi)^(-1/2)
      Double_t mpshift  = -0.22278298;       // Landau maximum location

      // Control constants
      Double_t np = 100.0;      // number of convolution steps
      Double_t sc =   5.0;      // convolution extends to +-sc Gaussian sigmas

      // Variables
      Double_t xx;
      Double_t mpc;
      Double_t fland;
      Double_t sum = 0.0;
      Double_t xlow,xupp;
      Double_t step;
      Double_t i;


      // MP shift correction
      mpc = par[1] - mpshift * par[0]; 

      // Range of convolution integral
      xlow = x[0] - sc * par[3];
      xupp = x[0] + sc * par[3];

      step = (xupp-xlow) / np;

      // Convolution integral of Landau and Gaussian by sum
      for(i=1.0; i<=np/2; i++) {
         xx = xlow + (i-.5) * step;
         fland = TMath::Landau(xx,mpc,par[0]) / par[0];
         sum += fland * TMath::Gaus(x[0],xx,par[3]);

         xx = xupp - (i-.5) * step;
         fland = TMath::Landau(xx,mpc,par[0]) / par[0];
         sum += fland * TMath::Gaus(x[0],xx,par[3]);
      }

      return (par[2] * step * sum * invsq2pi / par[3]);
}



Double_t AsyGaus(Double_t * xx, Double_t * par){
  Double_t x = xx[0];
  Double_t mean = par[0];
  Double_t sigma1 = par[1];
  Double_t sigma2 = par[2];
  Double_t amplitude = par[3];

  if(x < mean)
    return amplitude * TMath::Gaus(x, mean, sigma1);
  else
    return amplitude * TMath::Gaus(x, mean, sigma2);


}




void createTBevents(int input, int hitRate){

  printf("Starting Simulation of data\n");

  //creating the output file
  //  char outputFileName[100] = {"OutputFile.root"};
  //char outputFileName[100] = Form("OutputFile_%i.root",hitRate);
  char outputFileName[100];
  sprintf( outputFileName , "OutputFile_%d.root" , hitRate );

  printf("Creating output file: %s \n",outputFileName);
  TFile * outputFile = new TFile(outputFileName,"RECREATE");

  TH1F* exprandom = new TH1F("random","random",50, 0,50);

  //Counter for event number
  unsigned int eventNr;

  //Counter for total number of hits
  unsigned int hitsTotal = 0;

  
  short int col, row, adc;
  short int ladder = 2;
  short int mod = 3;
  short int disk = 2;
  short int blade = 2;
  short int panel = 2;
  float flux;
  
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
  }

  

  //Maximum number of events. Events does not correspond with Hits
  unsigned int maxEventNr = input;

  //Number of Hits per Event
  //This should be randomized later and be dependant on the rate
  double meanHitsPerEvent  = hitRate * 1000000 * 1 / 40000000 * 0.64;

 
  printf("meanHitsPerEvent: %f \n",meanHitsPerEvent); 
  //Maximum particle flux [MHz cm^-2]
  int maxParticleFlux = 500;

  //number of hits in current event
  int hitsInEvent = -1; 

  //create a random number generator
  TRandom3 * random = new TRandom3();
  TRandom3 * randomrow = new TRandom3();
  TRandom3 * randomcol = new TRandom3();
  TRandom3 * randomadc = new TRandom3();

  //using custom function to distribute values
  //values used from http://ntucms1.cern.ch/pixel_dev/flux/v8/016031/fitspot_bin_11.pdf
  
  TF1 * fx = new TF1("xfunc", "[0]*exp(2.59349*exp(-0.5*((-x+3.24273/.15+[1]/.15)/7.07486*.15)**2)+2.07765*exp(-0.5*((-x+9.33060e-01/.15+[1]/.15)/2.24067*.15)**2)-4.21847)",0, 52);
  
  fx->SetParameters(1,337.0);
  fx->SetParameters(2,1.74);
  
  TF1 * fy = new TF1("yfunc", AsyGaus,0,80,4);
  
  fy->SetParNames("mean","sigma1","sigma2","amplitude");
  fy->SetParameter("mean",43);
  fy->SetParameter("sigma1",11.4);
  fy->SetParameter("sigma2",15.0);
  fy->SetParameter("amplitude",347.0);
  
  TF1 * fadc = new TF1("adcfunc", langaufun,0,400,4);
  fadc->SetParNames("scale","mpv","area","sigma");
  fadc->SetParameter("scale",19);
  fadc->SetParameter("mvp",220);
  fadc->SetParameter("area",10000);
  fadc->SetParameter("sigma",30);




  while (eventNr < maxEventNr){

    if(eventNr<50000 && !(eventNr %5000)) cout <<"Processing event number "<< eventNr << " ....."<<endl;
    else if(eventNr<100000 && !(eventNr %10000)) cout <<"Processing event number "<< eventNr << " ....."<<endl;
    else if(!(eventNr %50000)) cout <<"Processing event number "<< eventNr << " ....."<<endl;



    //printf("eventNr: %d \n",eventNr);
    //Function used for fitting according to Xin an Stefano
    
    //Start by generating the number of hits per event
    //following a poisson distribution
    random->SetSeed(0);
    
    exprandom->Fill( random->Exp(5) );
    
    float meanHits = random->Exp(2);
    //redo rate
    float meanRate =  meanHits / ( 1000000. * 1. / 40000000. * 0.64 );
    hitRate = (int)meanRate;
    meanHitsPerEvent = meanHits;

    printf("new mean hits %f new mean rate %f hit rate %i\n" , meanHits , meanRate , hitRate );

    flux = meanRate;
    
    //Tomas: removed shadow initialisaiton
    //int hitsInEvent;
    if (hitRate > 50)
      {
	hitsInEvent = random->PoissonD(meanHitsPerEvent);
      }
    else
      {
	double a = random->Uniform(1.0);
	(a > (hitRate/100.)) ? hitsInEvent = 0 : hitsInEvent = 1 ; 
      }
    //    printf("hitsInEvent %d \n", hitsInEvent);


    hitsTotal += hitsInEvent;

    if(hitsInEvent < 0){
      printf("ERROR: Number of hits in event is negative!!!\n");
      break;
    }

    // if(hitsInEvent < 1){
    //   printf("ERROR: there are no hits in this event\n");
    // 	}
    //distribute the hits in the event over the roc according to uniform distribution

    for(int i = 0; i < hitsInEvent; ++i){

      //random row value
      randomrow->SetSeed(0);
      row = randomrow->Integer(80);
      
      //random column value
      randomcol->SetSeed(0);
      col = randomcol->Integer(52);

      // printf("row: %d | col: %d | adc: %d\n",row,col,adc);

      bpixTree[2]->Fill();

    }



    for(int i = 0; i < hitsInEvent; ++i){

      //random row value
      row = fy->GetRandom();
      
      //random column value
      col = fx->GetRandom();

      //random adc value
      adc = fadc->GetRandom();

      //printf("row: %d | col: %d | adc: %d\n",row,col,adc);


      bpixTree[1]->Fill();

    }


    ++ eventNr;
  }


  printf("Total number of Hits: %d\n",hitsTotal);
  printf("Writing output file: %s \n", outputFileName);

  outputFile->cd();
  outputFile->Write();
  outputFile->Close();

  printf("DONE!\n");

}


