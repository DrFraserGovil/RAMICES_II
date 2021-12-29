#pragma once
#include <sstream>
class StarEvents
{
	public:
		double StarMassFormed;
		int NStarsFormed;
		int CCSN;
		int AGBDeaths;
		int NSM;
		int SNIa;
	
		StarEvents();
		
		void AddHeaders(std::stringstream & output);
		void Save(std::stringstream & output,double timestep);
};