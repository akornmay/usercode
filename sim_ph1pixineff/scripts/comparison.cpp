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


/*
 *  This will be a script pulling plots from 3 different files to compare them
 *  1)Root tree from Geant4 simulation (DFinput)
 *  2)Root tree after DataFlow simulation (DFoutput)
 *  3)Analysis output
 */

void comparison(char DFin_location[256], char DFout_location[256], char AnaFile[256], char SimFile[256]){
  //here goes the first file
  TChain * DFinput = new TChain("Telescope");
  DFinput->Add(DFin_location);

  unsigned int Event_in;
  int roc_in;
  int col_in,row_in,vcal_in;
  float flux_in;
  double pulseHeight_in;
  cout << roc_in << endl;

  DFinput->SetBranchStatus("*",false);

  DFinput->SetBranchStatus("Event",true);
  DFinput->SetBranchStatus("roc",true);
  DFinput->SetBranchStatus("pulseHeight",true);
  DFinput->SetBranchStatus("vcal",true);
  DFinput->SetBranchStatus("col",true);
  DFinput->SetBranchStatus("row",true);
  DFinput->SetBranchStatus("flux",true);

  DFinput->SetBranchAddress("Event",&Event_in);
  DFinput->SetBranchAddress("roc",&roc_in);
  DFinput->SetBranchAddress("row",&row_in);
  DFinput->SetBranchAddress("col",&col_in);
  DFinput->SetBranchAddress("vcal",&vcal_in);
  DFinput->SetBranchAddress("flux",&flux_in);
  DFinput->SetBranchAddress("pulseHeight",&pulseHeight_in);

  cout << "Loaded DF input file from: " << DFin_location << endl;


  //here goes the second file
  TChain * DFoutput = new TChain("hitTree");
  DFoutput->Add(DFout_location);

  unsigned int event_number;
  int roc_out,col_out,row_out,vcal_out;
  double pulseHeight_out;
  long TS_out;


  DFoutput->SetBranchStatus("*",false);

  DFoutput->SetBranchStatus("event_number",true);
  DFoutput->SetBranchStatus("TS",true);
  DFoutput->SetBranchStatus("roc",true);
  DFoutput->SetBranchStatus("col",true);
  DFoutput->SetBranchStatus("row",true);
  DFoutput->SetBranchStatus("vcal",true);
  DFoutput->SetBranchStatus("pulseHeight",true);


  DFoutput->SetBranchAddress("event_number",&event_number);
  DFoutput->SetBranchAddress("TS",&TS_out);
  DFoutput->SetBranchAddress("roc",&roc_out);
  DFoutput->SetBranchAddress("col",&col_out);
  DFoutput->SetBranchAddress("row",&row_out);
  DFoutput->SetBranchAddress("vcal",&vcal_out);
  DFoutput->SetBranchAddress("pulseHeight",&pulseHeight_out);

  cout << "Loaded DF output file from: " << DFout_location << endl;

  //here come the analysis files
  //comes later


  //here come the original run data files



  //Now we make some histograms we want to compare


  TCanvas * c1 = new TCanvas("Comparison","Comparison",1200,1200);
  // gStyle->SetOptStat(0);
  c1->Divide(2,2);

  c1->cd(1);



  TH1I * Pixels_per_Event_in = new TH1I("Pixels per Event", "DataFlow input",20,0,20);
  unsigned int temp_evtn=-1;

  int ROC_DUT = 3;
  int nhits = 0;
  cout << "Total Entries input : " <<  DFinput->GetEntries() << endl;
  cout << "Total Entries output: " <<  DFoutput->GetEntries() << endl;
  //for(int i = 0; i < DFinput->GetEntries(); ++i)
  for(int i = 0; i < 5000000; ++i)
    {

      DFinput->GetEntry(i);
      // if(i%100 == 0) cout << "Entry: " << i << "Event " << Event_in << "ROC " << roc_in <<   endl;
      //cout << "Entry: " << i << "Event " << Event_in << "ROC " << roc_in << " " << row_in << "|" << col_in << endl;
      temp_evtn = Event_in;
      if(roc_in == -1)
	{
	  Pixels_per_Event_in->Fill(0);
	  continue;
	}
      else
	{
	  while(Event_in == temp_evtn)
	    {
	      // cout << "I'm in the loop. ROC " << roc_in << endl;
	      if(roc_in == ROC_DUT)
		{
		  ++nhits;
		}
	      ++i;
	      if(i == DFinput->GetEntries()) break;
	      DFinput->GetEntry(i);
	      //	      cout << "Entry: " << i << "Event " << Event_in << "ROC " << roc_in <<   endl;
	    }

	  --i;
	  Pixels_per_Event_in->Fill(nhits);
	  //	  cout << "Found event with " << nhits << "Hits on ROC " << ROC_DUT << endl;
	  nhits = 0;

	}
    }

  Pixels_per_Event_in->Draw();
  c1->Update();
  //second histogram
  cout << "Second histogramm" << endl;

  TH1I * Pixels_per_Event_out = new TH1I("Pixels per Event","DataFlow output",20,0,20);
  for(int i = 0; i < DFoutput->GetEntries();++i)
    {
      DFoutput->GetEntry(i);
      if(i%100 == 0) cout << "Entry: " << i << "Event " << event_number << "ROC " << roc_out <<   endl;
      //      cout << "Entry: " << i << "Event " << event_number << "ROC " << roc_out <<   endl;
      temp_evtn = event_number;
      if(roc_out == -1)
	{
	  nhits = 0;
	 Pixels_per_Event_out->Fill(nhits);
	  continue;
	}
      else
	{
	  while(event_number == temp_evtn)
	    {
	      // cout << "I'm in the loop. ROC " << roc_in << endl;
	      if(roc_out == ROC_DUT)
		{
		  ++nhits;
		}
	      ++i;
	      if(i == DFoutput->GetEntries()) break;
	      DFoutput->GetEntry(i);
	      // cout << "Entry: " << i << "Event " << event_number << "ROC " << roc_out <<   endl;

	    }

	  --i;
	  Pixels_per_Event_out->Fill(nhits);
	  //	  cout << "Found event with " << nhits << "Hits on ROC " << ROC_DUT << endl;
	  nhits = 0;

	}

    }

  c1->cd(2);
  Pixels_per_Event_out->Draw();
  c1->Update();

  //picking up the 3rd histogram
  TFile * simAnaFile = new TFile(SimFile,"READ");
  if ( simAnaFile->IsOpen()) printf("File opened successfully\n");				    
  
				    
  char histname[256];
  sprintf(histname,"MyCMSPixelClusteringProcessor/detector_3/pixelPerEvent_d%i",ROC_DUT);

  cout << histname << endl;

  TH1I * Hits_per_Event_sim;


  Hits_per_Event_sim=(TH1I*)simAnaFile->Get("MyCMSPixelClusteringProcessor/detector_3/pixelPerEvent_d3");
  
  c1->cd(3);
  Hits_per_Event_sim->GetXaxis()->SetRangeUser(0,20);
  Hits_per_Event_sim->SetTitle("Analysis of simulated data");
  Hits_per_Event_sim->Draw();

  c1->Update();


  //pick up the last "original" histogram

  TFile * realAnaFile = new TFile(AnaFile,"READ");
  if ( realAnaFile->IsOpen()) printf("File opened successfully\n");				    
  
				    
  sprintf(histname,"MyCMSPixelClusteringProcessor/detector_3/pixelPerEvent_d%i",ROC_DUT);

  TH1I * Hits_per_Event_ana;


  Hits_per_Event_ana=(TH1I*)realAnaFile->Get("MyCMSPixelClusteringProcessor/detector_3/pixelPerEvent_d3");
  
  c1->cd(4);
  Hits_per_Event_ana->GetXaxis()->SetRangeUser(0,20);
  Hits_per_Event_ana->SetTitle("Analysis of measured data");
  Hits_per_Event_ana->Draw();

  c1->Update();


  c1->SaveAs("comp.pdf");

  ///now for future references I'll try everything in one plot

  TCanvas * c2 = new TCanvas("c2","c2",600,600);
  c2->cd();
  //  gStyle->SetOptStat(0);

  double norm = 100.;
  cout << Pixels_per_Event_in->Integral() << endl;
  //Pixels_per_Event_out->GetXaxis()->SetLimits(0, norm);
  Pixels_per_Event_out->Scale(norm/Pixels_per_Event_out->Integral());
  Pixels_per_Event_out->SetMaximum(100);
  Pixels_per_Event_out->SetLineColor(2);
  Pixels_per_Event_out->SetLineWidth(3);
  Pixels_per_Event_out->Draw("SAME");

  Hits_per_Event_sim->Scale(norm/Hits_per_Event_sim->Integral());
  Hits_per_Event_sim->SetMaximum(100);
  Hits_per_Event_sim->SetLineColor(3);
  Hits_per_Event_sim->SetLineWidth(3);
  Hits_per_Event_sim->Draw("SAME");

  Hits_per_Event_ana->Scale(norm/Hits_per_Event_ana->Integral());
  Hits_per_Event_ana->SetMaximum(100);
  Hits_per_Event_ana->SetLineColor(4);
  Hits_per_Event_ana->SetLineWidth(3);
  Hits_per_Event_ana->Draw("SAME");

  Pixels_per_Event_in->Scale(norm/Pixels_per_Event_in->Integral());
  Pixels_per_Event_in->SetMaximum(100);
  Pixels_per_Event_in->SetLineColor(1);
  Pixels_per_Event_in->SetLineWidth(3);
  Pixels_per_Event_in->Draw("SAME");

  TLegend * leg1 = new TLegend(0.75,0.9,0.35,0.75);
  leg1->AddEntry(Pixels_per_Event_in,"DataFlow input","L");
  leg1->AddEntry(Pixels_per_Event_out,"DataFlow output","L");
  leg1->AddEntry(Hits_per_Event_sim,"Analysis of simulated data","L");
  leg1->AddEntry(Hits_per_Event_ana,"Analysis of measured data","L");


  leg1->Draw();


  c2->Update();

  c2->SaveAs("comp_Pixels_per_Event.pdf");

  /*
   *  Now some more plots
   */

  //hit map
  //simulated data
  TCanvas * c3 = new TCanvas("c3","c3",1200,600);
  c3->Divide(2,1);
  c3->cd(1);
  
  sprintf(histname,"MyCMSPixelClusteringProcessor/detector_3/pixelPerEvent_d%i",ROC_DUT);

  cout << histname << endl;

  TH2D * Hit_Map_sim;


  Hit_Map_sim=(TH2D*)simAnaFile->Get("MyCMSPixelClusteringProcessor/detector_3/hitMap_d3");
  
 
  Hit_Map_sim->SetTitle("Analysis of simulated data");
  Hit_Map_sim->Draw("COLZ");
  Hit_Map_sim->SetStats(0);
  c3->Update();

  //measured data

  c3->cd(2);

  TH2D * Hit_Map_ana;
  

  Hit_Map_ana=(TH2D*)realAnaFile->Get("MyCMSPixelClusteringProcessor/detector_3/hitMap_d3");
  
 
  Hit_Map_ana->SetTitle("Analysis of measured data");
  Hit_Map_ana->Draw("COLZ");
  Hit_Map_ana->SetStats(0);

  /*
   *cluster size
   */

  //simulated data 
  TCanvas * c4 = new TCanvas("c4","c4",1200,600);
  c4->Divide(2,1);
  c4->cd(1);
  

  sprintf(histname,"MyCMSPixelClusteringProcessor/detector_%i/clustersize_d%i",ROC_DUT,ROC_DUT);

  cout << histname << endl;

  TH1I * Clustersize_sim;


  Clustersize_sim=(TH1I*)simAnaFile->Get(histname);
  
 
  Clustersize_sim->SetTitle("Analysis of simulated data");
  Clustersize_sim->Draw();
  c4->Update();

  //measured data
  c4->cd(2);
  TH1I * Clustersize_ana;
  Clustersize_ana=(TH1I*)realAnaFile->Get(histname);
  
 
  Clustersize_ana->SetTitle("Analysis of measured data");
  Clustersize_ana->Draw();
  c3->Update();

  
  /*
   *number of tracks
   */

  string simTrackFileName(SimFile);
  simTrackFileName = simTrackFileName.replace(simTrackFileName.find("clustering"), sizeof("clustering")-1,"tracks");
 
  char * filename = simTrackFileName.c_str();

  cout << filename << endl;

  TFile * simTrackFile = new TFile(filename,"READ");
  if ( simTrackFile->IsOpen()) printf("Track file opened successfully\n");				    

  TCanvas * c5 = new TCanvas("c5","c5",1200,600);
  c5->Divide(2,1);
  c5->cd(1);

  sprintf(histname,"MyEUTelTestFitter/nTrack");
 
  cout << histname << endl;

  TH1D * NTracks_sim;
  NTracks_sim=(TH1D*)simTrackFile->Get(histname);
  NTracks_sim->SetTitle("Analysis of simulated data");
  NTracks_sim->Draw();
  c5->Update();

  string anaTrackFileName(AnaFile);
  anaTrackFileName = anaTrackFileName.replace(anaTrackFileName.find("clustering"), sizeof("clustering")-1,"tracks");
  char * filename2 = anaTrackFileName.c_str();

  TFile * anaTrackFile = new TFile(filename2,"READ");
  if ( anaTrackFile->IsOpen()) printf("Analysis track file opened successfully\n");				    

  c5->cd(2);

  TH1D * NTracks_ana;
  NTracks_ana=(TH1D*)anaTrackFile->Get(histname);
  NTracks_ana->SetTitle("Analysis of measured data");
  NTracks_ana->Draw();
  c5->Update();


}//comparison()








