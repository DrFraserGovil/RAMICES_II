#pragma once
#include <string>
#include <vector>
#include "JSL.h"
#include <sstream>
#include "ParameterLists.h"
#include "List.h"
#include "EnumSets.h"

#define PI 3.14159265358979323846

//Options classes are sets of initialisations of Argument objetcs, such that they can be seprated & categorised



//! A package of global parameter objects which can be passed around by reference. Most of the internal values are set as JSL::Argument objects and so can be initialised from the command line / config file
class GlobalParameters
{

	public:
		//! Simulation values - timescales, number of threads etc
		MetaValues Meta;
		
		//!Output directory values -- directory information etc.
		OutputValues Output;
		
		//!Resource directory values
		ResourceValues Resources;
		
		//! Abundance data + resource location data
		ElementValues Element;
		
		//! Stellar limits (mass, metallicity)
		StellarValues Stellar;

		//!Yield stuff
		YieldValues Yield;

		//! Hot gas cooling/injection parameters
		ThermalValues Thermal;
		
		//!Migration stuff
		MigrationValues Migration;
		
		//! Galactic size/ evolution parameters
		GalaxyValues Galaxy;

		//! A heterogeneous pointer array, which allows for a nice loop over the members. Any new parameter pack needs to be inserted here so that the member values can be initialised.
		std::vector<ParamList *> ParamMembers = {&Meta,&Output,&Resources,&Element,&Stellar,&Thermal,&Galaxy,&Yield,&Migration};
		
		//! Does absolutely nothing!
		GlobalParameters();
		
		//! Loops over the ParamMembers and initialises their values according to the ParamList object.
		void Initialise(int argc, char* argv[]);
		
		//! Writes the inputs to file as a mock-config file so that this exact simulation can be rerun
		void SaveInputs();
		
};
