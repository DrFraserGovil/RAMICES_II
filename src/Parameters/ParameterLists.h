#pragma once
#include "JSL.h"
#include "List.h"
#include "EnumSets.h"
using JSL::Argument;


//!The MetaValues contains variables associated with the base-level information about the sumulation - the number of cores to access, the timesteps etc. 
class MetaValues : public ParamList
{
	
	public:
		//! Controls whether the funky ASCII welcome message is played at the beginning of the code
		Argument<int> Verbosity = Argument<int>(1, "verbose");
		
		//!The maximum number of parallel threads which can be active at any given time
		Argument<int> ParallelThreads = Argument<int>(3,"thread");
		
		//!The top level timestep used in the main chemical loop
		Argument<double> TimeStep  = Argument<double>(0.01,"timestep");
		
		//~ //!The total duration of the chemical simulation
		Argument<double> SimulationDuration = Argument<double>(10.0,"duration");
		
		//!The number of hashes used to display progress bars
		Argument<int> ProgressHashes = Argument<int>(32,"progress-hashes");
		
	
		//!The number of timesteps in the simulation, computed from #SimulationDuration and #TimeStep
		int SimulationSteps;
		
		//!Boring constructor -- slots in the relevant arguments into the ParamList::argPointer array.
		MetaValues()
		{
			argPointers = {&Verbosity,&ParallelThreads,&TimeStep,&SimulationDuration,&ProgressHashes};
		};
		
		//! An overload of a normally empty function. Computes the value of #SimulationSteps
		virtual void Initialise(std::string resourceRoot);
};


class OutputValues : public ParamList
{
	public:
		//!The name of the output directory into which the output will be saved
		Argument<std::string> Root =  Argument<std::string>("Output/","output");
			
		//!The name for the output config file which would replicate this simulation
		Argument<std::string> Config = Argument<std::string>("rerun.config","config-out");
		
		Argument<std::string> YieldSubdir = Argument<std::string>("Yields/","yield-dir");
		
		//!The name of the file containing galactic-scale mass information
		Argument<std::string> GalaxyMassFile = Argument<std::string>("Mass.dat","galaxy-mass-file");
		
		Argument<std::string> EventRateFile = Argument<std::string>("Events.dat","event-rate-file");
		
		//!The ring-star data identifier
		Argument<std::string> StarFile = Argument<std::string>("StellarCatalogue.dat","ring-data-stars");
		
		
		
		//!The enrichment file identifier
		Argument<std::string> ChemicalPrefactor  = Argument<std::string>("Enrichment_","enrichment-base");
		
		//!The cold gas filename
		Argument<std::string> ColdGasDataFile= Argument<std::string>("ColdGas.dat","enrichment-cold");
		
		//!The hot gas filename
		Argument<std::string> HotGasDataFile= Argument<std::string>("HotGas.dat","enrichment-hot");
		
		std::string LogarithmicColdGasFile;
		std::string AbsoluteColdGasFile;
		std::string LogarithmicHotGasFile;
		std::string AbsoluteHotGasFile;
		
		//!Boring constructor -- slots in the relevant arguments into the ParamList::argPointer array.
		OutputValues()
		{
			argPointers = {&Root, &GalaxyMassFile,&StarFile, &ChemicalPrefactor,&ColdGasDataFile,&HotGasDataFile,&YieldSubdir};
		};
		
		//! An overload of a normally empty function. Goes through and creates the necessary directory structure 
		virtual void Initialise(std::string resourceRoot);
};

class ResourceValues : public ParamList
{
	public:
		//! The location of the funky ASCII welcome messgae
		Argument<std::string> WelcomeFile = Argument<std::string>("welcome.dat","welcome-file");
		
		//!The location of the directory which the code looks for its expected resource file structure
		Argument<std::string> ResourceRoot  = Argument<std::string>("Resources/","resource");
		
		//!The location of the directory within ResourceRoot which houses the stellar yield data
		Argument<std::string> YieldRoot = Argument<std::string>("ChemicalData/","yield-root");
		
		
		Argument<std::string> IsochroneDirectory = Argument<std::string>("Isochrones/","iso-dir");
		
		Argument<std::string> LifeTimeFile = Argument<std::string>("LifetimeGrid.dat","lifetime-file");
		
		Argument<std::string> IsochroneRepository = Argument<std::string>("NewPadova/","iso-repo");
		//!Boring constructor -- slots in the relevant arguments into the ParamList::argPointer array.
		ResourceValues()
		{
			argPointers = {&WelcomeFile, &ResourceRoot,&YieldRoot,&IsochroneDirectory,&LifeTimeFile,&IsochroneRepository};
		};
		
		//! An overload of a normally empty function. Goes through and creates the necessary directory structure 
		virtual void Initialise(std::string resourceRoot);
	
};

//! The elemental suboptions contains variables and data associated with the solar abundances (and where to locate them), and how to extract and extrapolate the yield data from files. 
class ElementValues : public ParamList
{
	
	public:
		
		//! Human readable names for the elements, in the order associated with the ElementIDs. These names are primarily elemental symbols, except Metals, which uses "Z"
		std::vector<std::string> ElementNames;

		// The values of Z associated with each element
		std::vector<int> ProtonCounts;

		//! Solar abundances (in mass units) of the elements, in the order associated with the ElementIDs
		std::vector<double> SolarAbundances;
		
		//! The file in which the solar abundances can be found as a csv
		Argument<std::string> SolarAbundanceFile = Argument<std::string>("ChemicalData/SolarAbundances_Maria.dat","solar-values-file"); 

		//! The column of the solar abundance files which contains the ElementName for cross matching
		Argument<int> SolarAbundanceFileNameColumn = Argument<int>(0,"solar-values-name-col");
		
		//! The column of the solar abundance file which contains the relevant solar abundance value to be saved to memory
		Argument<int> SolarAbundanceFileDataColumn = Argument<int>(3,"solar-values-data-col");
		
		
		
		//!Boring constructor -- slots in the relevant arguments into the ParamList::argPointer array
		ElementValues()
		{
				argPointers = {&SolarAbundanceFile, &SolarAbundanceFileDataColumn, &SolarAbundanceFileNameColumn};
		};
		
		//! An overload of a normally empty function. Loads in the values fo the solar abundance data file into the SolarAbundances vector.
		virtual void Initialise(std::string resourceRoot);
		
		//! A fairly dumbly-written function which sorts the elemental symbols in ElementNames into the order specified by the global id-enum.
		void GiveElementsNames();
		
};

//! The subset of values associated with stars + their remnants
class StellarValues : public ParamList
{
	
	public:
		//!Minimum stellar mass that IMF can generate
		Argument<double> MaxStellarMass = Argument<double>(100,"mass-max");
		
		//!Maxmimum stellar mass that IMF can generate
		Argument<double> MinStellarMass = Argument<double>(0.1,"mass-min");
		
		//!Mass of stars which we consider immortal without checking their isochrones
		Argument<double> ImmortalMass = Argument<double>(0.3,"mass-immortal");
		
		//!Number of points along the stellar mass grid
		Argument<int> MassResolution = Argument<int>(199,"mass-resolution");
		
		//! A grid which holds the masses onto which all interpolation will take place. Allows for the possibility of non-uniform steps. The values are the centre of each mass divide
		std::vector<double> MassGrid;
		
		//! The corresponding widths of each interval on the mass line.
		std::vector<double> MassDeltas;
		
			
		
		//!Minimum Z that the ILM(??) can consider
		Argument<double> MinLogZ = Argument<double>(-6,"logz-min");
		
		//!Maximum Z that the ILM(??) can consider
		Argument<double> MaxLogZ = Argument<double>(-0.1,"logz-max");

		//!Z Resolution
		Argument<int> LogZResolution = Argument<int>(100,"logz-resolution");

		//! As with MassGrid, but for metallicity (assumed to be always uniform in log-space)
		std::vector<double> LogZGrid;
		double LogZDelta;

		//!The fraction of supernovae ejecta which is thrown into the CGM
		Argument<double> EjectionFraction = Argument<double>(0.45,"eject");
		
		//!For every 1 solar mass of stars which form, this fraction of gas is heated into the hot phase
		Argument<double> FeedbackFactor = Argument<double>(0.5,"mass-load");
		

		
		//! The normal Kennicutt-Schmidt power law index
		Argument<double> SchmidtMainPower = Argument<double>(1.4,"schmidt-main");
		
		//! The low-density Kennicutt-Schmidt power law index
		Argument<double> SchmidtLowPower = Argument<double>(4.0,"schmidt-low");
		
		//! The density cut for the low/high density switchover in Schmidt power law
		Argument<double> SchmidtDensityCut = Argument<double>(0,"schmidt-cut");
		
		//! The Schmidt prefactor
		Argument<double> SchmidtPrefactor = Argument<double>(2,"schmidt-factor");
		
		//! The slope of the high-mass tail fo the IMF
		Argument<double> IMF_Slope = Argument<double>(2.3,"imf-slope");
		
		
		
		//!Boring constructor -- slots in the relevant arguments into the ParamList::argPointer array
		StellarValues()
		{
			argPointers = {&MaxStellarMass, &MinStellarMass, &ImmortalMass, &MassResolution, &MinLogZ, &MaxLogZ, &LogZResolution, &EjectionFraction,&SchmidtMainPower, &SchmidtLowPower, &SchmidtDensityCut, &SchmidtPrefactor, &FeedbackFactor};
		}
		
		//! Initialises the mass grid etc.
		void Initialise(std::string resourceRoot);
};


class YieldValues : public ParamList
{
	public:
		Argument<double> TargetNi56Yield = Argument<double>(0.1,"ideal-ni56");
		Argument<double> MassOverhang = Argument<double>(5,"yield-mass-overhang");
		std::vector<std::string> ProcessNames;
		std::vector<SourceProcess> ProcessTypes;
		
		//!Time before SNIa can turn on, in Gyr
		Argument<double> SNIa_DelayTime = Argument<double>(0.2,"sn1a-delay");
		
		Argument<double> SNIa_ActiveFraction = Argument<double>(0.1,"sn1a-fraction");
		
		Argument<double> SNIa_LongFraction = Argument<double>(0.99,"sn1a-fraction-long");
		
		Argument<double> SNIa_ShortScale = Argument<double>(0.1,"sn1a-short-decay");
		
		Argument<double> SNIa_TypicalMass = Argument<double>(1.37,"sn1a-progenitor-mass");
		
		Argument<double> NSM_TypicalMass = Argument<double>(1.4,"nsm-progenitor-mass");
		
		Argument<double> SNIa_LongScale = Argument<double>(1.5,"sn1a-long-decay");
		
		Argument<double> CCSN_MassCut = Argument<double>(10,"ccsn-mass");
		
		Argument<double> ECSN_MassCut = Argument<double>(8.5,"ecsn-mass");
		Argument<double> CODwarf_MassCut = Argument<double>(3.2,"co-mass");
		Argument<double> Collapse_MassCut = Argument<double>(40,"bh-mass");
		
		//!Fraction of stars in ECSN mass range which go ECSN vs CCSN
		Argument<double> ECSN_Fraction = Argument<double>(0,"ecsn-fraction");
		
		Argument<double> NSM_DelayTime = Argument<double>(0.02,"nsm-delay");
		Argument<double> NSM_ActiveFraction = Argument<double>(0.001,"nsm-fraction");
		Argument<double> NSM_Scale = Argument<double>(10,"nsm-decay");
		
	YieldValues()
	{
		argPointers = {&SNIa_DelayTime, &SNIa_ShortScale, &SNIa_LongScale, &NSM_DelayTime, & SNIa_ActiveFraction, &SNIa_LongFraction, &CCSN_MassCut,&NSM_ActiveFraction, &NSM_Scale,&SNIa_TypicalMass,&NSM_TypicalMass, &TargetNi56Yield, &ECSN_Fraction};
	}
	//! Initialises the mass grid etc.
		void Initialise(std::string resourceRoot);
	
};

//!Thermal suboptions contain variables which deal with the thermal subroutines - cooling timescales injection fractions etc. 
class ThermalValues : public ParamList
{
	
	
	public:
		//!Fraction of CCSN ejecta which is put into the hot phase
		Argument<double> HotInjection_CCSN = Argument<double>(0.7,"fh-ccsn");
		
		//!Fraction of NSM ejecta which is put into the hot phase
		Argument<double> HotInjection_NSM = Argument<double>(0.4,"fh-nsm");
		
		//!Fraction of AGB ejecta which is put into the hot phase
		Argument<double> HotInjection_AGB = Argument<double>(0.7,"fh-agb");
		
		Argument<double> FeedbackEjectFactor = Argument<double>(0,"feedback-eject");
		
		Argument<double> ChimneyFactor = Argument<double>(0,"chimney");
		
		//!Fraction of SNIa ejecta which is put into the hot phase
		Argument<double> HotInjection_SNIa = Argument<double>(0.99,"fh-sn1a");
		
		//! The exponential timescale over which the hot gas cools into the cold gas
		Argument<double> GasCoolingTimeScale = Argument<double>(1,"cool");
	
		Argument<int> NumericalResolution = Argument<int>(30,"cool-resolution");
		
		Argument<double> DormantHotFraction = Argument<double> (1e-20,"dormant-hot-frac");
		
		Argument<double> CoolingPower = Argument<double>(1,"cooling-index");
		//!Boring constructor -- slots in the relevant arguments into the ParamList::argPointer array
		ThermalValues()
		{
			argPointers = {&HotInjection_CCSN, &HotInjection_NSM, &HotInjection_SNIa, &GasCoolingTimeScale, &HotInjection_AGB, &NumericalResolution, &DormantHotFraction,&CoolingPower, &FeedbackEjectFactor,&ChimneyFactor};
		}
};


//!Holds values associated with how matter mvoes throughout the disc
class MigrationValues: public ParamList
{
		
	public:
		//!Turns on or off the gas inflow in the disc
		Argument<bool> InflowActive = Argument<bool>(true,"inflow-on");
		//! A parameter to do with the inflow weighting scheme (icky)
		Argument<double> InflowParameterA = Argument<double>(0.33,"inflow-a");
		
		//! A parameter to do with the inflow weighting scheme (icky)
		Argument<double> InflowParameterB = Argument<double>(0.53,"inflow-b");
		
		//! The maximum amount of gas which can be moved moved between rings during the inflow portion
		Argument<double> MaxStealFraction = Argument<double>(0.95,"max-steal");
		
		//! The strength of the random-walk mixing process, in units of 1e-3 kpc^2/Gyr
		Argument<double> MarkovDispersionStrength = Argument<double>(0.2,"mixing-strength");
		
		//! The order to which the mixing matrix is computed - note that higher values allow instantaneous dispersion to higher radii
		Argument<int> DispersionOrder = Argument<int>(3,"mixing-order");
		
		Argument<double> DispersionTruncation = Argument<double>(1e-10,"mixing-truncation");
		
		//!Boring constructor -- slots in the relevant arguments into the ParamList::argPointer array
		MigrationValues()
		{
			argPointers = {&InflowParameterA,&InflowParameterB,&MaxStealFraction,&MarkovDispersionStrength,&DispersionOrder, &DispersionTruncation, &InflowActive};
		}
	
	
};

class CatalogueValues: public ParamList
{
	public:
	
		Argument<bool> SynthesisActive = Argument<bool>(true,"stellar-synthesis");
	
		//! The timesteps used to interpolate isochrones over
		Argument<double> IsochroneTimeStep = Argument<double>(0.1,"isochrone-dt");
		
		Argument<double> IsochroneMagnitudeResolution = Argument<double>(100,"isochrone-mag-resolution");
		
		Argument<double> SolarRadius = Argument<double>(8.2,"solar-radius");
		
		Argument<int> RadialResolution = Argument<int>(3,"isochrone-radial-resolution");
		Argument<int> AzimuthalResolution = Argument<int>(360,"isochrone-radial-resolution");
		
	
		
		Argument<double> VerticalHeightStart = Argument<double>(0.05,"vertical-height-z0");
		Argument<double> VerticalHeightScaling = Argument<double>(0.3,"vertical-height-scaling");
		Argument<double> VerticalHeightPower = Argument<double>(0.66,"vertical-height-power");
		
		
		Argument<int> SampleCount = Argument<int>(10,"catalogue-sample");

		//!Boring constructor -- slots in the relevant arguments into the ParamList::argPointer array
		CatalogueValues()
		{
			argPointers = {&SynthesisActive,&IsochroneTimeStep,&IsochroneMagnitudeResolution,&SolarRadius,&RadialResolution,&AzimuthalResolution,&VerticalHeightStart,&VerticalHeightScaling,&VerticalHeightPower};
		}
	
};

//!The galaxy suboptions contians variables associated with the galaxy as a whole, such as the maximum radius, and various mass/infall properties
class GalaxyValues : public ParamList
{
	
	
	public:
		//! The number of annuli into which the galaxy is split
		Argument<int> RingCount = Argument<int>(100,"rings");
		
		
		//! The cutoff radius of the galaxy
		Argument<double> Radius = Argument<double>(20.0,"radius");
		
		Argument<bool> UsingVariableRingWidth = Argument<bool>(false,"variable-ring-width");
		
		//!Width of the innermost ring (kpc)
		Argument<double> Ring0Width = Argument<double>(0.05,"inner-ring-width");
		
		Argument<bool> CGMAbsorbing = Argument<bool>(true,"cgm-absorb");
		
		std::vector<double> RingRadius;
		std::vector<double> RingWidth;
		
		//! Initial in-situ mass of the galaxy (assumed to be 100% gas)
		Argument<double> PrimordialMass = Argument<double>(2,"M0");
		
		
		//! Fraction of primordial gas which is hot
		Argument<double> PrimordialHotFraction = Argument<double>(0,"primordial-hot");
		
		//!Initial Mass of the CGM Reservoir
		Argument<double> CGM_Mass = Argument<double>(200,"cgm-mass");
		
		//!Final Mass of the CGM Reservoir, mass added is CGM_Mass_End - CGM_Mass
		Argument<double> CGM_Mass_End = Argument<double>(200,"cgm-mass-end");
		

		//! The initial exponential scale length of the galaxy
		Argument<double> MinScaleLength = Argument<double>(0.75,"scale-length-min");
	
		//! The exponential scale length that the galaxy achieves at ScaleLengthFinalTime
		Argument<double> MaxScaleLength = Argument<double>(3.75,"scale-length-max");
		
		//! The delay time before the scale length begins to grow
		Argument<double> ScaleLengthDelay = Argument<double>(1.0,"scale-length-delay");
		
		//! The speed with which the scale length grows
		Argument<double> ScaleLengthTimeScale = Argument<double>(2.0,"scale-length-time");
		
		//! The time at which the scale length stops growing at becomes fixed
		Argument<double> ScaleLengthFinalTime = Argument<double>(12.0,"scale-length-final");

		//! The mass of the first (fast) exponential infall
		Argument<double> InfallMass1 = Argument<double>(50,"M1");
		
		//! The mass of the second (slow) exponential infall
		Argument<double> InfallMass2 = Argument<double>(100,"M2");
		
		//! The exponential timescale for the first (fast) exponential infall
		Argument<double> InfallTime1 = Argument<double>(0.4,"b1");
		
		//! The exponential timescale for the second (slow) exponential infall
		Argument<double> InfallTime2 = Argument<double>(6.0,"b2");

		Argument<double> InfallMassMerger = Argument<double>(0,"merger-mass");
		
		Argument<double> InfallTimeMerger = Argument<double>(0.4,"merger-timescale");
		
		Argument<double> MergerDelayTime = Argument<double>(8,"merger-delay");
		
		Argument<double> MergerTurnOnWidth = Argument<double>(0.3,"merger-width");
		
		//!maximum fraction which can be removed by SFR + associated feedback 
		Argument<double> MaxSFRFraction = Argument<double>(0.95,"max-sfr");

		//!Boring constructor -- slots in the relevant arguments into the ParamList::argPointer array
		GalaxyValues()
		{
			argPointers = {&RingCount, &PrimordialMass, &PrimordialHotFraction, &CGM_Mass, &CGM_Mass_End, &Radius, &MinScaleLength, &MaxScaleLength, &ScaleLengthDelay, &ScaleLengthTimeScale, &ScaleLengthFinalTime, &InfallMass1, &InfallMass2, &InfallTime1, &InfallTime2,  &MaxSFRFraction, &Ring0Width, &UsingVariableRingWidth, &CGMAbsorbing, &InfallMassMerger, & InfallTimeMerger, & MergerDelayTime,&MergerTurnOnWidth};
		}
		void Initialise(std::string resourceRoot);
};
