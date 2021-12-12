#pragma once
#include <vector>

#include "../Parameters/GlobalParameters.h"
#include "../Gas/GasReservoir.h"
#include "../Stars/StarReservoir.h"
#include "../Stars/IMF.h"
class Ring
{
	public:
		//! Initialises itself into a primordial state
		Ring(int radiusIndex, double mass, const IMF_Functor & imf, const GlobalParameters & param);
	
		double Mass();
		const double Radius;
		const double Width;
		//Relic Reservoir
		StarReservoir Stars;
		GasReservoir Gas;
		
		void MakeStars();
	private:
		
		
		int RadiusIndex;
		
		double Area;
	
		const GlobalParameters & Param;
		const IMF_Functor & IMF;
};
