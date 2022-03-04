#pragma once
#include <sstream>
class StarEvents
{
	public:
		double StarMassFormed;
		double NStarsFormed;
		double CCSN;
		double AGBDeaths;
		double NSM;
		double SNIa;
		double ECSN;
		double Efficiency;
		StarEvents();
		
		void AddHeaders(std::stringstream & output);
		void Save(std::stringstream & output,double timestep);
};
