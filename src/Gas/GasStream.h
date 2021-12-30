#pragma once
#include "../Parameters/GlobalParameters.h"
#include "Gas.h"
/*!
 * A gas stream is how a homogeneously-sourced set of gas gets moved around . They are created through SourceEvents - such as CCSN or accretion events.
*/
class GasStream
{
	public:
		//!A label identifying where and how this gas stream was created
		SourceProcess Source;
		
		const Gas & Hot() const;
		const Gas & Cold() const;
		
		double & Hot(ElementID el);
		
		double & Cold(ElementID el);
		
		const double & Hot(ElementID el) const;
		const double & Cold(ElementID el) const;
		
		//! Default constructor -- assigns itself as ::Unknown, with zero mass
		GasStream();
		
		//! Gives itself zero mass in both components \param source The value of #Source which this object inherits
		GasStream(SourceProcess source);
		
		//! Initialises itself to the correct hot/cold components \param source The value of #Source which this object inherits \param hot The gas component which is copied into #Hot \param cold The gas component which is copied into #Cold
		GasStream(SourceProcess source,const Gas & hot, const Gas & cold);
		
		//! Splits a single Gas object up into fractions determined by the hot fraction \param source The value of #Source which this object inherits \param gas The total gas mass of the new stream \param hotFraction the fraction of the gas which is put into #Hot, keeping the elemental abundances the same 
		GasStream(SourceProcess source, const Gas & gas, double hotFraction);
		
		//! This function attempts to be clever. Checks the #NedsRecomputing flag, and if necessary, calls ComputeMasses(), then sets the flag accordingly. Should reduce the number of loops needed. \returns The current total mass of the stream
		double Mass();
		
		double Mass() const;
		//! See Mass() for details \returns The current mass of the #Hot component
		double HotMass();
		
		//! See Mass() for details \returns The current mass of the #Cold component
		double ColdMass();
		
		//! As with HotMass(), but returns only the last computed value...assumes proper normalisation before casting to const!
		double HotMass() const;
		
		//! As with ColdMass(), but returns only the last computed value...assumes proper normalisation before casting to const!
		double ColdMass() const;
		
		//! Removes (i.e. throws away) the chosen amount of mass, keeping the hot/cold ratio and the elemental abundances the same \param amountToRemove The amount of mass to lose from the stream
		void Deplete(double amountToRemove);
		
		//!Moves the specified mass from the cold reservoir into the hot reservoir, keeping the cold elemental abundances constant
		void Heat(double amountToHeat);
		
		//!Moves the specified mass from the hot reservoir to the cold reservoir, keeping the hot elemental abundances constant
		void Cool(double amountToCool);
		//~ void DepleteFraction(double fraction)
		//~ {
				//~ Deplete(fraction* Mass());
		//~ }
		
		//! Removes (i.e. throws away) the mass from the hot and cold components, keeping the individual elemental abundances the same \param amountToRemove_Cold, the amount of cold gas to lose \param amountToRemove_Hot, the amount of hot gas to lose
		void Deplete(double amountToRemove_Cold, double amountToRemove_Hot);
		
		//! Adds the gas contained within the input to the current stream \param input The object which donates the gas (unaltered)
		void Absorb(const GasStream & input);
		
		//! Adds the specified fractiongas contained within the input to the current stream \param input The object which donates the gas (unaltered) \param fraction the fraction of the input object which is absorbed
		void Absorb(const GasStream & input,double fraction);
		
		//! Adds the gas to the current stream, splitting it according to the hotFraction \param input The object which donates the gas (unaltered) \param hotFraction The amount of the input which goes into the #Hot stream
		void Absorb(const Gas & input, double hotFraction);
		
		//! Sets the #NeedsRecomputing flag to true. Used to inform the Stream that someone (most probably a call through GasReservoir::operator[]) has gone over its head and touched its internal workings.
		void Dirty();
		
		
	private:
		//! A flag used to prevent excessive recomputing of the internal masses. By design, if Absorb(), Deplete() etc. calls are avoided, the components of #Hot and #Cold are invariant, so the values of Mass() are constant. Therefore if this flag is false, simply returns the last computed value of the mass value. 
		bool NeedsRecomputing;
		
		//! The last computed value of #Hot.#Gas::Mass()
		double internal_HotMass;
		
		//! The last computed value of #Cold.#Gas::Mass()
		double internal_ColdMass;
		
		//! The sum of #internal_HotMass and #internal_ColdMass
		double internal_TotalMass;
		
		//! If #NeedsRecomputing is true, this recalculates #internal_HotMass and #internal_ColdMass 
		void ComputeMasses();
		
		//!The container for the hot component
		Gas internal_Hot;
		
		//!The container for the cold component
		Gas internal_Cold;
};

