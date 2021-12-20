#pragma once
#include "../Parameters/InitialisedData.h"

class RemnantPopulation
{
	public:
		RemnantPopulation();
	
		
		void Feed(int timeIndex, double bhMass, double wdMass, double nsMass);
		void Decay(int currentTime);
	private:
		//the MASS of remnants created at each time 
		std::vector<double> ShortSNIaBuffer;
		std::vector<double> LongSNIaBuffer;
		std::vector<double> NSMBuffer;
	
		//~ const YieldGrid SNIaYield;
	
		double BlackHoleMass;
		double DormantWDMass;
		double DormantNSMass;
		const GlobalParameters & Param;
};
