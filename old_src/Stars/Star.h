#pragma once
#include "../Options.h"

class StarSet
{
	//cobirthed stars are a set of N stars born at the same time with the same mass and metallicities
	public:
		double Mass;
		int NStars;
		
		double Metallicity;
		
		StarSet();
		StarSet(Options * opts);
		StarSet(Options * opts, double mass, double z, int N);
	private:
		
		
		double RemainingLifeTime;
		double BirthTime;
		double LifeTime;
		
		Options * Opts;
	
};
