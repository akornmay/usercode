#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TStyle.h>
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
#include <TProfile2D.h>
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TCanvas.h"

#include <map>
#include <list>


//to compile
/*

//depends on compiler version and slc version used. On lxplus do:
1.) source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.01/x86_64-slc5-gcc43-opt/root/bin/thisroot.sh
//or get the proper ROOT environment somewhere else

// compile the macro
2.) g++ `root-config --libs` intensityMap.cpp -I $ROOTSYS/include -o intensityMap

//run it
3.) run ./intensityMap
*/


int main(int argc, char *argv[]) {

  // opening the spill intensity file

  std::ifstream intensityFile("/home/akornmay/testbeamLogs/spillIntensity.txt");

  if (intensityFile.is_open()) 
    { 
      std::cout << "Intensity file successfully opened" << std::endl;
    }
  else
    {
      std::cout << "Error when loading intensity file" << std::endl;
      exit(1);
    }



  //processing single lines in the spill intensity file

  std::string line;
  std::string dummy_str;

  
  int runnumber;
  double batches;
  long int protons;
  double counts;
  long int timestamp;
  double dummy_dbl;


  std::map<int,long int> protonsMap;
  std::map<int,double> countsMap;

  std::map<int,long int> filtered_protonsMap;
  std::map<int,double> filtered_countsMap;

  while(std::getline(intensityFile,line))
    {
      //read file line by line
      std::istringstream iss;
      iss.str(line);
      iss >> runnumber >> batches >> dummy_dbl >> protons >> dummy_dbl >> dummy_dbl  >> dummy_dbl >> counts >> timestamp >> dummy_str; 
      //std::cout << line << '\n';
      //std::cout << runnumber << " | " << batches<< " | " << protons<< " | " << counts << std::endl;
      
      if(protonsMap.count(runnumber)==0)
	{
	  protonsMap[runnumber] = protons;
	}
      else
	{
	  std::cout << "Double RUN number " << runnumber << std::endl;
	}

      if(countsMap.count(runnumber)==0)
	{
	  countsMap[runnumber] = counts;
	}
      else
	{
	  std::cout << "Double RUN number " << runnumber << std::endl;
	}


      line.clear(); 
    }

  intensityFile.close();

  std::cout << "DONE creating intensity maps" << std::endl;

  // opening the spill config file

  std::ifstream configFile("/home/akornmay/testbeamLogs/pxltbGuiLogPixelTestBoard1.csv");
  //  std::ifstream configFile("/home/akornmay/testbeamLogs/pxltbGuiLogPixelTestBoard2.csv");

  if (configFile.is_open()) 
    { 
      std::cout << "Config file successfully opened" << std::endl;
    }
  else
    {
      std::cout << "Error when loading config file" << std::endl;
      exit(1);
    }


  //read config file and fill a list
  std::list<int> runList;

  while(std::getline(configFile,line))
    {
      //read file line by line
      std::istringstream iss;
      iss.str(line);
      iss >> runnumber >> dummy_str;

      //std::cout << runnumber << std::endl;

      runList.push_front(runnumber);

      line.clear();
    }

  std::cout << "the RunList has " << runList.size() << " Elements" << std::endl;



  //now we check if the run number from the list also exists in the intensity map

  //iterate over all values
  for(std::list<int>::iterator it = runList.begin(); it != runList.end(); ++it)
    {
      if(protonsMap.count(*it) != 0)
	{
	  //this value also exists as key here
	  filtered_protonsMap[*it] = protonsMap[*it];
	}

      if(countsMap.count(*it) != 0)
	{
	  //this value also exists as key here
	  filtered_countsMap[*it] = countsMap[*it];
	}

    }

  std::cout << "the filtered_protonsMap has " << filtered_protonsMap.size() << " Elements" << std::endl;
  std::cout << "the filtered_countsMap has " << filtered_countsMap.size() << " Elements" << std::endl;


  //now we have all the values so we create some histograms
  //output file

  TFile ofile("intensityOut.root","RECREATE");

  TTree intensityTree("intensityTree","a tree to store the beam intensity for each spill");

  int run;
  long int prot;
  double count;

  intensityTree.Branch("runnumber",&run,"runnumber/i");
  intensityTree.Branch("protons",&prot,"protons/L");
  intensityTree.Branch("count",&count,"count/D");


  for(std::map<int,long int>::iterator it = filtered_protonsMap.begin(); it != filtered_protonsMap.end(); ++it)
    {

      run = it->first;
      protons = it->second;
      count = filtered_countsMap[it->first];

      //      std::cout << run << " | " << protons << " | " << count << std::endl;

      intensityTree.Fill();

    }

  //let's create some plots

  TCanvas c("c","canvas",1200,800);
  c.cd();
  gStyle->SetOptStat(0);
  c.SetFillColor(10);


  //  TH2F countHist("countHist","Arbitrary number from downstream counter for all recorded runs");

  int n = intensityTree.Draw("count:runnumber","","");

  //  TH2F *countHist = (TH2F*)gDirectory->Get("countHist");

  TGraph *g = new TGraph(n,intensityTree.GetV2(),intensityTree.GetV1());
  g->SetNameTitle("countsPerRun","Counts on downstream scaler for each run of test board 1;run number;counts");
  
  
  c.SetLogy();
  g->SetMaximum(6000);
  g->SetMinimum(100);
  g->SetMarkerStyle(20);
  g->SetMarkerSize(0.5);

  g->Draw("ap");

  c.Update();

  c.Print("plot.pdf");

  ofile.Write();
  ofile.Close();




}


