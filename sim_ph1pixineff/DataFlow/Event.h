#ifndef EVENT_H_
#define EVENT_H_

#include <vector>
#include <algorithm>

//
// hit data structure
//

class pxhit {
 public:
  unsigned int event_number;
  
  double phase;
  double evtPhase;
  long timeStamp; 
  unsigned int dtime; //time difference between two events/triggers
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
  
  unsigned int trigger_number;	
  unsigned int token_number; 	
  char triggers_stacked;	//To be implemented !
  char trigger_phase;		//To be implemented !
  char data_phase;		//To be implemented !
  char status;			//To be implemented !
  
   
   
  void printhit();   
  
  bool operator < (const pxhit& b)const{
    long aa=roc*1000+dcol;
    long bb=b.roc*1000+b.dcol;
    return (aa<bb);
  };
  void clear() { timeStamp=0;} ;
  void init() { timeStamp=-1; wrongTS = true; row = -17;} ;

  pxhit()
    {
      status=7;
      ineff = false;
      inefftype = 0;
    }
};

typedef std::vector<pxhit> hit_vector;
typedef std::vector<pxhit>::iterator hit_iterator;

class Event 
{
public:
  hit_vector hits[4];                                                   //the size of this array is still a remenant of the 4-Layer pixel detector simulation
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
