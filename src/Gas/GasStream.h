#pragma once
#include "../Parameters/GlobalParameters.h"
#include "Gas.h"
/*!
 * A gas stream is how a homogeneously-sourced set of gas gets moved around . They are created through SourceEvents - such as CCSN or accretion events.
*/
class GasStream
{
	public:
		SourceProcess Source;
		
		Gas Hot;
		Gas Cold;
	
		GasStream();
		GasStream(SourceProcess source);
		GasStream(SourceProcess source,const Gas & hot, const Gas & cold);
		GasStream(SourceProcess source, const Gas & gas, double hotFraction);
		
		double Mass();
		double HotMass();
		double ColdMass();
		
		void Deplete(double amountToRemove);
		void Deplete(double amountToRemove_Cold, double amountToRemove_Hot);
		void Absorb(const GasStream & input);
		void StreamIn(const Gas & input, double hotFraction);
	private:
		bool NeedsRecomputing;
		double internal_HotMass;
		double internal_ColdMass;
		double internal_TotalMass;
		void ComputeMasses();
};

