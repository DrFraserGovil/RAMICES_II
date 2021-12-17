#pragma once
#include "../Parameters/GlobalParameters.h"



struct Integral
{
	double ZerothMoment;
	double FirstMoment;
};
class IMF_Functor
{
	public:
		IMF_Functor(const GlobalParameters & param);
		
		double operator()(double mass);
		
		double FormationCount(double formationMass) const;
		double Weighting(int i) const;
		
	private:
		const GlobalParameters & Param;
		
		double IMF_Normalisation;
		double IMF_MeanMass;
		Integral MomentCompute(double start,double stop, int resolution);
		void Normalise();
		double IMF(double mass);
		std::vector<double> IMF_Weighting;
};

