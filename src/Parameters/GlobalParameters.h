#pragma once
#include <string>
#include <vector>
#include "JSL.h"
#include "ParameterLists.h"
#include "List.h"
#include "EnumSets.h"
#define PI 3.14159265358979323846

//Options classes are sets of initialisations of Argument objetcs, such that they can be seprated & categorised




class GlobalParameters
{

	public:
		
		MetaValues Meta;
		ElementValues Element;
		StellarValues Stellar;
		ThermalValues Thermal;
		GalaxyValues Galaxy;

		std::vector<ParamList *> ParamMembers = {&Meta,&Element,&Stellar,&Thermal,&Galaxy};
		
		GlobalParameters();
		void Initialise(int argc, char* argv[]);
};
