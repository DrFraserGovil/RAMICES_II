#include "Options.h"

Options::Options()
{
	//I don't think Options needs to do anything other than initialise suboptions. I might be wrong. 
	
}

SimulationOptions::SimulationOptions()
{
	FileRoot = "Output/";
	ParallelThreads = 1;
	NRings = 100;
	UseOldYieldGrid = false;
	
	LoggingLevel = 3;
	LogToFile = true;
	LogToTerminal = true;
}

ElementOptions::ElementOptions()
{
	NSpecies = 13;
	
	HydrogenID = 0;
	HeliumID = 1;
	MetalsID = 2;
	IronID = 3;
	OxygenID = 4;
	MagnesiumID = 5;
	CarbonID = 6;
	SiliconID = 7;
	CalciumID = 8;
	ManganeseID = 9;
	ChromiumID = 10;
	CobaltID = 11;
	EuropiumID = 12;
	
	ElementNames = {"Hydrogen", "Helium", "Z", "Iron", "Oxygen", "Magnesium", "Carbon", "Silicon", "Calcium", "Manganese", "Chromium", "Cobalt", "Europium"};
	SolarHydrogen = 0.7;
	SolarHelium = 0.29;
	SolarMetals = 0.014;
	SolarIron = 0.00177087549;
	SolarOxygen = 0.01186096386081467955;
	SolarMagnesium = 0.0009124545511693467503;
	SolarCarbon = 0.004257760671428572;
	SolarSilicon =  0.0009934774898542858;
	SolarCalcium = 9.163470611428572e-05;
	SolarManganese = 1.3500899e-05;
	SolarChromium = 2.4322227342857144e-05;
	SolarCobalt = 4.90740625e-06;
	SolarEuropium = 0.0000001;
}

StellarOptions::StellarOptions()
{
	MinMass = 0.5;
	MaxMass = 100;
	MinZ = 10e-6;
	MaxZ = 0.08;
	DeathLossFraction = 0.45;
}

RemnantOptions::RemnantOptions()
{
	SNIaFraction = 0.05;
	NSMFraction = 0.0001;	
}

CollapsarOptions::CollapsarOptions()
{
	EffectiveYield = 0.002;
	MetallictyCutoff = 0.1;
	CutoffWidth = 0.02;
}

ThermalOptions::ThermalOptions()
{
	HotFractionCCSN = 0.7;
	HotFractionNSM = 0.4;
	HotFractionSNIa = 0.99;
	
	GasCoolTimeScale = 1.0;
}


GalaxyOptions::GalaxyOptions()
{
	
}
