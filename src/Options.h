#ifndef OPTIONS

#define OPTIONS
#include <string>
#include <vector>
//the options class holds a number of stuff for global variable-type access and definitions of variables which are used everywhere. It also interfaces with the command-line parser, enabling command line-level access to internal variables.
	//for ease of use, the options are split into multiple sub-options, to prevent too many variables being stored in a single class





class SimulationOptions
{
	//the simulation suboptions contains variables associated with the base-level information about the sumulation - the number of cores to access, the timesteps etc. 
	
	public:
		std::string FileRoot;
		int ParallelThreads;
		int NRings;
		bool UseOldYieldGrid;
		bool UseSpinningYields;
		
		int LoggingLevel;
		bool LogToFile;
		bool LogToTerminal;
		
		SimulationOptions();
};

class ElementOptions
{
	//the elemental suboptions contains variables associated with the elemental IDs and the number of elements being tracked in the simulation
	
	public:
		int NSpecies;
		
		int HydrogenID;
		int HeliumID;
		int MetalsID;
		int IronID;
		int OxygenID;
		int MagnesiumID;
		int CarbonID;
		int SiliconID;
		int CalciumID;
		int ManganeseID;
		int ChromiumID;
		int CobaltID;
		int EuropiumID;
		
		std::vector<std::string> ElementNames;
		
		double SolarHydrogen;
		double SolarHelium;
		double SolarMetals;
		double SolarIron;
		double SolarOxygen;
		double SolarMagnesium;
		double SolarCarbon;
		double SolarSilicon;
		double SolarCalcium;
		double SolarManganese;
		double SolarChromium;
		double SolarCobalt;
		double SolarEuropium;
		
		int OrfeoID;
		int MarigoID;
		int LimongiID;
		int MaederID;
		int MixedID;
		
		double maxInterpolationFactor;
		ElementOptions();
};

class StellarOptions
{
	//the stellar suboptions contains variables associated with stars + their remnants
	
	public:
		double MaxMass; 
		double MinMass;
		double MinZ;
		double MaxZ;
		double DeathLossFraction;
		StellarOptions();
	
};

class RemnantOptions
{
	public:
		double SNIaFraction;
		double NSMFraction;
		
		
		
		RemnantOptions();
};

class CollapsarOptions
{
	public:
		double EffectiveYield;
		double MetallictyCutoff;
		double CutoffWidth;
	
		CollapsarOptions();
};

class ThermalOptions
{
	//thermal suboptions contain variables which deal with the thermal subroutines - cooling timescales injection fractions etc. 
	
	public:
		double HotFractionCCSN;
		double HotFractionNSM;
		double HotFractionSNIa;
		
		double GasCoolTimeScale;
	
		ThermalOptions();
	
};

class GalaxyOptions
{
	//the galaxy suboptions contians variables associated with the galaxy as a whole, such as the maximum radius, and various mass/infall properties
	
	public:
		
		GalaxyOptions();
	
};


class Options
{
	
	
	public:
	
		SimulationOptions Simulation;
		ElementOptions Element;
		StellarOptions Stellar;
		ThermalOptions Thermal;
		GalaxyOptions Galaxy;
		RemnantOptions Remnant;
		CollapsarOptions Collapsar;
		
		Options();
};
#endif
