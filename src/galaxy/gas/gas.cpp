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

	void Gas::DepleteByFraction(double frac)
	{
		if (frac < 0 || frac > 1)
		{
			LOG(ERROR) << "Cannot deplete gas by a fraction '" << frac << "', values must be in range [0,1]";
			throw std::logic_error("Unphysical quantity encountered");
		}
		double remainingFraction = 1.0 - frac;
		for (int i = 0; i < Element::Count; ++i)
		{
			internalMassOf[i] *= remainingFraction;
		}
		RegisterChanges();
	}
	void Gas::DepleteByMass(double mass)
	{
		double m = Mass();
		if (m == 0)
		{
			if (mass == 0)
			{
				return;
			}
			LOG(ERROR) << "Cannot deplete a non-zero amount of gas from an empty gas reservoir";
			throw std::logic_error("Unphysical quantity encountered");
		}
		double depletionFraction = mass/Mass();
		DepleteByFraction(depletionFraction);
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
		}
		g.massNeedsRecomputing = true;
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
		return Gas::WithComposition(mass,targetGas.Composition());
	}

	Gas Gas::Empty()
	{
		auto g =  Gas();
		return g;
	}
