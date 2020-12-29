#pragma once
#include <vector>
#include "GasReservoir.h"
#include "../Options.h"
#include "Galaxy.h"
#include "algorithm"
#include <math.h>
class Galaxy; //placeholder to allow parent/child and child/parent interaction

class Ring
{
	public:
		GasReservoir Gas;
		
		void Accrete(double t, double newDensity, double newR);
		double Mass();
		double SurfaceDensity;
		double Radius;
		Ring(Options * opts);
		Ring(Options * opts, int id, double dr, Galaxy * parent);
	private:
		Options * Opts;
		
		double Width;
		
		Galaxy * Parent;
		double Area;
		double RingMass(double GalaxyMass, double ScaleLength);
};
