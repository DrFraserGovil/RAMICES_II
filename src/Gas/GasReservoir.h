#pragma once
#include <vector>
#include "../Parameters/GlobalParameters.h"
#include "GasStream.h"
#include <sstream>
/*!
 *  A GasReservoir is a heterogenously sourced pool of gas, such as those found within each ring, or representing the CGM.
 * In practicality, they are a container for a vector of GasStream objects + assorted ways for these objects to interact with one another
 * 
*/

class GasReservoir
{
	public:
		
		//! Default constructor. Initialises the #Components and gives them their #SourceProcess ID.
		GasReservoir();
		
		//! useful constructor
		GasReservoir(const GlobalParameters & param);
		
		//! A vector-like access overload, allowing indexing into the #Components vector using appropriate #SourceProcesses
		GasStream & operator[](SourceProcess source);
		
		//! An annoyingly necessary redeclaration for when constant references don't want to play ball
		const GasStream & operator[](SourceProcess source) const;
		
		
		//!\returns The current total mass of the reservoir, the sum of GasStream::Mass() calls over the #Components vector.
		double Mass();
		
		//!\returns The current total cold-gas mass of the reservoir, the sum of GasStream::ColdMass() calls over the #Components vector.
		double ColdMass();
		
		//!\returns The current total hot-gas mass of the reservoir, the sum of GasStream::HotMass() calls over the #Components vector.
		double HotMass();
		
		//! Transfer the contents of the input reservoir and sum them into the reservoir \param givingGas the reservoir which will be summed into the current object (unaltered)
		void Absorb(const GasReservoir & givingGas);
		
		
		
		//! Transfer the contents of the input stream into the element of #Components indicated by the input's #GasStream::Source flag. \param givingGas the stream which is absorbed into the reservoir (unaltered)
		void Absorb(const GasStream & givingGas);		

		void Absorb(const GasStream & givingGas, double fraction);		

		void Absorb(const std::vector<GasStream> & givingGas);

		void Absorb(const std::vector<GasStream> & givingGas, double fraction);

		void AbsorbMemory(int t, const GasStream & input);
		
		//! Calls #GasStream::Deplete(double) on each element of #Components, keeping the relative mass contribution of each component equal \param amountToLose The total amount of mass to be lost from the reservoir (shared amongst components)
		void Deplete(double amountToLose);
		
		
		//!Wipes all mass from the reservoir
		void Wipe();
		
		//! Calls #GasStream::Deplete(double, double) on each element of #Components, keeping the relative hot mass and cold mass contribution of each component equal \param amountToLose_Cold The total amount of cold gas mass to be lost from the reservoir (shared amongst components) \param amountToLose_Hot The total amount of hot gas mass to be lost from the reservoir (shared amongst components)
		void Deplete(double amountToLose_Cold, double amountToLose_Hot);
		
		//! Heats up the specified amount of gas into the hot reservoir, keeping the elemental abundances of the cold gas reservoir constant
		void Heat(double amoutToHeat);
		
		//!Executes the usual cooling mechanism
		void PassiveCool(double dt, bool isCGM);
		
		//! Transfers the specified amount of mass across from the target, removing the mass from the target and adding it to the current object. Maintains the thermal, source and elemental ratios of the source object
		void TransferFrom(GasReservoir & givingGas, double massToMove);
		
		//! Transfers the specified amount of mass across from the target, removing the mass from the target and adding it to the current object. Maintains the source and elemental ratios of the source object
		void TransferColdFrom(GasReservoir & givingGas, double massToMove);
		
		void TransferHotFrom(GasReservoir & givingGas, double massToMove);
		
		void TransferAndHeat(GasReservoir & givingGas, double massToMove);
		//! Extracts the chosen amount of cold gas from the reservoir, and puts it into an accretion stream
		GasStream AccretionStream(double amountToLose);
		
		//! Generates a primordial gas reservoir of the specified mass -- only the ::Primordial component is populated, with the nature of that component determined by several key parameters in GlobalParameters \param mass The total mass of the new reservoir \param param A reference to the global parameter set - required for primordial abundances and hot-gas fractions 
		static GasReservoir Primordial(double mass, const GlobalParameters & param);

		double ColdGasMetallicity() const;
		
	
		const std::vector<GasStream> & Composition() const;
	private:
	
		//! A representation of the total amount of gas within the reservoir, separated by the origin of the gas
		std::vector<GasStream> Components;
		

		const GlobalParameters & Param;
		
		
};


