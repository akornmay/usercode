#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
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
2.) g++ `root-config --libs` checkTrackClusters.cpp -I $ROOTSYS/include -o checkTrackClusters

//run it
3.) run ./checkTrackClusters "filename.root" "filename2.root" ..
*/


int main(int argc, char *argv[]) {

  //create output file
  TFile outputfile("checkTrackClusters.root","RECREATE");
  //  TFile outputfile("checkTrackClusters_sim175363.root","RECREATE");


  //load the root file specified in the argument
  //  TFile trackfile(argv[1],"READ");

  //std::cout << "Opening track file: " << argv[1] << std::endl;

  // now we need the correct tree file

  double x, y;
  double clustersize;


  TChain tree("MyEUTelFitTuple/EUFit");
  //  tree.Add(argv[1]);


    tree.Add("/afs/cern.ch/user/r/rslu/AFS-workspace/public/cmspxltb-submission/histograms-tb1/179506-tracks.root");
  tree.Add("/home/akornmay/results/179507_PixelTestBoard1_REALDATA/histograms/179507-tracks.root");
  tree.Add("/home/akornmay/results/179509_PixelTestBoard1_REALDATA/histograms/179509-tracks.root");
  tree.Add("/home/akornmay/results/179515_PixelTestBoard1_REALDATA/histograms/179515-tracks.root");
  tree.Add("/home/akornmay/results/179516_PixelTestBoard1_REALDATA/histograms/179516-tracks.root");
  tree.Add("/home/akornmay/results/179518_PixelTestBoard1_REALDATA/histograms/179518-tracks.root");
  //tree.Add("/home/akornmay/results/175363_PixelTestBoard1/histograms/175363-tracks.root");




  if(tree.GetEntries() == 0)
    {
      std::cout << "The tree is empty" << std::endl << "EXIT!" << std::endl;
      exit(1);
    }


  tree.SetBranchStatus("*",false);

  tree.SetBranchStatus("fitX_3",true);
  tree.SetBranchStatus("fitY_3",true);
  tree.SetBranchStatus("dutClusterSize",true);

  tree.SetBranchAddress("fitX_3",&x);
  tree.SetBranchAddress("fitY_3",&y);
  tree.SetBranchAddress("dutClusterSize",&clustersize);

  const int numberBinsX = 300;
  const int numberBinsY = 200;
  const int maxClusterSize = 8;

  TProfile2D reducedClustersize("reducedClustersize","Clustersize reduced to 4 pixel cells;um;um",numberBinsX,0.,300.,numberBinsY,0.,200.);

  std::vector<TH2I *> histVector(4,NULL);

  for(int kk = 0; kk < 4; ++kk)
    {

      char title[256];
      sprintf(title,"ClusterSizeMap of all clusters of size;um;um",kk+1);
      char name[256];
      sprintf(name,"ClusterSizeMap_%i",kk+1);

      histVector[kk] = new TH2I(name,title,numberBinsX,0.,300.,numberBinsY,0.,200.);

    } 


  for(int oo = 0; oo < tree.GetEntries(); ++oo)
    {
     
      if(oo%1000 == 0) std::cout << "Track " << oo << std::endl;
      //std::cout << x << " -- " << y << " -- " << clustersize << std::endl;

      tree.GetEntry(oo);
      if(clustersize > 0 && clustersize < maxClusterSize)
	{
	  reducedClustersize.Fill(((int)(1000*x))%300,((int)(1000*y))%200, clustersize);
	  
	  if(clustersize == 1)
	    {
	      //	      std::cout << "Filling Vector size 1" << std::endl;
	      histVector[0]->Fill(((int)(1000*x))%300,((int)(1000*y))%200);
	    }
	  else if(clustersize == 2)
	    {
	      //	      std::cout << "Filling Vector size 2" << std::endl;
	      histVector[1]->Fill(((int)(1000*x))%300,((int)(1000*y))%200);
	    }
	  else if(clustersize == 3)
	    {
	      //	      std::cout << "Filling Vector size 3" << std::endl;
	      histVector[2]->Fill(((int)(1000*x))%300,((int)(1000*y))%200);
	    }
	  else if(clustersize >= 4)
	    {
	      //	      std::cout << "Filling Vector size 4" << std::endl;
	      histVector[3]->Fill(((int)(1000*x))%300,((int)(1000*y))%200);
	    }

	}

    }
  reducedClustersize.Write();

  for(int kk = 0; kk < 4; ++kk)
    {
      histVector[kk]->Write();
    }



  //sigmas
  double sX =  4.7;
  double sY =  4.7;

  int sigmas = 5;





  TH2D* helperH = (TH2D*)reducedClustersize.Clone();
  //  helperH->Rebin2D(5,5);
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


  // integrate over certain areas to get the 1d profiles in x and y

  // limits in x in um
  
  int xleft1 = 30;
  int xright1 = 100;

  int xleft2 = 180;
  int xright2 = 260;


  TProfile yprof("yprof","Profile of clustersize in y direction",200,0,200);

  //now we fill the profile

  for(int xvar = xleft1; xvar < xright1; ++xvar)
    {
            //get the bin in x
      int xbin = reducedClustersize.GetXaxis()->FindBin(xvar);
      
      for(int ii = 0; ii < nY; ++ii)
	{
	  if(reducedClustersize.GetBinContent(xvar,ii) > 0)
	    {
	      yprof.Fill(reducedClustersize.GetYaxis()->GetBinCenter(ii),reducedClustersize.GetBinContent(xvar,ii));
	    }
	}
    }

  for(int xvar = xleft2; xvar < xright2; ++xvar)
    {
            //get the bin in x
      int xbin = reducedClustersize.GetXaxis()->FindBin(xvar);
      
      for(int ii = 0; ii < nY; ++ii)
	{
	  if(reducedClustersize.GetBinContent(xvar,ii) > 0)
	    {
	      yprof.Fill(reducedClustersize.GetYaxis()->GetBinCenter(ii),reducedClustersize.GetBinContent(xvar,ii));
	    }
	}
    }

  yprof.Write();


// limits in x in um
  
  int ybot1 = 45;
  int ytop1 = 95;

  int ybot2 = 140;
  int ytop2 = 190;


  TProfile xprof("xprof","Profile of clustersize in x direction",300,0,300);

  //now we fill the profile

  for(int yvar = ybot1; yvar < ytop1; ++yvar)
    {
            //get the bin in x
      int ybin = reducedClustersize.GetYaxis()->FindBin(yvar);
      
      for(int ii = 0; ii < nX; ++ii)
	{
	  if(reducedClustersize.GetBinContent(ii,yvar) > 0)
	    {
	      xprof.Fill(reducedClustersize.GetXaxis()->GetBinCenter(ii),reducedClustersize.GetBinContent(ii,yvar));
	    }
	}
    }

  for(int yvar = ybot2; yvar < ytop2; ++yvar)
    {
            //get the bin in x
      int xbin = reducedClustersize.GetYaxis()->FindBin(yvar);
      
      for(int ii = 0; ii < nX; ++ii)
	{
	  if(reducedClustersize.GetBinContent(ii,yvar) > 0)
	    {
	      xprof.Fill(reducedClustersize.GetXaxis()->GetBinCenter(ii),reducedClustersize.GetBinContent(ii,yvar));
	    }
	}
    }

  xprof.Write();








  outputfile.Close();



}


