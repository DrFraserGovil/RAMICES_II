#pragma once
#include "../Parameters/InitialisedData.h"
#include "Ring.h"
#include "../Gas/GasReservoir.h"
#include "../Stars/IMF.h"
#include "../Stars/SLF.h"
#include "MigrationMatrix.h"
#include <sstream>
#include <iomanip>
#include <thread>
#include <future>

enum ParallelJob {RingStep, Compounding, Scattering, AssignIsochrones,Synthesis,Selection};

class Galaxy
{
	public:
		Galaxy(InitialisedData & Data);
		void Evolve();
		void SynthesiseObservations();
		std::vector<Ring> Rings;
	private:
		
		std::vector<std::thread> Threads;
		std::vector<MigrationMatrix> Migrator;
		GasReservoir CGM;
		const GlobalParameters & Param;
		
		void LaunchParallelOperation(int time,int nOperations,ParallelJob type);
		
		//Infall Stuff
		double GasScaleLength(double t);
		double InfallMass(double t);
		void InsertInfallingGas(int ring, double amount);
		void Infall(double t);
		
		//Star Formation
		
		void RingEvolve(int timestep,int ringStart, int ringEnd);
		void ScatterYields(int timestep, int ringStart, int ringEnd);
		void ScatterGas(int timestep);
	
		
		void ComputeScattering(int t);
		void CompoundScattering(int currentTime,int timeStart, int timeEnd);
		
		void AssignMagnitudes(int time, int ringstart, int ringend);
		
		double PredictSurfaceDensity(double radius,double width, double totalGasMass, double scalelength);
		double GasMass();
		double ColdGasMass();
		double StarMass();
		void CGMOperations();

		double RelicMass();
		double Mass();
		void SaveState(double t);
		void SaveState_Mass(double t);
		void SaveState_Enrichment(double t);
		void SaveState_Events(double t);
		
		static std::string MassHeaders(); 
		
		InitialisedData & Data;
		
		std::vector<double> RingMasses;
		
		void ComputeVisibilityFunction();
		void SelectionFunction(int ringstart, int ringend, int threadID);
		void StellarSynthesis(int ringstart, int ringend,int threadID);
		std::vector<std::string> SynthesisOutput;
		std::vector<double> SynthesisProgress;
		
		double DimmestStar;
		double BrightestStar;
		int ParallelBars = 0;
		
};
