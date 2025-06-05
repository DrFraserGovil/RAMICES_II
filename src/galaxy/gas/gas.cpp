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

	void Gas::Absorb(const Gas & input,double fraction)
	{
		for (int i = 0; i < Element::Count; ++i)
		{
			internalMassOf[i] += input.internalMassOf[i]*fraction;
		}
		RegisterChanges();
	}
	void Gas::Absorb(Element::Species element, double amount)
	{
		internalMassOf[element] += amount;
		RegisterChanges();
	}


	void Gas::Deplete(double amount, Move type)
	{
		
		double fraction = validateMovement(amount,type,"Gas::Deplete");

		if (fraction > 0)
		{
			double remainingFraction = 1.0 - fraction;
			for (int i = 0; i < Element::Count; ++i)
			{
				internalMassOf[i] *= remainingFraction;
			}
			RegisterChanges();
		}
	}

	void Gas::Transfer(Gas & source, Gas & destination, double amount,Move type)
	{
		double transferFraction = source.validateMovement(amount,type,"Gas::Transfer");
		if (transferFraction > 0)
		{
			double sourceRetain = 1.0 - transferFraction;
			for (int i = 0; i < Element::Count; ++i)
			{
				destination.internalMassOf[i] += transferFraction * source.internalMassOf[i];
				source.internalMassOf[i] *= sourceRetain;
			}
			source.RegisterChanges();
			destination.RegisterChanges();
		}
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
	
	double Gas::validateMovement(double amount, Move type,const std::string & callingFunction)
	{
		switch (type)
		{
			case (Move::Fraction):
			{
				
				if (amount < 0 || amount > 1)
				{
					LOG(ERROR) << "Cannot call " << callingFunction << " with a fraction '" << amount << "', values must be in range [0,1]";
					throw std::logic_error("Unphysical quantity encountered");
				}
				if (Mass() == 0 && amount > 0)
				{
					LOG(WARN) << "Attempted to call " << callingFunction << " on a non-zero fraction of gas from a zero-mass object. Nothing happened."; 
					return 0;
				}
				return amount;
				break;
			}
			case (Move::Mass):
			{
				double m = Mass();
				//do some error checking
				if (m == 0 && amount == 0) //about to divide by m, so m==0 needs special handling, even if it is physical
				{
					return 0;
				}
				if (amount < 0 || amount > m)
				{
					LOG(ERROR) << "Cannot call " << callingFunction << " on a mass " << amount << ". Value must be in range [0," << m << "]";
					throw std::logic_error("Unphysical quantity encountered");
				}
				
				return amount/m;
				break;//technically unneccessary but always good practice to remember a break to prevent fallthrough!
			}
		}
	}


//Factories
	Gas Gas::WithComposition(double mass,const std::vector<double> & composition,bool forceAcceptZeroMass)
	{
		if (mass < 0)
		{
			LOG(ERROR) << "Cannot create a gas object with negative mass (" << mass << " < 0)";
			throw std::logic_error("Unphysical quantity encountered");
		}
		if (mass == 0)
		{
			if (!forceAcceptZeroMass)
			{
				LOG(WARN) << "Constructed a gas with specified composition, but zero mass. This is usually unintended";
			}
			return Gas::Empty();
		}
		if (composition.size() != Element::Count)
		{
			LOG(ERROR) << "Partial composition vectors are invalid. Cannot construct a gas object without a correctly sized composition";
			throw std::runtime_error("Incorrectly sized composition vector (" + std::to_string(composition.size()) + ")");
		}
		auto g = Gas();
		for (int el = 0; el < Element::Count; ++el)
		{
			g.internalMassOf[el] = composition[el] * mass;
			g.internalComposition[el] = composition[el];
		}
		g.internal_Mass = mass;
		g.massNeedsRecomputing = false;
		g.compNeedsRecomputing = false;

		
		return g;
	}

	Gas Gas::WithMass(const std::vector<double> & massArray)
	{
		auto g = Gas();

		for (int el = 0; el < Element::Count; ++el)
		{
			g.internalMassOf[el] = massArray[el];
		}
		g.RegisterChanges();
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
		return Gas::WithComposition(mass,targetGas.Composition(),true);
	}

	Gas Gas::FractionalCopy(double fraction, const Gas & targetGas)
	{
		return WithSameComposition(fraction*targetGas.Mass(),targetGas);
	}

	Gas Gas::Empty()
	{
		auto g =  Gas();
		return g;
	}
