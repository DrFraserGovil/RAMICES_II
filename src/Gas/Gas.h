#pragma once
#include "../Parameters/GlobalParameters.h"

class Gas
{
	public:

		std::vector<double> Species;
		Gas();
		Gas(const std::vector<double> & elements);
		
		double Mass();
	
	
		static Gas Empty();
		static Gas Primordial(double mass);
};

