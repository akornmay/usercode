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
2.) g++ `root-config --libs` checkEta.cpp -I $ROOTSYS/include -o checkEta

//run it
3.) run ./checkEta "filename.root"
*/


int main(int argc, char *argv[]) {

  //create output file
  TFile outputfile("outputfile.root","RECREATE");


  //load the root file specified in the argument and add the tree to a TChain
  TChain telescope("Telescope");
  telescope.Add(argv[1]);



  //specify what we need from the tree
  telescope.SetBranchStatus("*",false);

  telescope.SetBranchStatus("Event",true);
  telescope.SetBranchStatus("roc",true);
  telescope.SetBranchStatus("energy",true);
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

  //  const int numberBinsX = 300;
  //  const int numberBinsY = 200;
  const int numberBinsX = 300;
  const int numberBinsY = 200;


  //create a new TH2D plot
  //TH2D reducedClustersize("reducedClustersize","Clustersize reduced to 4 pixel cells;um;um",numberBinsX,0.,300.,numberBinsY,0.,200.);
  TProfile2D reducedClustersize("reducedClustersize","Clustersize reduced to 4 pixel cells;um;um",numberBinsX,0.,300.,numberBinsY,0.,200.);
  TH2D hitMap("HitMap","Pixel Hit Map for all ROCs",52,0,52,80,0,80);
  //loop over the tree

  int testROC = 0;
  long int tempEvent = -1;
  double tempEnergy, tempX, tempY = -666;
  int clustersize = 1;

  int clustersizearray[numberBinsX][numberBinsY] = {0};
  int clustercounterarray[numberBinsX][numberBinsY] = {0};

  int counter1 = 0;
  int counter2 = 0;
  //  for(testROC = 0; testROC < 8; ++testROC){
  for(int ii = 0; ii < telescope.GetEntries(); ++ii)
    {
      if(ii%10000 == 0) std::cout << "Event " << ii << std::endl;

      telescope.GetEntry(ii);
      // we are only interested in one ROC
      if(roc == testROC)
	{
	  if(roc!= -1) hitMap.Fill(col,row);
	  if(event == tempEvent)
	    {
	      //do something if it is the same event
	      //check if same impact point
	      ++counter1;

	      if(x == tempX && y == tempY)
		{
		  //	  std::cout <<"temp ("<<tempEvent << "|" << tempX << "," << tempY << ")  new ("<< event << "|" << x << "," << y << std::endl;
		  ++clustersize;
		  ++counter2;
		} 
	    }
	  else
	    {
	      //if not the same event we just keep going
	      reducedClustersize.Fill(((int)(1000*tempX))%300,((int)(1000*tempY))%200, clustersize);

	      //std::cout << clustersize << std::endl;
	      tempEvent = event;
	      tempEnergy = energy;
	      tempX = x;
	      tempY = y;

	      clustersize = 1;
	    }


	}//if testROC
    }//for loop over tree
  //}//for loop over testROC

  //  reducedClustersize.Reset();
  //reducedClustersize.SetBinContent(150,100,10);
  //reducedClustersize.SetBinEntries(reducedClustersize.GetBin(150,100),1);

  reducedClustersize.Write();
  hitMap.Write();
  //let's try do do a gaussian smearing

  //sigmas
  double sX =  4.7;
  double sY =  4.7;
  //double sX =  43.3;
  //double sY =  28.9;
  //double sX =  20.3;
  //double sY =  15.9;
  //double sX =  10;
  //double sY =  7;

  int sigmas = 5;

  //TH2D* helperH = (TH2D*)reducedClustersize.Clone();
  TH2D* helperH = (TH2D*)reducedClustersize.Clone();
  //helperH->Rebin2D(5,5);
  helperH->SetNameTitle("rebinnedClustersize","Clustersize reduced to 4 pixel cells rebinned;um;um");

  double content;
  double X, Y, w, z;
  const UInt_t nX = helperH->GetNbinsX();
  const UInt_t nY = helperH->GetNbinsY();
  UInt_t kmin, kmax, lmin, lmax;

  double xStep = 300. / nX;
  double yStep = 200. / nY;

  double sum = 0;
  for(int i = 0; i < nX; ++i)
    {
      for(int j = 0; j < nY; ++j)
	{
	  sum+= reducedClustersize.GetBinContent(i,j);
	}
    }
  std::cout <<  "computed integral = " << sum << std::endl;




  //TH2D* gaussianH  = new TH2D("gaussian", "Gaussian smeard reduced clustersize histogramm;um;um",nX,0.,300.,nY,0.,200.);
  TProfile2D* gaussianH  = new TProfile2D("gaussian", "Gaussian smeard reduced clustersize histogramm;um;um",nX,0.,300.,nY,0.,200.);

   
  for(UInt_t i=1;i<=nX;++i)
   {
      X = gaussianH->GetXaxis()->GetBinCenter(i);
      for(UInt_t j=1;j<=nY;++j)
      {
         Y = gaussianH->GetYaxis()->GetBinCenter(j);
         
         
         content = 0;
	 double myweight;
	 double weightSum = 0;

         
         //For bin i,j, we look in the old histogram at all bins in a nearby region and calculate the probability of being smeared in the current one
         
	 kmin = helperH->GetXaxis()->FindBin(X-sigmas*sX);//For being faster, we reduce the range to 3-sigma. Use kmin = 1 if you want the whole range
	 kmax = helperH->GetXaxis()->FindBin(X+sigmas*sX);//For being faster, we reduce the range to 3-sigma. Use kmax = nX if you want the whole range
         //kmin = 1;
         //kmax = nX;
         
         for(UInt_t k=kmin;k<=kmax;++k)
         {
            w = helperH->GetXaxis()->GetBinCenter(k);
	    lmin = helperH->GetYaxis()->FindBin(Y-sigmas*sY);//For being faster, we reduce the range to 3-sigma. Use lmin = 1 if you want the whole range
	    lmax = helperH->GetYaxis()->FindBin(Y+sigmas*sY);//For being faster, we reduce the range to 3-sigma. Use lmax = nY if you want the whole range
            //lmin = 1;
            //lmax = nY;
            
            for(UInt_t l=lmin;l<=lmax;++l)
            {
               z = helperH->GetYaxis()->GetBinCenter(l);
               //We approximate the integral of the gaussian function between binLowEdge and binUpEdge by the integrand times the bin width. This bin width and the normalization is divided after the end of the loop. The Error Function could be used instead.
               //content+=helperH->GetBinContent(k,l)*TMath::Gaus(X,w,sX,false)*TMath::Gaus(Y,z,sY,false);
	       myweight = TMath::Gaus(X,w,sX,true) * xStep;
	       myweight *= TMath::Gaus(Y,z,sY,true) * yStep;
	       if(helperH->GetBinContent(k,l) != 0)
		 {
		   content += helperH->GetBinContent(k,l) * myweight;
		   weightSum += myweight;
	       
		 }
            }
         }
         gaussianH->SetBinContent(i,j,content/weightSum);
	 gaussianH->SetBinEntries(gaussianH->GetBin(i,j),1);
	 //	 std::cout << "SetBinContent(" << i << ", " <<  j << ", " << content << ")" << std::endl;
      }
   }   

  helperH->Write();
  //  gaussianH->Rebin2D(5,5);
  gaussianH->Write();

  sum = 0;
  for(int i = 0; i < nX; ++i)
    {
      for(int j = 0; j < nY; ++j)
	{
	  sum+= gaussianH->GetBinContent(i,j);
	}
    }
  std::cout <<  "computed integral = " << sum << std::endl;




  //
  // let's plot the vertical and horizontal cuts
  // B = Border // C = Center
  //

  TH1D * verticalCutB = new TH1D("verticalCutB","Border cluster size profile in y-Direction;um;Cluster size",nY,0.,200.);
  TH1D * verticalCutC = new TH1D("verticalCutC","Center cluster size profile in y-Direction;um;Cluster size",nY,0.,200.);

  TH1D * horizontalCutB = new TH1D("horizontalCutB","Border cluster size profile in x-Direction;um;Cluster size",nX,0.,300.);
  TH1D * horizontalCutC = new TH1D("horizontalCutC","Center cluster size profile in x-Direction;um;Cluster size",nX,0.,300.);

  //let's do the most stupid thing and just copy the values from the 2d plot

  for(int ii = 1; ii <= nY ; ++ii)
    {
      verticalCutB->SetBinContent(verticalCutB->GetXaxis()->FindBin(gaussianH->GetYaxis()->GetBinCenter(ii)), gaussianH->GetBinContent(gaussianH->GetXaxis()->FindBin(75.),ii));
      verticalCutC->SetBinContent(verticalCutC->GetXaxis()->FindBin(gaussianH->GetYaxis()->GetBinCenter(ii)), gaussianH->GetBinContent(gaussianH->GetXaxis()->FindBin(150.),ii));
    }


  for(int ii = 1; ii <= nX ; ++ii)
    {
      horizontalCutB->SetBinContent(horizontalCutB->GetXaxis()->FindBin(gaussianH->GetXaxis()->GetBinCenter(ii)), gaussianH->GetBinContent(ii,gaussianH->GetYaxis()->FindBin(50.)));
      horizontalCutC->SetBinContent(horizontalCutC->GetXaxis()->FindBin(gaussianH->GetXaxis()->GetBinCenter(ii)), gaussianH->GetBinContent(ii,gaussianH->GetYaxis()->FindBin(100.)));
    }

  verticalCutC->Write();
  horizontalCutC->Write();

  verticalCutB->Write();
  horizontalCutB->Write();


  outputfile.Close();
 

}


