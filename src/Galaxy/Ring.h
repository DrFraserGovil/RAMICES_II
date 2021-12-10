#pragma once
#include <vector>

#include "../Parameters/GlobalParameters.h"
#include "../Gas/GasReservoir.h"

class Ring
{
	public:
		//! Initialises itself into a primordial state
		Ring(int radiusIndex, double mass, const GlobalParameters & param);
	
		double Mass();
		const double Radius;
		const double Width;
		//Relic Reservoir
		//Star Reservoir
		GasReservoir Gas;
		
	private:
		
		
		int RadiusIndex;
		
		double Area;
	
		const GlobalParameters & Param;
};
