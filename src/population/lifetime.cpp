#include "lifetime.h"
#include "../settings/SimulationSettings.h"
#include "../utility/Archiver.h"
using namespace Stellar;

Lifetime::Lifetime()
{
	if (Settings.Stellar.LifetimeFile.Value() != "__none__")
	{
		InitialiseFromFile(Settings.Stellar.LifetimeFile.Value());

		
	}
	else
	{
		InitialiseFromIsochrones();
		if (Settings.Stellar.LifetimeFileOutput.Value() != "__none__")
		{
			SaveToFile();
		}
	}
}


void Lifetime::SaveToFile()
{
	Archiver::Archive output(Settings.Stellar.LifetimeFileOutput.Value(), Archiver::ArchiveMode::Write);

	output.ActivateStream("manifest.dat");
	int i = 0;
	for (auto logZ : LogZ)
	{
		output << logZ << " lifetime_" << i<<".dat\n";
		++i;
	}
	output.DeactivateStream();

	for (int i = 0; i < Isochrones.size(); ++i)
	{
		output.ActivateStream("lifetime_" + std::to_string(i) + ".dat");
		for (int j = 0; j < Isochrones[i].InitialMass.size(); ++j)
		{
			output << Isochrones[i].InitialMass[j] << " " << Isochrones[i].Lifetime[j] << "\n";
		}
		output.DeactivateStream();
	}

	output.Close();
}

void Lifetime::InitialiseFromFile(std::string filename)
{
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

	int i = 0;
	for (auto file : files)
	{
		IsochroneLifetimes iso(LogZ[i]);
		output.ForTabularLineIn<double,double>(file," ",[&](auto line){
			iso.Add(std::get<0>(line), std::get<1>(line));
		});
		Isochrones.push_back(iso);
	}
}

void Lifetime::InitialiseFromIsochrones()
{
	//Do some crazy isochrone parsing stuff
}