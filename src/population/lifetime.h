#pragma once
#include <vector>


namespace Stellar
{
	class IsochroneLifetimes
	{
		public:
			IsochroneLifetimes(double logZ) : LogZ(logZ){};
			void Add(double m, double t) {InitialMass.push_back(m); Lifetime.push_back(t);};
			double LogZ;
			std::vector<double> InitialMass;
			std::vector<double> Lifetime;
	};

	class IntegralSlice
	{
		public:
			std::vector<std::pair<double,double>> Mass;
			std::vector<double> FValue;
	};

	class Lifetime
	{
		public:
			Lifetime(){};
			const std::vector<IntegralSlice> & getSlice(int zIndex);
			std::vector<double> LogZ;
		private:
			std::vector<IsochroneLifetimes> Isochrones;
			std::vector<std::vector<IntegralSlice>> Slices;

			void InitialiseFromFile(std::string filename);
			void InitialiseFromIsochrones();
			void SaveToFile();

	};

	
}