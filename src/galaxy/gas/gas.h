#pragma once

#include "../../settings/SimulationSettings.h"
#include "../../utility/Log.h"
class Gas
{
	public:
		//No public constructor, instead have some factory functions so you can be sure with what you're getting

		//! The default entry point, creates an empty gas object  \return A gas object with no mass, and no elemental composition
		static Gas Empty();

		//! Create a gas object with a specified mass, and elemental composition   \param mass the final mass of the gas object \param composition A fractional array of length Element::Count which specifies the fraction of the total mass composed of each corresponding element. \returns A gas object with a specified mass and fractional composition
		static Gas WithComposition(double mass, const std::vector<double> & composition);

		static Gas WithSameComposition(double mass, const Gas & targetGas);

		//! Creates the object with a direct copy of the mas grid. The final mass is equal to the sum of massArray.
		static Gas WithMass(const std::vector<double> & massArray);
		// 
		//! \return A gas object of the specified mass but with a primordial elemental abundance distribution (X = 0.75, Y = 0.25 etc)
		static Gas Primordial(double mass);
	
		const std::vector<double> & Composition() const;


		static void Transfer(Gas & source, Gas & destination);
		static void TransferFraction(Gas & source, Gas & destination, double fraction);
		static void TransferMass(Gas & source, Gas & destination, double mass);


		void Absorb(Element::Species element, double amount);
		void Absorb(const Gas & input);
		void DepleteByFraction(double amount);
		void DepleteByMass(double amount);
	
		//! \return The current total mass within the #Species array
		double Mass() const;
		
		//! \return The current metallicity
		double Metallicity() const;
		
		double FractionOf(Element::Species id) const;
		double MassOf(Element::Species id) const;
		//! \return A reference to the indexed member of #Species, allowing for vector like access
		double & operator[](Element::Species id);
		double & operator[](int id);
		double  operator[](Element::Species id) const;
		double  operator[](int id) const;
		
	
		// double Mass() const;
		// //! An annoyingly necessary redeclaration for when the object is const and normal references don't behave nicely
		// const double & operator[](Element::Species id) const;
		
	private:
		//! Default Constructor initialises the chunk of gas to have zero mass
		Gas();
		
		static void internalTransfer(Gas & source, Gas & destination, double transferFraction);

		void internalDeplete(double depletionFraction);
		//! The central mass array. Has ::ElementCount elements, indexed by ElementID
		std::vector<double> internalMassOf;
		
		void ComputeMass() const;
		void ComputeComposition() const;
		void RegisterChanges();

		//caches declared mutable so that const Gas doesn't return nonsense values
		//mutable means that they are non-const even in a const. object
		//that's fine here because they're internal caches
		mutable bool massNeedsRecomputing;
		mutable bool compNeedsRecomputing;
		mutable std::vector<double> internalComposition;
		mutable double internal_Mass;
	
};