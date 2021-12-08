#pragma once
#include "JSL.h"
#include "List.h"
#include "ElementIDs.h"
using JSL::Argument;


//!The MetaValues contains variables associated with the base-level information about the sumulation - the number of cores to access, the timesteps etc. 
class MetaValues : public ParamList
{
	
	public:
		//!The name of the output directory into which the output will be saved
		Argument<std::string> SaveRoot =  Argument<std::string>("Output/","output");
		
		//!The location of the directory which the code looks for its expected resource file structure
		Argument<std::string> ResourceRoot  = Argument<std::string>("Resources/","resource");
		
		//!The maximum number of parallel threads which can be active at any given time
		Argument<int> ParallelThreads = Argument<int>(1,"thread");
		
		//!The top level timestep used in the main chemical loop
		Argument<double> TimeStep  = Argument<double>(0.1,"timestep");
		
		//!The total duration of the chemical simulation
		Argument<double> SimulationDuration = Argument<double>(14.0,"duration");
		
		MetaValues()
		{
			argPointers = {&SaveRoot,&ResourceRoot,&ParallelThreads,&TimeStep,&SimulationDuration};
		};
};



class ElementValues : public ParamList
{
	//the elemental suboptions contains variables associated with the elemental IDs and the number of elements being tracked in the simulation
	
	public:
		
		//! Human readable names for the elements, in the order associated with the ElementIDs. These names are primarily elemental symbols, except Metals, which uses "Z"
		std::vector<std::string> ElementNames;

		//! Solar abundances (in mass units) of the elements, in the order associated with the ElementIDs
		std::vector<double> SolarAbundances;
		
		//! The file in which the solar abundances can be found as a csv
		Argument<std::string> SolarAbundanceFile = Argument<std::string>("ChemicalData/SolarAbundances.dat","-solar-values-file"); 

		//! The column of the solar abundance files which contains the ElementName for cross matching
		Argument<int> SolarAbundanceFileNameColumn = Argument<int>(0,"-solar-values-name-col");
		
		//! The column of the solar abundance file which contains the relevant solar abundance value to be saved to memory
		Argument<int> SolarAbundanceFileDataColumn = Argument<int>(3,"-solar-values-data-col");
		
		//! Enums to identify different elemental yield sources
		enum SourceIDs {Orfeo,Marigo,Limongi,Maeder,Mixed};
		
		ElementValues()
		{
				argPointers = {&SolarAbundanceFile, &SolarAbundanceFileDataColumn, &SolarAbundanceFileNameColumn};
		};
		virtual void Initialise(std::string resourceRoot);
		void GiveElementsNames();
		
};

class StellarValues : public ParamList
{
	//the stellar suboptions contains variables associated with stars + their remnants
	
	public:
		Argument<double> MaxStellarMass = Argument<double>(100,"-max-mass");
		Argument<double> MinStellarMass = Argument<double>(0.5,"-min-mass");
		Argument<double> MinZ = Argument<double>(1e-7,"-min-z");
		Argument<double> MaxZ = Argument<double>(0.052,"-max-z");

		Argument<double> EjectionFraction = Argument<double>(0.45,"eject");
		Argument<double> SNIaFraction = Argument<double>(0.05,"-sn1a-frac");
		Argument<double> NSMFraction = Argument<double>(0.0001,"-nsm-frac");
		Argument<double> FeedbackFactor = Argument<double>(0.4,"-mass-load");
	
		StellarValues()
		{
			argPointers = {&MaxStellarMass, &MinStellarMass, &MinZ, &MaxZ, &EjectionFraction, &SNIaFraction, &NSMFraction};
		}
};


class ThermalValues : public ParamList
{
	//thermal suboptions contain variables which deal with the thermal subroutines - cooling timescales injection fractions etc. 
	
	public:
		Argument<double> HotInjection_CCSN = Argument<double>(0.7,"-fh-ccsn");
		Argument<double> HotInjection_NSM = Argument<double>(0.4,"-fh-nsm");
		Argument<double> HotInjection_SNIa = Argument<double>(0.99,"-fh-sn1a");
		
		Argument<double> GasCoolingTimeScale = Argument<double>(1.0,"cool");
	
	
		ThermalValues()
		{
			argPointers = {&HotInjection_CCSN, &HotInjection_NSM, &HotInjection_SNIa, &GasCoolingTimeScale};
		}
};

class GalaxyValues : public ParamList
{
	//the galaxy suboptions contians variables associated with the galaxy as a whole, such as the maximum radius, and various mass/infall properties
	
	public:
		Argument<int> NRings = Argument<int>(100,"rings");
		
		Argument<double> PrimordialMass = Argument<double>(1.8,"M0");
		Argument<double> PrimordialHotFraction = Argument<double>(0,"-primordial-hot");
		Argument<double> MaxRadius = Argument<double>(20.0,"radius");
		
		Argument<double> MinScaleLength = Argument<double>(1.75,"-scale-length-min");
		Argument<double> MaxScaleLength = Argument<double>(3.75,"-scale-length-max");
		Argument<double> ScaleLengthDelay = Argument<double>(1.0,"-scale-length-delay");
		Argument<double> ScaleLengthTimeScale = Argument<double>(2.0,"-scale-length-time");
		Argument<double> ScaleLengthFinalTime = Argument<double>(12.0,"-scale-length-final");

		Argument<double> InfallMass1 = Argument<double>(4.5,"M1");
		Argument<double> InfallMass2 = Argument<double>(45,"M2");
		Argument<double> InfallTime1 = Argument<double>(0.3,"b1");
		Argument<double> InfallTime2 = Argument<double>(14.0,"b2");

		Argument<double> InflowParameterA = Argument<double>(0.33,"-inflow-a");
		Argument<double> InflowParameterB = Argument<double>(0.53,"-inflow-b");
		
		//!maximum fraction which can be removed by SFR + associated feedback 
		Argument<double> MaxSFRFraction = Argument<double>(0.98,"-max-sfr");

		
		GalaxyValues()
		{
			argPointers = {&NRings, &PrimordialMass, &PrimordialHotFraction, &MaxRadius, &MinScaleLength, &MaxScaleLength, &ScaleLengthDelay, &ScaleLengthTimeScale, &ScaleLengthFinalTime, &InfallMass1, &InfallMass2, &InfallTime1, &InfallTime2, &InflowParameterA, &InflowParameterB, &MaxSFRFraction};
		}
	
};
