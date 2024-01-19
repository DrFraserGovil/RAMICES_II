#pragma once
#include <vector>

#include "../Parameters/InitialisedData.h"
#include "IMF.h"
#include "../Gas/GasReservoir.h"
#include "RemnantPopulation.h"
#include "SLF.h"
#include "StarEvents.h"
#include "IsochroneTracker.h"

#include "../../../Agama/src/units.h"
#include "../../../Agama/src/potential_factory.h"
#include "../../../Agama/src/actions_torus.h"
//! A simple struct for tracking the number of stars of a given mass



class IsoMass
{
	public:
		int MassIndex;
		double Count;
		double Metallicity;
		int BirthIndex;
		int DeathIndex;
		IsochroneCube Isochrone;
		IsoMass();
		IsoMass(double n, int m, double z, int birth, int death);
};


class StellarPopulation
{
	public:
		StellarPopulation(InitialisedData & data, int parentRing);
	
		void PrepareIMF();
		int BirthRadius;
		double Metallicity; 
		int BirthIndex;
		//!Returns the number of stars formed (spread across all mass grids)
		int FormStars(double formingMass, int timeIndex, GasReservoir & formingGas);
		double Mass();
		IsoMass & Relic();
		const IsoMass & Relic() const;
		IsoMass & operator[](int i);
		const IsoMass & operator[](int i) const;
		bool Active();
		
		void Death(int time, std::vector<GasReservoir> & TemporalYieldGrid, RemnantPopulation & remnants, StarEvents & EventRate);
		std::vector<IsoMass> Distribution;
		IsoMass ImmortalStars;
		
		std::vector<GasStream> BirthGas;
		
		std::string CatalogueHeaders();
		std::string CatalogueEntry(std::vector<int> popEntry, 
								   int m, 
								   double currentGuidingRadius, 
								   double birthRadius,double age, 
								   const potential::PtrPotential& pot, 
								   const units::InternalUnits& unit, 
								   std::vector<double> Mv,
								   int& numberAccepted) const;
		double Age;
	private:
		const GlobalParameters & Param;



		const IMF_Functor & IMF; 
		SLF_Functor & SLF;
		const YieldGrid & CCSNYield;
		const YieldGrid & ECSNYield;
		const YieldGrid & AGBYield;
		InitialisedData & Data;
		bool IsLifetimeMonotonic;
		bool IsDepleted;
		int DepletionIndex;
		
		double internal_MassCounter;
		
		void MonotonicDeathScan(int time,std::vector<GasReservoir> & YieldGrid, RemnantPopulation & remnants, StarEvents & eventRate);
		void FullDeathScan(int time);
		
		void RecoverMatter(int time,int nstars, int mass, GasReservoir & temporalYieldGrid, RemnantPopulation & remnants);
		
		// bool RejectStars(double mass, double Age, const coord::PosVelCar& PS, double Mv);

		Gas TempGas;
};
