#pragma once

#include "../../settings/SimulationSettings.h"
class Gas
{
	public:

		//! Default Constructor initialises the chunk of gas to have zero mass
		Gas();
		
	
		//! \return The current total mass within the #Species array
		double Mass();
		
		//! \return The current metallicity
		double Metallicity();
		
		double FractionOf(Element::Species id);
		double MassOf(Element::Species id);
		//! \return A reference to the indexed member of #Species, allowing for vector like access
		double & operator[](Element::Species id);
		double & operator[](int id);
		
		//! \return A gas object of the specified mass but with a primordial elemental abundance distribution (X = 0.75, Y = 0.25 etc)
		static Gas Primordial(double mass);
		
		//! \return A default-constructed object, but name is clear that the object is empty
		static Gas Empty();
		
		// double Mass() const;
		// //! An annoyingly necessary redeclaration for when the object is const and normal references don't behave nicely
		// const double & operator[](Element::Species id) const;
		
	private:
		//! The central mass array. Has ::ElementCount elements, indexed by ElementID
		std::vector<double> internalMassOf;
	
		void ComputeMass();
		
		bool NeedsRecomputing;
		double internal_Mass;
	
};