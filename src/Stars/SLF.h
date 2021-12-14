#pragma once
#include "../Parameters/GlobalParameters.h"

class SLF_Functor
{
	public:
		SLF_Functor(const GlobalParameters & Param);
		
		int operator() (int mass, double metallicity);
		double PredictLifetime(double mass, double logmetallicity);
	private:
		
		double ValueInquiry(int m, int z);
		const GlobalParameters & Param;
		
		double LifeTime(int mass, int metallicity);
		std::vector<std::vector<double>> PrecomputedGrid;
		const double NotComputed = -1;
};