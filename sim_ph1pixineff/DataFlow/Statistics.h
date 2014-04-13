#ifndef STATISTICS_H_
#define STATISTICS_H_

#include <iostream>
#include <iomanip>
#include <string>


using std::cout;
using std::endl;
using std::string;
	
class statistics 
{

public:
	double inefficiency(){
		double ineff=(double)(px_overwrite+dcol_busy+TBM_busy+
				      TS_overflow+DB_overflow+ro_Wait+ro_Reset);
		return ineff!=0 ? ineff/(double) total_hits : 0;
	}
	
	void Reset(){
		total_hits=0;
		px_overwrite=0;
		dcol_busy=0;
		TBM_busy=0;
		TS_overflow=0;
		DB_overflow=0;
		ro_Wait=0;
		ro_Reset=0;
		px_fluence=0.;
		px_ro=0.;
	}
	
	statistics& operator=(statistics const & a){
		total_hits=a.total_hits;
		px_overwrite=a.px_overwrite;
		dcol_busy=a.dcol_busy;
		TBM_busy=a.TBM_busy;
		TS_overflow=a.TS_overflow;
		DB_overflow=a.DB_overflow;
		ro_Wait=a.ro_Wait;
		ro_Reset=a.ro_Reset;
		px_fluence=a.px_fluence;
		px_ro=a.px_ro;
		return *this;
	}
	
	statistics& operator+=(statistics const & a){
		total_hits+=a.total_hits;
		px_overwrite+=a.px_overwrite;
		dcol_busy+=a.dcol_busy;
		TBM_busy+=a.TBM_busy;
		TS_overflow+=a.TS_overflow;
		DB_overflow+=a.DB_overflow;
		ro_Wait+=a.ro_Wait;
		ro_Reset+=a.ro_Reset;
		px_fluence+=a.px_fluence;
		px_ro+=a.px_ro;
		return *this;
	}
	
	void PrintStat(string text){
		cout << std::fixed;
		cout << "Statistics for " << text << endl<<endl;
		cout << "Total number of hits:      "<<total_hits<<endl;
		Print("Pixel overwrite:             ", px_overwrite);
		Print("Column drain busy (3rd hit): ", dcol_busy);
		if(TBM_busy>0){
		   Print("TBM stack overflow:          ", TBM_busy);
		}
		Print("Time stamp buffer overflow:  ", TS_overflow);
		Print("Data buffer overflow:        ", DB_overflow);
		Print("Column blocked for readout:  ", ro_Wait);
		Print("Reset after readout:         ", ro_Reset);
		Print("Overall inefficiency:                       ", inefficiency());
		cout << std::setprecision(2);
		if(text.find("Module")!=string::npos){
			px_fluence/=(double) CHIPS_PER_MODULE;
		}
		cout << "Pixel fluence:               "<<px_fluence<<" MHz/cm2"<<endl;
	}
	
	inline void Print(string text, unsigned long value){
	        cout << text << std::setw(10) << value << " ,   "; 
		cout << std::setprecision(2)<< (double)value/(double)(total_hits)*100<<"%"<<endl;
		cout << std::setprecision(0);
	}
	inline void Print(string text, double value){
		cout << text << std::setprecision(2) << value*100<<"%"<<endl;
		cout << std::setprecision(0);
	}
	unsigned long total_hits;
	unsigned long px_overwrite;
	unsigned long dcol_busy;
	unsigned long TBM_busy;
	unsigned long TS_overflow;
	unsigned long DB_overflow;
	unsigned long ro_Wait;
	unsigned long ro_Reset;
	double px_fluence;
	double px_ro;
	
};


#endif /*STATISTICS_H_*/
