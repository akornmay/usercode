#include "Event.h"

void Event::New(long clk, int trg)
{
	clock=clk;
	for(int i=0; i<4; i++) hits[i].clear();
	trigger=(trg==1);
}

bool sortbypulse(const pxhit &lhs, const pxhit &rhs) { 
  return lhs.pulseHeight > rhs.pulseHeight; 
}

void Event::clusterize( bool inefficient )
{
  //choose the correct hits in array
  hit_vector myhits;

  for ( int i = 0; i < 4; i++ )
    if ( hits[i].size() > 0 ) 
      {
	myhits = hits[i];
	break;
      }

  //end if no hits
  if ( myhits.size() == 0 ) return;

  //if only efficient hits, remove others
  if ( inefficient )
    {
      //printf("ALL hits in event: %i\n" , myhits.size() );
      for (int i = 0; i < myhits.size(); i++ )
	{
	  if ( myhits[i].ineff )
	    {
	      myhits.erase(myhits.begin() + i );
	      i -= 1;
	    }
	}
      //printf(" - hits after: %i\n" , myhits.size() );
    }

  std::sort(myhits.begin() , myhits.end() , sortbypulse );
  
  int hsize = myhits.size();
  int hsize0 = hsize;

  if ( inefficient )
    clustersafter.clear();
  else
    clustersall.clear();

  for (int i = 0; i < hsize; i++ )
    {
      double mypulse = myhits[i].pulseHeight;
      int mycol = myhits[i].mycol;
      int myrow = myhits[i].myrow;
      //printf("look around hit %i: ADC = %.5f at [%i,%i]\n" , i , mypulse , mycol , myrow );

      std::vector<pxhit> cluster;

      cluster.push_back( myhits[i] ); 

      //printf("- cluster size (should be 1): %i\n" , cluster.size() );

      //loop over all remaining hits
      for (int ihit = i+1 ; ihit < hsize; ihit++ )
	{
	  //find neighbouring hits 3x3
	  int thiscol = myhits[ihit].mycol;
	  int thisrow = myhits[ihit].myrow;
	  double dist = sqrt( (mycol-thiscol)*(mycol-thiscol) + (myrow-thisrow)*(myrow-thisrow) );
	  
	  //printf("check hit %i: adc = %.2f at [%i,%i]\n" , ihit , myhits[ihit].pulseHeight , thiscol , thisrow );

	  //hit close in 3x3 range, add hit to cluster and remove from vector
	  if ( dist < 1.5 )
	    {
	      cluster.push_back( myhits[ihit] ); 
	      //printf("- close hit ADC: %.5f at [%i,%i]\n" , myhits[ihit].pulseHeight , thiscol , thisrow );

	      myhits.erase(myhits.begin() + ihit);
	      hsize = myhits.size();

	      //printf("- - removed this hit, size now: %i\n" , hsize);
	      //printf("- - cluster size after adding 1 hit: %i\n" , cluster.size() );
	      ihit -= 1;
	    }
	}

      if ( inefficient )
	clustersafter.push_back ( cluster );
      else
	clustersall.push_back ( cluster );

      //printf("number of clusters so-far: %i\n" , clustersall.size() );
    }
  /*
  if ( inefficient ) 
    printf("number of hits = %i and eff clusters %i in event.\n" , hsize0 , clustersafter.size() );
  else
    printf("number of hits = %i and clusters %i in event.\n" , hsize0 , clustersall.size() );
  */
}
