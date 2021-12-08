#include "GasReservoir.h"

GasReservoir::GasReservoir()
{
	Mass = 0.0;
	ColdMass = 0.0;
	HotMass= 0.0;
}
GasReservoir::GasReservoir(Options * opts)
{
	Mass = 0.0;
	ColdMass = 0.0;
	HotMass= 0.0;
	Opts = opts;
	int n = Opts->Element.NSpecies;
	ColdChemicals = std::vector<double>(n,0.0);
	HotChemicals = std::vector<double>(n,0.0);
}

GasReservoir::GasReservoir(Options * opts, std::vector<double> coldChemicals)
{
	Opts = opts;
	
	ColdMass = std::accumulate(std::begin(coldChemicals), std::end(coldChemicals), 0.0);
	Mass = ColdMass;
	HotMass = 0.0;
	
	int n = Opts->Element.NSpecies;
	ColdChemicals.resize(n);
	HotChemicals.resize(n);
	for (int i = 0; i < n; ++i)
	{
		ColdChemicals[i] = coldChemicals[i] / ColdMass;
		HotChemicals[i] = 0.0;
	}
}

GasReservoir::GasReservoir(Options * opts, std::vector<double> coldChemicals, std::vector<double> hotChemicals)
{
	Opts = opts;
	
	ColdMass = std::accumulate(std::begin(coldChemicals), std::end(coldChemicals), 0.0);
	HotMass = std::accumulate(std::begin(hotChemicals), std::end(hotChemicals), 0.0);
	Mass = ColdMass + HotMass;
	
	
	int n = Opts->Element.NSpecies;
	ColdChemicals.resize(n);
	HotChemicals.resize(n);
	for (int i = 0; i < n; ++i)
	{
		ColdChemicals[i] = coldChemicals[i] / ColdMass;
		HotChemicals[i] = hotChemicals[i] / HotMass;
	}
}


GasReservoir GasReservoir::operator+(const GasReservoir& b)
{
	GasReservoir c = GasReservoir(Opts);
	//c.Print();
	
	c.Mass = this->Mass + b.Mass;
	c.ColdMass = this->ColdMass + b.ColdMass;
	c.HotMass = this->HotMass + b.HotMass;
	

	int n = Opts->Element.NSpecies;

	log(1) << std::to_string(n) + "\n";
	for (int i = 0; i < n; ++i)
	{
		
		c.ColdChemicals[i] = (this->ColdChemicals[i] * this->ColdMass + b.ColdChemicals[i] * b.ColdMass)/c.ColdMass;
		c.HotChemicals[i] = (this->HotChemicals[i] * this->HotMass + b.HotChemicals[i] * b.HotMass) / c.HotMass;
	}
	return c;
}

void GasReservoir::Print()
{
	log(1) << "Mass = " + std::to_string(Mass) + " (Cold = " + std::to_string(ColdMass) + ", Hot = " + std::to_string(HotMass) + ")\n";
	
	int n = Opts->Element.NSpecies;
	for (int i = 0; i < n; ++i)
	{
		log(1) << Opts->Element.ElementNames[i] + ":\t\t" + std::to_string(ColdChemicals[i]) + "\t" + std::to_string(HotChemicals[i]) + "\n";
	}
}

void GasReservoir::SetPrimordial(double mass)
{
	Mass = mass;
	HotMass = mass * Opts->Galaxy.PrimordialHotFraction;
	ColdMass = mass - HotMass;
	
	double primordialHydrogen = 0.76;
	double primordialHelium = 0.24;
	
	ColdChemicals[Opts->Element.HydrogenID] = primordialHydrogen;
	ColdChemicals[Opts->Element.HeliumID] = primordialHelium;
	
	HotChemicals[Opts->Element.HydrogenID] = primordialHydrogen;
	HotChemicals[Opts->Element.HeliumID] = primordialHelium;
}

void GasReservoir::AddTo(GasReservoir * receivingGas)
{
	//AddTo adds the entire contents of the calling object into the recievingGas
	//It does NOT alter the calling object  - to simultaneously alter masses, use the ShiftGas operation
	
	double newColdMass = receivingGas->ColdMass + this->ColdMass;
	double newHotMass = receivingGas->HotMass + this->HotMass;
	
	int n = Opts->Element.NSpecies;
	for (int i = 0; i < n; ++i)
	{
		if (newColdMass > 0.0)
		{
			receivingGas->ColdChemicals[i] = (receivingGas->ColdChemicals[i] * receivingGas->ColdMass + this->ColdChemicals[i] * this->ColdMass) / newColdMass;
		}
		else
		{
			receivingGas->ColdChemicals[i] = 0.0;
		}
		if (newHotMass > 0.0)
		{
			receivingGas->HotChemicals[i] = (receivingGas->HotChemicals[i] * receivingGas->HotMass + this->HotChemicals[i] * this->HotMass) / newHotMass;
		}
		else
		{
			receivingGas->HotChemicals[i] = 0.0;
		}
	}
	receivingGas->Mass += this->Mass;
	receivingGas->ColdMass += this->ColdMass;
	receivingGas->HotMass += this->HotMass;
}

void GasReservoir::SubtractFrom(GasReservoir * losingGas)
{
	//SubtractFrom removes the entire contents of the calling object from the losingGas
	//It does not alter the calling object - to simultaneously alter masses, use the ShiftGas operation
	
	//make masses negative
	this->Mass = -this->Mass;
	this->ColdMass = -this->ColdMass;
	this->HotMass = -this->HotMass;
	
	
	// add negative mass gas to target
	AddTo(losingGas);
	
	//revert the signs back (just in case)
	this->Mass = -this->Mass;
	this->ColdMass = -this->ColdMass;
	this->HotMass = -this->HotMass;
}

void ShiftGas(GasReservoir * giver, GasReservoir * taker, double coldMass, double hotMass)
{
	//ShiftGas takes an amount of cold gas and hot mass and moves it from the giver to the taker,
	//both objects change mass in this operation
	
	//check more gas isn't being moved than is actually present
	coldMass= std::min(coldMass, giver->ColdMass);
	hotMass = std::min(hotMass, giver->HotMass);
	
	//differential is a gas reservoir with same abundance ratio as parent, but with altered mass
	GasReservoir differential = *giver;
	differential.ColdMass = coldMass;
	differential.HotMass = hotMass;
	differential.Mass = coldMass + hotMass;
	
	differential.AddTo(taker);
	differential.SubtractFrom(giver);
}

void GasReservoir::GiveTo(GasReservoir * receivingGas,double coldMass, double hotMass)
{
	ShiftGas(this,receivingGas,coldMass,hotMass);
}

void GasReservoir::TakeFrom(GasReservoir * givingGas, double coldMass, double hotMass)
{
	ShiftGas(givingGas,this,coldMass,hotMass);
}

void GasReservoir::GiveTo(GasReservoir * receivingGas,double mixedMass)
{
	double coldFrac = ColdMass / Mass;
	double coldMass = mixedMass * coldFrac;
	double hotMass = mixedMass * (1.0 - coldFrac);
	
	ShiftGas(this,receivingGas,coldMass,hotMass);
}

void GasReservoir::TakeFrom(GasReservoir * givingGas, double mixedMass)
{
	double coldFrac = ColdMass / Mass;
	double coldMass = mixedMass * coldFrac;
	double hotMass = mixedMass * (1.0 - coldFrac);
	ShiftGas(givingGas,this,coldMass,hotMass);
}
void GasReservoir::Deplete(double coldMass, double hotMass)
{
	//depleting a reservoir removes a set amount of gas, without altering the chemical balance
	Mass -= coldMass + hotMass;
	ColdMass -= coldMass;
	HotMass -= hotMass;
}
