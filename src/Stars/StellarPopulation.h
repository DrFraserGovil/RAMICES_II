#pragma once
#include <vector>
#include "../Parameters/GlobalParameters.h"
#include "IMF.h"
#include "../Gas/GasStream.h"
#include "../Gas/Gas.h"
#include "SLF.h"
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
		StellarPopulation(const IMF_Functor & imf, SLF_Functor & SLF, const GlobalParameters & param);
	
		void PrepareIMF();
		void FormStars(double formingMass, int timeIndex, double formingMetallicity);
		double Mass();
		IsoMass & Relic();
		const IsoMass & Relic() const;
		IsoMass & operator[](int i);
		const IsoMass & operator[](int i) const;
		bool Active();
		void Death(int time);
	private:
		const GlobalParameters & Param;
		IsoMass ImmortalStars;
		std::vector<IsoMass> Distribution;

		const IMF_Functor & IMF; 
		SLF_Functor & SLF;
		
		bool IsLifetimeMonotonic;
		bool IsDepleted;
		int DepletionIndex;
		
		double internal_MassCounter;
		
		void MonotonicDeathScan(int time);
		void FullDeathScan(int time);
		
		Gas TempGas;
};
