#pragma once
#include <vector>


namespace Stellar
{
	class IsochroneLifetimes
	{
		public:
			IsochroneLifetimes(double logZ) : LogZ(logZ){InitialMass.resize(0); Lifetime.resize(0);};
			void Add(double m, double t) {
				InitialMass.push_back(m); Lifetime.push_back(t);};
			double LogZ;
			std::vector<double> InitialMass;
			std::vector<double> Lifetime;
	};

	class IntegralSlice
	{
		public:
			IntegralSlice(){Mass.resize(0); FValue.resize(0);}
			std::vector<std::pair<double,double>> Mass;
			std::vector<double> FValue;
			void Add(double lowerMass, double upperMass, double F){Mass.push_back(std::pair<double,double>{lowerMass,upperMass}); FValue.push_back(F);};
	};

	class Lifetime
	{
		public:
			Lifetime();
			const std::vector<IntegralSlice> & getSlice(int zIndex);
			std::vector<double> LogZ;
		private:
			std::vector<IsochroneLifetimes> Isochrones;
			std::vector<std::vector<IntegralSlice>> Slices;

			void InitialiseFromFile(std::string filename);
			void CallIsochroneGenerator(std::string isochroneDirector, std::string outputName);

			void PrecomputeSlices(double simulationDuration, int simulationResolution);
	};

	
}