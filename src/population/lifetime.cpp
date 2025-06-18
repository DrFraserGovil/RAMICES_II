#include "lifetime.h"
#include "../settings/SimulationSettings.h"
#include "../utility/Archiver.h"
#include <filesystem>
using namespace Stellar;

Lifetime::Lifetime()
{
	LOG(DEBUG) << "Initialising Stellar::Lifetime object";
	auto lifeTimeFile = Settings.Stellar.LifetimeFile.Value();
	bool fileExists = std::filesystem::exists(lifeTimeFile);

	if (!fileExists || Settings.Stellar.ForceIsochroneRecompute)
	{
		if (fileExists)
		{
			LOG(WARN) << "A forced isochrone recompute has been called. This will overwrite the existing lifetime file at " << lifeTimeFile;
		}
		CallIsochroneGenerator(Settings.Stellar.IsochroneDirectory,lifeTimeFile);
	}

	InitialiseFromFile(lifeTimeFile);
	PrecomputeSlices(Settings.System.SimulationDuration,Settings.System.SimulationResolution);
}


void Lifetime::InitialiseFromFile(std::string filename)
{
	LOG(DEBUG) << "Reading stellar lifetimes from file object";
	Archiver::Archive output(filename, Archiver::ArchiveMode::Read);

	auto fileDirectory = output.ListFiles();
	if (std::find(fileDirectory.begin(),fileDirectory.end(),"manifest.dat")==fileDirectory.end())
	{
		LOG(ERROR) << filename << " is not a valid corrupted lifetime file. You may need to run without the " << Settings.Stellar.LifetimeFile.GetTrigger() << " option to force a reconstruction";
		throw std::runtime_error("Invalid input file detected");
	}
	std::vector<std::string> files;
	LogZ.resize(0);
	output.ForTabularLineIn<double,std::string>("manifest.dat"," ",[&](auto line){
		LogZ.push_back(std::get<0>(line));
		files.push_back(std::get<1>(line));
	});

	LOG(DEBUG) << "Read in isochrone metallicities " << MakeString(LogZ);

	int i = 0;
	for (auto file : files)
	{
		IsochroneLifetimes iso(LogZ[i]);
		output.ForTabularLineIn<double,double>(file," ",[&](auto line){
			iso.Add(std::get<0>(line), std::get<1>(line));
		});
		Isochrones.push_back(iso);
		LOG(DEBUG) << "Isochrone log(Z)=" << iso.LogZ << " loaded with " << iso.InitialMass.size() << " points between " << iso.InitialMass[0] << " and " << iso.InitialMass.back();
		i+=1;
	}
}

void Lifetime::CallIsochroneGenerator(std::string isochroneDirector, std::string outputName)
{
	LOG(ERROR) << "The functionality to generate isochrones from within RAMICES does not yet exist";
	throw std::runtime_error("Invalid program state due to lazy programmers");
	//Do some crazy isochrone parsing stuff
}


double inline gFunc(double x, double Delta, double delta,double Tm, double Tp)
{
	if (x < Tm) return 0;
	if (x > Tp) return delta;

	if (x < Delta) return pow(x-Tm,2)/(2*delta);
	else return delta - pow(Tp - x,2)/(2*delta);

}

void Lifetime::PrecomputeSlices(double simulationDuration, int simulationResolution)
{
	double delta = simulationDuration/simulationResolution;
	Slices.resize(0);
	for (int zIndex = 0; zIndex < LogZ.size(); ++zIndex)
	{
		auto & iso = Isochrones[zIndex];
		std::vector<IntegralSlice> slices;
		for (int i = 0; i < simulationResolution; ++i)
		{
			IntegralSlice slice;
			double Delta = i * delta; //yes delta & Delta is confusing -- it is being used for equality with the notes which actually use the symbols!

			double Tm = Delta - delta;
			double Tp = Delta + delta;

			double prevMass = 0;
			double prevLifetime = 999;


			//this approach is notably inefficient. However, it is always guaranteed to catch t
			for (int j = 0; j < iso.Lifetime.size(); ++j)
			{
				auto [tau_min,tau_max] = std::minmax(prevLifetime,iso.Lifetime[j]);

				bool outsideRegion = (tau_min > Tp) || (tau_max < Tm);
				
				if (!outsideRegion)
				{
					double F = (gFunc(tau_max,Delta,delta,Tm,Tp) - gFunc(tau_min,Delta,delta,Tm,Tp))/(tau_max - tau_min);
					slice.Add(prevMass,iso.InitialMass[j],F);
				}
				prevMass = iso.InitialMass[j];
				prevLifetime = iso.Lifetime[j];
			}
			slices.push_back(slice);
		}
		Slices.push_back(slices);
	}

	
}