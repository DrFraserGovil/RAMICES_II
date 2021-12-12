#pragma once
#include <vector>
#include "../Parameters/GlobalParameters.h"
#include "IMF.h"
//! A simple struct for tracking the number of stars of a given mass
class IsoMass
{
	public:
		double Mass;
		double Count;
		int BirthIndex;
		int DeathIndex;
		IsoMass();
		IsoMass(int n, double m, int birth, int death);
};


class StellarPopulation
{
	public:
		StellarPopulation(const IMF_Functor & imf, const GlobalParameters & param);
	
		void PrepareIMF();
		void FormStars(double formingMass, int timeIndex);
	private:
		const GlobalParameters & Param;
		IsoMass ImmortalStars;
		std::vector<IsoMass> Distribution;
		//~ std::vector<double> IMFWeighting;
		//~ double IMF(double totalMass);
		//~ Integral IMF_Integrand(double start, double stop, int resolution);
		
		//~ double IMF_Normalisation;
		//~ double IMF_MeanValue;
		const IMF_Functor & IMF; 
};
