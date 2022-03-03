#pragma once
#include "../Parameters/GlobalParameters.h"
#include "Isochrone.h"
#include <filesystem>
#include <random>



class IsochroneTracker
{
	
	
	public:
		IsochroneTracker(const GlobalParameters & param);
		void Construct();
		
		IsochroneCube GetProperties(int mass, double z, double age);
		
	private:
		const GlobalParameters Param;
		void IsoLog(std::string val);
		void ParseFile(std::string file);
		std::vector<double> CapturedZs;
		std::vector<double> CapturedTs;
		std::vector<std::vector<std::vector<IsochroneEntry>>> Grid;
		std::vector<std::vector<std::vector<IsochroneEntry>>> UnsortedGrid;
		bool isTimeLogUniform;
		double DeltaLogT;
		
		std::default_random_engine generator;
		std::normal_distribution<double> distribution;
		
		double NormalSample(double mu, double sigma);
		double UniformSample(double lowerBound, double upperBound);
		
		void ExtractSample(IsochroneCube & output, int sampleMass, double sampleZ, double sampleAge);
};
