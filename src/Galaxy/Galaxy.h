#pragma once
#include "../Parameters/InitialisedData.h"
#include "Ring.h"
#include "../Gas/GasReservoir.h"
#include "../Stars/IMF.h"
#include "../Stars/SLF.h"
#include <sstream>

#include <thread>
#include <future>

enum ParallelJob {RingStep, Scattering};

class Galaxy
{
	public:
		Galaxy(InitialisedData & Data);
		void Evolve();
	
		std::vector<Ring> Rings;
	private:
		
		std::vector<std::thread> Threads;
		GasReservoir IGM;
		const GlobalParameters & Param;
		
		void LaunchParallelRings(int time,ParallelJob type);
		
		//Infall Stuff
		double GasScaleLength(double t);
		double InfallMass(double t);
		void InsertInfallingGas(int ring, double amount);
		void Infall(double t);
		
		//Star Formation
		
		void RingEvolve(int timestep,int ringStart, int ringEnd);
		void ScatterStep(int timestep, int ringStart, int ringEnd);

	
		

		void ScatterYields(int time);
		double PredictSurfaceDensity(double radius,double width, double totalGasMass, double scalelength);
		double GasMass();
		double ColdGasMass();
		double StarMass();

		double RelicMass();
		double Mass();
		void SaveState(double t);
		void SaveState_Mass(double t);
		void SaveState_Enrichment(double t);
		void SaveState_Events(double t);
		
		static std::string MassHeaders(); 
		
		InitialisedData & Data;
		

};
