#include "gas.h"

using namespace Element;
Gas::Gas() : internalMassOf(Element::Count,0.0), internalComposition(Element::Count,0.0)
{
	internal_Mass = 0;
}


//Property Queries
	double Gas::Mass() const
	{
		if (massNeedsRecomputing)
		{
			ComputeMass();
		}
		return internal_Mass;
	}

	double Gas::Metallicity() const
	{
		double nonMetal = FractionOf(Hydrogen) + FractionOf(Helium);
		return 1.0 - nonMetal;
	}


	double Gas::MassOf(Element::Species id) const
	{
		return internalMassOf[id];
	}
	double Gas::FractionOf(Element::Species id) const
	{
		return Composition()[id];
	}

	const std::vector<double> & Gas::Composition() const
	{
		if (compNeedsRecomputing)
		{
			ComputeComposition();
		}
		return internalComposition;
	}

//Access and modifications 
	double & Gas::operator[](Element::Species id)
	{
		RegisterChanges();//returning a reference means the object may have been modified
		return internalMassOf[id];
	}

	double & Gas::operator[](int id)
	{
		RegisterChanges();//returning a reference means the object may have been modified
		return internalMassOf[id];
	}
	double Gas::operator[](Element::Species id) const
	{
		return internalMassOf[id];
	}

	double Gas::operator[](int id) const
	{
		return internalMassOf[id];
	}

	void Gas::Absorb(const Gas & input)
	{
		for (int i = 0; i < Element::Count; ++i)
		{
			internalMassOf[i] += input.internalMassOf[i];
		}
		RegisterChanges();
	}
	void Gas::Absorb(Element::Species element, double amount)
	{
		internalMassOf[element] += amount;
		RegisterChanges();
	}
	void Gas::DepleteByFraction(double frac)
	{
		if (frac < 0 || frac > 1)
		{
			LOG(ERROR) << "Cannot deplete gas by a fraction '" << frac << "', values must be in range [0,1]";
			throw std::logic_error("Unphysical quantity encountered");
		}
		
		internalDeplete(frac);
	}
	void Gas::DepleteByMass(double massToDeplete)
	{
		double m = Mass();
		//do some error checking
		if (m == 0 && massToDeplete == 0) //about to divide by m, so m==0 needs special handling, even if it is physical
		{
			return;
		}
		if (massToDeplete < 0 || massToDeplete > m)
		{
			LOG(ERROR) << "Cannot move a mass " << massToDeplete << ". Value must be in range [0," << m << "]";
			throw std::logic_error("Unphysical quantity encountered");
		}
		
		double depletionFraction = massToDeplete/m;
		internalDeplete(depletionFraction);
	}


	void Gas::Transfer(Gas & source, Gas & destination)
	{
		internalTransfer(source,destination,1.0);
	}

	void Gas::TransferFraction(Gas & source, Gas & destination, double fraction)
	{
		if (fraction < 0 || fraction > 1)
		{
			LOG(ERROR) << "Cannot move a fraction '" << fraction << "' of gas, values must be in range [0,1]";
			throw std::logic_error("Unphysical quantity encountered");
		}
		if (source.Mass() == 0 && fraction > 0)
		{
			LOG(WARN) << "Attempted to move a non-zero fraction of gas from a zero-mass object. Nothing happened."; 
			return;
		}
		internalTransfer(source,destination,fraction);
	}

	void Gas::TransferMass(Gas & source, Gas & destination,double massToTransfer)
	{
		double m = source.Mass();
		//do some error checking
		if (massToTransfer < 0 || massToTransfer > m)
		{
			LOG(ERROR) << "Cannot transfer a mass " << massToTransfer << ". Value must be in range [0," << m << "]";
			throw std::logic_error("Unphysical quantity encountered");
		}
		if (m == 0 && massToTransfer == 0) //about to divide by m, so m==0 needs special handling, even if it is physical
		{
			return;
		}
			
		double depletionFraction = massToTransfer/m;
		internalTransfer(source,destination,depletionFraction);
	}

	void Gas::internalTransfer(Gas & source, Gas & destination, double transferFraction)
	{
		//assumes error checking already handled properly
		double sourceRetain = 1.0 - transferFraction;
		for (int i = 0; i < Element::Count; ++i)
		{
			destination.internalMassOf[i] += transferFraction * source.internalMassOf[i];
			source.internalMassOf[i] *= sourceRetain;
		}
		source.RegisterChanges();
		destination.RegisterChanges();
	}

	void Gas::internalDeplete(double depletionFraction)
	{
		double remainingFraction = 1.0 - depletetionFraction;
		for (int i = 0; i < Element::Count; ++i)
		{
			internalMassOf[i] *= remainingFraction;
		}
		RegisterChanges();
	}
//Internal functions
	void Gas::RegisterChanges()
	{
		massNeedsRecomputing = true;
		compNeedsRecomputing = true;
	}
	void Gas::ComputeMass() const
	{
		internal_Mass = 0;
		for (int i =0; i < Element::Count; ++i)
		{
			internal_Mass += internalMassOf[i];
		}
		massNeedsRecomputing = false;
	}
	void Gas::ComputeComposition() const
	{
		double m = Mass();
		for (int i =0; i < Element::Count; ++i)
		{
			if (m == 0)
			{
				internalComposition[i] = 0;
			}
			else
			{
				internalComposition[i] = internalMassOf[i]/m;
			}
		}
		compNeedsRecomputing = false;
	}
	

//Factories
	Gas Gas::WithComposition(double mass,const std::vector<double> & composition)
	{
		if (mass < 0)
		{
			LOG(ERROR) << "Cannot create a gas object with negative mass (" << mass << " < 0)";
			throw std::logic_error("Unphysical quantity encountered");
		}
		
		if (composition.size() != Element::Count)
		{
			LOG(ERROR) << "Partial composition vectors are invalid. Cannot construct a gas object without a correctly sized composition";
			throw std::runtime_error("Incorrectly sized composition vector (" + std::to_string(composition.size()) + ")");
		}
		auto g = Gas();
		for (int el = 0; el < Element::Count; ++el)
		{
			g[el] = composition[el] * mass;
			g.internalComposition[el] = composition[el];
		}
		g.internal_Mass = mass;
		g.massNeedsRecomputing = false;
		g.compNeedsRecomputing = false;

		if (mass == 0)
		{
			LOG(WARN) << "Constructed a gas with specified composition, but zero mass. This is usually unintended";
			g.compNeedsRecomputing = true; // flag here because the default copy doesn't work on 0 mass-gases
		}
		return g;
	}

	Gas Gas::WithMass(const std::vector<double> & massArray)
	{
		auto g = Gas();
		for (int el = 0; el < Element::Count; ++el)
		{
			g[el] = massArray[el];
		}
		g.massNeedsRecomputing = true;
		return g;
	}

	Gas Gas::Primordial(double mass)
	{
		return Gas::WithComposition(mass,Settings.Abundance.PrimordialAbundances);
	}

	Gas Gas::WithSameComposition(double mass, const Gas & targetGas)
	{
		if (targetGas.Mass() == 0)
		{
			LOG(ERROR) << "Gas to be copied has zero mass, and hence no composition to copy";
			throw std::logic_error("Unphysical quantity encountered");
		}
		return Gas::WithComposition(mass,targetGas.Composition());
	}

	Gas Gas::Empty()
	{
		auto g =  Gas();
		return g;
	}
