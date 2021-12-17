#pragma once
#include "../Parameters/GlobalParameters.h"

class RemnantPopulation
{
	public:
		RemnantPopulation(const GlobalParameters & param);
	
		void Feed(int timeIndex, double bhMass, double wdMass, double nsMass);
	
	private:
		//the MASS of remnants created at each time 
		std::vector<double> ShortSNIaBuffer;
		std::vector<double> LongSNIaBuffer;
		std::vector<double> NSMBuffer;
	
		double BlackHoleMass;
		double DormantWDMass;
		double DormantNSMass;
		const GlobalParameters & Param;
};
