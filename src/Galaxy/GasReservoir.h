#pragma once
#include <vector>
#include "../GenericFunctions/Logger.h"
#include "../Options.h"
#include <numeric>
class GasReservoir
{
	public:
		double Mass;
		
		double ColdMass;
		double HotMass;
		
		//chemicals are always stored in fractional form
		std::vector<double> ColdChemicals;
		std::vector<double> HotChemicals;
		
		GasReservoir();
		GasReservoir(Options * opts);
		GasReservoir(Options * opts, std::vector<double> coldChemicals);
		GasReservoir(Options * opts, std::vector<double> coldChemicals, std::vector<double> hotChemicals);
		
		void GiveTo(GasReservoir * recievingGas,double coldMass, double hotMass);
		void GiveTo(GasReservoir * recievingGas,double mixedMass);
		void TakeFrom(GasReservoir * givingGas, double coldMass, double hotMass);
		void TakeFrom(GasReservoir * givingGas, double mixedMass);
		void Deplete(double coldMass, double hotMass);
		void AddTo(GasReservoir * acceptor);
		void SubtractFrom(GasReservoir * loser);
		void SetPrimordial(double mass);
		
		GasReservoir operator+(const GasReservoir& b);
		GasReservoir operator-(const GasReservoir& b);
		
		void Print();
		
	private:
		Options * Opts;
};
