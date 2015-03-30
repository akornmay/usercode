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


void trackslope(char trackFileName[256]){

  
  TFile * trackFile = new TFile("slope.root","RECREATE");
  if ( trackFile->IsOpen()) printf("File opened successfully\n");				    
  
				    
  char txt[256];
  sprintf(txt,"%s/MyEUTelFitTuple/EUFit",trackFileName);

  cout << txt << endl;

  TChain tree;
  tree.Add(txt);


  double fitX_0;
  double fitX_7;

  double fitY_0;
  double fitY_7;

  const double telescopelength = 7*16.;



  tree.SetBranchStatus("*",false);

  tree.SetBranchStatus("fitX_0",true);
  tree.SetBranchStatus("fitX_7",true);
  tree.SetBranchStatus("fitY_0",true);
  tree.SetBranchStatus("fitY_7",true);

  tree.SetBranchAddress("fitX_0",&fitX_0);
  tree.SetBranchAddress("fitX_7",&fitX_7);
  tree.SetBranchAddress("fitY_0",&fitY_0);
  tree.SetBranchAddress("fitY_7",&fitY_7);



  TH1D slopeX("slope X","slope X",100,-1,1);
  TH1D slopeY("slope Y","slope Y",100,-1,1);

  double x,y;

  for(int i = 0; i < tree.GetEntries(); ++i)
    {
      tree.GetEntry(i);
      //      cout << fitX_0 << "  -  "<< (TMath::Abs(fitX_0 - fitX_7)) << endl;
      slopeX.Fill(TMath::ATan((TMath::Abs(fitX_0) - TMath::Abs(fitX_7))/telescopelength)*180/TMath::Pi());
      slopeY.Fill(TMath::ATan((TMath::Abs(fitY_0) - TMath::Abs(fitY_7))/telescopelength)*180/TMath::Pi());
      //cout << (TMath::Abs(fitX_0) - TMath::Abs(fitX_7))/telescopelength * 180 / TMath::Pi() << endl;
    }

  //add some discription


  sprintf(txt,"Track slope in X-Axis;Slope [Deg];Entries");
  slopeX.SetTitle(txt);
  sprintf(txt,"Track slope in Y-Axis;Slope [Deg];Entries");
  slopeY.SetTitle(txt);

  slopeX.Fit("gaus");
  slopeY.Fit("gaus");



  slopeX.Write();
  slopeY.Write();



}
