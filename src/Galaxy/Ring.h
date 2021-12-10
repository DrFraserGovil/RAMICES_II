#pragma once
#include <vector>

#include "../Parameters/GlobalParameters.h"
#include "../Gas/GasReservoir.h"

class Ring
{
	public:
		//~ Ring();
		Ring(int radiusIndex, double mass, const GlobalParameters & param);
	
		double Mass();
	private:
		//Relic Reservoir
		//Star Reservoir
		GasReservoir Gas;
		double Radius;
		int RadiusIndex;
		double Width;
		double Area;
	
		const GlobalParameters & Param;
};
