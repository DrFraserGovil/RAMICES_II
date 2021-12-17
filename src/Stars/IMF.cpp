#include "IMF.h"

double IMF_Functor::IMF(double mass)
{
	//~ std::cout << "IMF " << mass << std::endl;
	//! implementation of Chabrier (2003)
	if (mass == 0)
	{
		return 0;
	}
	const double prefactor = 0.158/(log(10));
	const double mu = 0.08;
	const double sigma = 0.69;
	const double k = prefactor * exp( - log10(mu) * log10(mu)/(2*sigma*sigma));
	
	double value;
	if (mass < 1)
	{
		double dist = (log10(mass / 0.08) ) / 0.69;
		value = prefactor / mass * exp( - dist*dist / 2);
	}
	else
	{
		value = k * pow(mass,-Param.Stellar.IMF_Slope);
	}
	return value * IMF_Normalisation;
}
double IMF_Functor::operator ()(double mass)
{
	return IMF(mass);
}

IMF_Functor::IMF_Functor(const GlobalParameters & param): Param(param)
{
	Normalise();	
	if (Param.Meta.Verbosity > 0)
	{
		std::cout << "\tIMF Functor Initialised" << std::endl;
	}
}
double IMF_Functor::FormationCount(double formingMass) const
{
	//1e9 comes from conversion between stellar mass (IMF units) and galactic mass (formation units)
	return formingMass * 1e9 / IMF_MeanMass;
}

double IMF_Functor::Weighting(int i) const
{
	return IMF_Weighting[i];
}


Integral IMF_Functor::MomentCompute(double start, double stop, int resolution)
{
	double x = start;
	Integral integral;
	integral.ZerothMoment = 0.5 * (IMF(start) + IMF(stop));
	integral.FirstMoment = 0.5 * (IMF(start)*start + IMF(stop)*stop);
	double delta = (stop - start)/resolution;
	for (int i = 0; i < resolution; ++i)
	{
		double m = start + (i+1)*delta;
		double imf = IMF(m);
		integral.ZerothMoment += imf;
		integral.FirstMoment += imf*m; 
	}	
	integral.ZerothMoment *= delta;
	integral.FirstMoment *= delta;
	return integral;
}
void IMF_Functor::Normalise()
{
	IMF_Normalisation = 1;
	int inter_delta_resolution = 3000;
	
	Integral integratedIMF = MomentCompute(Param.Stellar.MinStellarMass, Param.Stellar.ImmortalMass,inter_delta_resolution);
	IMF_Weighting.resize(Param.Stellar.MassResolution);
	for (int i = 0; i < Param.Stellar.MassResolution; ++i)
	{
		double m = Param.Stellar.MassGrid[i];
		double w = Param.Stellar.MassDeltas[i];
		Integral subsection = MomentCompute(m - w/2, m + w/2,inter_delta_resolution);
		integratedIMF.ZerothMoment += subsection.ZerothMoment;
		integratedIMF.FirstMoment += subsection.FirstMoment;
		IMF_Weighting[i] = subsection.ZerothMoment;
	}
	
	//perform the normalisation corrections
	IMF_Normalisation = 1.0/integratedIMF.ZerothMoment;
	IMF_MeanMass = integratedIMF.FirstMoment/integratedIMF.ZerothMoment;
	for (int i = 0;i < IMF_Weighting.size(); ++i)
	{
		IMF_Weighting[i] *= IMF_Normalisation;
	}
}

