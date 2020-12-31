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

class Galaxy
{
	public:
		std::vector<Ring> Rings;
		
		double GasScaleLength(double t);
		double InfallRate(double t);
		
		Galaxy(Options * opts);
		
		void Evolve();
		
		GasReservoir IGM;
		double GasMass;
	private:
		Options * Opts;
		YieldGrid Demosthenes;
		
		void UpdateGasMass(double t);
		double ColdGasMass();
		double HotGasMass();
		
		GasReservoir PullIGM(double mass);
		
		//saving and logging stuff
		void OpenLogs();
		void CloseLogs();
		 
		void SaveState(double t);
		std::fstream GalaxyState;
		
};
