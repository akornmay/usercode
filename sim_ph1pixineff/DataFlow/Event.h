#ifndef EVENT_H_
#define EVENT_H_

#include <vector>
#include <algorithm>

//
// hit data structure
//

class pxhit {
 public:
   double phase;
   double evtPhase;
   long timeStamp; 
   bool trigger;
   int roc;
   int row;
   int dcol;
   int myrow;
   int mycol;
   bool ineff;
   int CD_Select;
   double pulseHeight;
   float flux;
   bool wrongTS;
   int inefftype; // 1 = ro_Wait, 2 = px_overwrite , 3 = DB_overflow, 4 = ro_Reset, 5 = TS_overflow
   int vcal;
   bool operator < (const pxhit& b)const{
     long aa=roc*1000+dcol;
     long bb=b.roc*1000+b.dcol;
     return (aa<bb);
   };
   void clear() { timeStamp=0;} ;
   pxhit()
     {
       ineff = false;
       inefftype = 0;
     }
};

typedef std::vector<pxhit> hit_vector;
typedef std::vector<pxhit>::iterator hit_iterator;

class Event 
{
public:
   hit_vector hits[4];
   std::vector< std::vector< pxhit> > clustersall;
   std::vector< std::vector< pxhit> > clustersafter;
   long clock;
   bool trigger;
   float flux;
   void New(long clk, int trg);
   void clusterize( bool inefficient );
   double getInefficiency()
   {
     if ( clustersall.size() > 0 )
       {
	 //printf ("CLUSTER EFF = %f : %i / %i\n" , (float)clustersafter.size() / clustersall.size() , clustersafter.size() , clustersall.size() );
	 return (float)clustersafter.size() / clustersall.size();
       }
     else
       return 1;
   }

   double getInefficiencyHit()
   {
     hit_vector myhits;

     for ( int i = 0; i < 4; i++ )
       if ( hits[i].size() > 0 )
	 {
	   myhits = hits[i];
	   break;
	 }

     if ( myhits.size() > 0 )
       {
	 int eff_hits = 0;
	 for ( int i = 0; i < myhits.size(); i++)
	   if ( !myhits[i].ineff )
	     eff_hits++;
	 return (float)eff_hits / myhits.size();
       }
     else
       return 1;
   }
};

#endif /*EVENT_H_*/
