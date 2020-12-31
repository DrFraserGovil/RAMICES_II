#pragma once

#include <vector>
#include <math.h>
#include "../Options.h"
#include "../GenericFunctions/Logger.h"
#include "../ChemicalAssemblers/YieldGrid.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include "Ring.h"

class Ring;//placeholder to allow parent/child and child/parent interaction

struct MassKeeper
{
	double Total()
	{
		return HotGas + ColdGas + Stellar + Remnant;
	}
	double HotGas;
	double ColdGas;
	double Stellar;
	double Remnant;
};

class Galaxy
{
	public:
		std::vector<Ring> Rings;
		
		double GasScaleLength(double t);
		double InfallRate(double t);
		
		Galaxy(Options * opts);
		
		void Evolve();
		
		GasReservoir PullIGM(double mass);
		void PushIGM(int donorID, double hotmass, double coldmass);
		
		MassKeeper Mass;
	private:
		Options * Opts;
		YieldGrid Demosthenes;
		GasReservoir IGM;
		void UpdateGasMass(double t);
		double ColdGasMass();
		double HotGasMass();
		
		
		
		//saving and logging stuff
		void OpenLogs();
		void CloseLogs();
		 
		void ChemicalEvolution(double t);
		 
		std::vector<std::string> PropertyHeaders();
		std::vector<double> ReportProperties(double t);
		void SaveState(double t);
		std::fstream RingState;
		std::fstream GalaxyState;
		double GasExcess;
};
