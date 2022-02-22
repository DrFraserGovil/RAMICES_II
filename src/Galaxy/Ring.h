#pragma once
#include <vector>
#include <iomanip>
#include "../Parameters/InitialisedData.h"
#include "../Gas/GasReservoir.h"
#include "../Stars/StarReservoir.h"
#include "../Stars/IMF.h"
#include "../Stars/SLF.h"
class Ring
{
	public:
		//! Initialises itself into a primordial state
		Ring(int radiusIndex, double mass, InitialisedData & data);
	
		double Mass();
		const double Radius;
		const double Width;
		double Area;
		//Relic Reservoir
		StarReservoir Stars;
		GasReservoir Gas;
		
		void MakeStars();
		void KillStars(int time);
		void Cool();
		void TimeStep(int t);
		void UpdateMemory(int t);
		
		void SaveChemicalHistory(int t, std::stringstream & absoluteStreamCold, std::stringstream & logarithmicStreamCold, std::stringstream & absoluteStreamHot, std::stringstream & logarithmicStreamHot);
		
		
		double SelectionEffect(double Mv, double age);
		
		void ComputeSelectionFunction(const double brightLimit, const double dimLimit);
		void MetCheck(const std::string & location);
	private:
		
		//~ std::vector<GasReservoir> PreviousEnrichment;
		int RadiusIndex;
		
		
	
		InitialisedData & Data;
		const GlobalParameters & Param;

		std::vector<std::vector<double>> ColdBuffer;
		std::vector<std::vector<double>> HotBuffer;
		
		std::vector<std::vector<double>> SelectionGrid;
		double MinMv;
		double MaxMv;
};
