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
#include <TH2D.h>
#include <TProfile.h>
#include <TProfile2D.h>


//to compile
/*

//depends on compiler version and slc version used. On lxplus do:
1.) source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.01/x86_64-slc5-gcc43-opt/root/bin/thisroot.sh
//or get the proper ROOT environment somewhere else

// compile the macro
2.) g++ `root-config --libs` checkHits.cpp -I $ROOTSYS/include -o checkHits

//run it
3.) run ./checkHits "filename.root"
*/


//the task of this script is to check if we have twice the same hit in the same event number, caused by the charge sharing


int main(int argc, char *argv[]) {

  //create output file
  TFile outputfile("checkHitsOut.root","RECREATE");


  //load the root file specified in the argument and add the tree to a TChain
  TChain telescope("Telescope");
  telescope.Add(argv[1]);



  //specify what we need from the tree
  telescope.SetBranchStatus("*",false);

  telescope.SetBranchStatus("Event",true);
  telescope.SetBranchStatus("roc",true);
  telescope.SetBranchStatus("col",true);
  telescope.SetBranchStatus("row",true);
  telescope.SetBranchStatus("x",true);
  telescope.SetBranchStatus("y",true);

  //get some variables
  long int event;
  int roc, col, row;
  double energy, x, y;

  //set branch addresses
  telescope.SetBranchAddress("Event",&event);
  telescope.SetBranchAddress("roc",&roc);
  telescope.SetBranchAddress("energy",&energy);
  telescope.SetBranchAddress("col",&col);
  telescope.SetBranchAddress("row",&row);
  telescope.SetBranchAddress("x",&x);
  telescope.SetBranchAddress("y",&y);

  int temproc = -5; 
  long int tempevent = -1;

  int temprow = -5;
  int tempcol = -5;


  for(int ii = 0; ii < telescope.GetEntries(); ++ii)
    {
      if(ii%1000 == 0) std::cout << "Event " << ii << std::endl;

      telescope.GetEntry(ii);
      if(event == tempevent)
	{
	  if(temproc == roc)
	    {
	      if(tempcol == col && temprow == row)
		{
		  std::cout << "double entry for " << ii << std::endl;
		}	      
	    }
	  else
	    {
	      temproc = roc;
	      tempcol = col;
	      temprow = row;
	    }

	}
      else
	{
	  tempevent = event;
	  temproc = roc;


	}


    }//for loop over tree


  outputfile.Close();
 

}


