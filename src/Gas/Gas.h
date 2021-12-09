#pragma once
#include "../Parameters/GlobalParameters.h"

/*!
 * The most basic gas object there is -- an array of element-masses and some rules for interacting with it. 
*/
class Gas
{
	public:
	
		
		//! Default Constructor initialises the chunk of gas to have zero mass
		Gas();
		
		//! Turns the elements array into the #Species entity, otherwise does nothing interesting
		Gas(const std::vector<double> & elements);
		
		//! \return The current total mass within the #Species array
		double Mass();
	
		//! \return A reference to the indexed member of #Species, allowing for vector like access
		double & operator[](ElementID id);
		
		//! An annoyingly necessary redeclaration for when the object is const and normal references don't behave nicely
		const double & operator[](ElementID id) const;
		
	private:
		//! The central mass array. Has ::ElementCount elements, indexed by ElementID
		std::vector<double> Species;
	
		//! \return A default-constructed object, but name is clear that the object is empty
		static Gas Empty();
		
		//! \return A gas object of the specified mass but with a primordial elemental abundance distribution (X = 0.75, Y = 0.25 etc)
		static Gas Primordial(double mass);
};

