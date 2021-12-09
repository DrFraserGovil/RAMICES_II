#pragma once
#include <vector>
#include "GasReservoir.h"
#include "../Options.h"
#include "Galaxy.h"
#include "algorithm"
#include <math.h>
#include "../Stars/StellarPopulation.h"

class Galaxy; //placeholder to allow parent/child and child/parent interaction

struct GasRequest
{
	double IGM;
	double Disc;
};


class Ring
{
	public:
		GasReservoir Gas;
		double Radius;
		double SurfaceDensity;
		
		double Mass();
		
		GasRequest AccretionRequest(double t, double newDensity, double newR);
		void FormStars(double t);
		
		void UpdateInternalProperties();
		
		
		std::vector<std::string> PropertyHeaders();
		std::vector<double> ReportProperties(double t);
		Ring(Options * opts);
		Ring(Options * opts, int id, double dr, Galaxy * parent);
		
		StellarPopulation Stars;
	private:
		Options * Opts;
		
		
		double Width;
		double InnerRadius;
		double OuterRadius;
		
		double Area;
		int ID;
		Galaxy * Parent;
		
		double RingMass(double GalaxyMass, double ScaleLength);
		
		double StarFormationRate();
};
