#pragma once
#include "../Yields/YieldGrid.h"
#include "../Yields/SimpleYield.h"
#include "../Parameters/InitialisedData.h"
#include "StarEvents.h"
struct MassReport
{
	double Total;
	double WD;
	double NS;
	double BH;
};

class RemnantPopulation
{
	public:
		RemnantPopulation(InitialisedData & data);
	
		
		void Feed(int timeIndex, double bhMass, double wdMass, double nsMass);
		void Feed(int timeIndex, RemnantOutput rem);
		void Decay(int currentTime, std::vector<GasReservoir> & scatteringReservoir, StarEvents & EventRate);
		MassReport Mass();
	private:
		//the MASS of remnants created at each time 
		std::vector<double> ShortSNIaBuffer;
		std::vector<double> LongSNIaBuffer;
		std::vector<double> NSMBuffer;
	
		const SimpleYield & SNIaYield;
		const SimpleYield & NSMYield;
		double BlackHoleMass;
		double DormantWDMass;
		double DormantNSMass;
		const GlobalParameters & Param;
};
