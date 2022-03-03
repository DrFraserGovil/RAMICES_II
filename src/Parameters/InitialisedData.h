#pragma once
class YieldGrid;
class SimpleYield;
#include <random>
#include "../Stars/IMF.h"
#include "../Stars/SLF.h"
#include "../Stars/IsochroneTracker.h"
#include "../Yields/YieldGrid.h"
#include "../Yields/SimpleYield.h"
#include "GlobalParameters.h"

//! These will act like globally-defined functions, but have the scope for modifying themselves as they go along
class InitialisedData
{
	public:
		const IMF_Functor IMF;
		SLF_Functor SLF;
		const GlobalParameters & Param;
		const YieldGrid CCSNYield;
		const YieldGrid AGBYield;
		const YieldGrid ECSNYield;
		const SimpleYield SNIaYield;
		const SimpleYield NSMYield;
		IsochroneTracker Isochrones;
		InitialisedData(const GlobalParameters & param);
	
		void Log(const std::string & input) const;
		void Log(const std::string & input, int importance) const;
		void LogFlush() const;
		void UrgentLog(const std::string & input) const;
	
		void ProgressBar(int & currentBars, int currentStep, int totalSteps);
		double NormalDist();
		double NormalDist(double mu, double sigma);
		double UniformDist(double lowerBound, double upperBound);
	private:
		std::default_random_engine generator;
		std::normal_distribution<double> distribution;
	
	
};
