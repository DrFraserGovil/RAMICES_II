#pragma once
#include "../Parameters/GlobalParameters.h"

/*!
 * The most basic gas object there is -- an array of element-masses and some rules for interacting with it. 
*/
class Gas
{
	public:
		//! The central mass array. Has
		std::vector<double> Species;
		Gas();
		Gas(const std::vector<double> & elements);
		
		double Mass();
	
		double & operator[](ElementID id);
		const double & operator[](ElementID id) const;
		
		static Gas Empty();
		static Gas Primordial(double mass);
};

