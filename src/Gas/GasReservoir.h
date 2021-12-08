#pragma once
#include <vector>
#include "../Parameters/GlobalParameters.h"
#include "GasStream.h"
/*!
 *  A GasReservoir is a heterogenously sourced pool of gas, such as those found within each ring, or representing the IGM
 * In practicality, they are a container for a vector of GasStream objects + assorted ways for these objects to interact with one another
 * 
*/

class GasReservoir
{
	public:
		GasReservoir();
		
		double Mass();
		double ColdMass();
		double HotMass();
		
		//gas movement options
		void Absorb(const GasReservoir & givingGas);
		void Absorb(const GasStream & givingGas);		
		void TakeFrom(GasReservoir & givingGas, double amountToTake);
		void Deplete(double amountToLose);
		void Deplete(double amountToLose_Cold, double amountToLose_Hot);
		
		static GasReservoir Primordial(double mass, GlobalParameters & param);
		const GasStream & Component(SourceProcess source) const;
	private:
		std::vector<GasStream> Components;
	
	
		
};


