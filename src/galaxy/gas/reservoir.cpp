#include "reservoir.h"


//Constructors & factories

	/*
		basic constructors
		Note that they *do not perform temperature validation* - this is left to the factory functions
	*/
	GasReservoir::GasReservoir() : Hot(Gas::Empty()), Cold(Gas::Empty()){}

	GasReservoir::GasReservoir(const Gas & cold, const Gas & hot) : Hot(hot), Cold(cold){}

	GasReservoir::GasReservoir(const Gas & source, double temp) : Hot(Gas::FractionalCopy(temp,source)), Cold(Gas::FractionalCopy(1.0 - temp,source)){
	}

	GasReservoir GasReservoir::Empty()
	{
		return GasReservoir();
	}
	GasReservoir GasReservoir::Primordial(double mass, double temperature)
	{
		TemperatureValidate(temperature);
		return GasReservoir(Gas::Primordial(mass),temperature);
	}

	GasReservoir GasReservoir::FromGas(const Gas & sourceGas, double temperature)
	{
		TemperatureValidate(temperature);
		return GasReservoir(sourceGas,temperature);
	}
	GasReservoir GasReservoir::FromGas(const Gas & cold, const Gas & hot)
	{
		return GasReservoir(cold,hot);
	}


//interface
	
	double GasReservoir::TotalMass() const
	{
		return Cold.Mass() + Hot.Mass(); //don't bother with caching here -- the individual reservoirs cache, and addition is basically free
	}

	double GasReservoir::GetTemperature() const
	{
		double tm = TotalMass();
		if (tm > 0)
		{
			return Hot.Mass() / tm; //temperature = fraction of gas that is hot, so is just the mass ratio.
		}
		else
		{
			return 0; //we define an empty reservoir as zero mass for....convenience more than anything. Prevents explosions.
		}
	}
	void GasReservoir::Absorb(const GasReservoir & source)
	{
		Hot.Absorb(source.Hot);
		Cold.Absorb(source.Cold);
	}

	void GasReservoir::Absorb(const Gas & source, double temp)
	{
		TemperatureValidate(temp); //external temp interface, so validate it! 
		
		//Treat the temp as a fraction of gas (which it is), passed to a fractional absorb call
		Hot.Absorb(source,temp);
		Cold.Absorb(source,1.0-temp);
	}

	void GasReservoir::Deplete(double amount, Move type)
	{
		//have a nice function that determines how to interpret amount/type so as to keep the temperature constant.
		auto [coldDeplete,hotDeplete] = TemperatureBalancer(amount,type,GetTemperature());
		
		if (coldDeplete > 0)
		{
			Cold.Deplete(coldDeplete,type);
		}
		if (hotDeplete > 0)
		{
			Hot.Deplete(hotDeplete,type);
		}

	}

	void GasReservoir::Deplete(Temperature temperature,double amount, Move type)
	{
		//just a boring overload for Cold.Deplete or Hot.Deplete
		switch (temperature)
		{
			case Temperature::Cold:
				Cold.Deplete(amount,type);
				break;
			case Temperature::Hot:
				Hot.Deplete(amount,type);
				break;
		}
	}

//Transfers

	void GasReservoir::Transfer(Gas & source, GasReservoir & destination, double sourceTemperature, double amount, Move type)
	{
		//functionally identical to calling Transfer(FromGas(source,sourceTemperature),destination,...etc)
		//however: this does it `in place' to avoid invocation overhead
		
		TemperatureValidate(sourceTemperature);
		

		//sequential fractional transfers are a *pain* due to intermediary mass changes. Force into mass space right here
		if (type == Move::Fraction)
		{
			amount *= source.Mass();
		}
		auto [coldTransfer,hotTransfer] = TemperatureBalancer(amount,Move::Mass,sourceTemperature);

		if (coldTransfer > 0)
		{
			Gas::Transfer(source,destination.Cold,coldTransfer,Move::Mass);
		}
		if (hotTransfer > 0)
		{
			Gas::Transfer(source,destination.Hot,hotTransfer,Move::Mass);
		}

	}
	void GasReservoir::Transfer(GasReservoir & source, GasReservoir & destination, double amount, Move type)
	{
		//the same balancing code used for depletion (which is how the source experiences it)
		//peg the balancer to the *source*, not the destination, as it's the source's temp which remains unchanged
		auto [coldTransfer,hotTransfer] = TemperatureBalancer(amount,type,source.GetTemperature());
		
		if (coldTransfer> 0)
		{
			Gas::Transfer(source.Cold,destination.Cold,coldTransfer,type);
		}
		if (hotTransfer > 0)
		{
			Gas::Transfer(source.Hot,destination.Hot,hotTransfer,type);
		}
	}


//Internal functions
	std::pair<double, double> GasReservoir::TemperatureBalancer(double amount, Move type,double temperature)
	{
		//split 'amount' into hot and cold versions in such a way that the source's temperature is unchanged.
		double coldAmount;
		double hotAmount;
		switch (type)
		{
			case (Move::Mass) :
			{
				//in the case of mass move, need to move a temp-weighted mass from each reservoir
				coldAmount = amount * (1.0 - temperature);
				hotAmount = amount * temperature;
				break;
			}
			case (Move::Fraction):
			{
				//trivial in the case of fractional move -- deplete from both equally!
				coldAmount = amount;
				hotAmount = amount;
				break;
			}
		}
		return std::pair<double,double>{coldAmount,hotAmount};
	}

	void inline GasReservoir::TemperatureValidate(double temperature)
	{
		//throw a nice error if an invalid temp is used
		if (temperature < 0 || temperature > 1)
		{
			LOG(ERROR) << "Gas temperature of " << temperature << " is meaningless; temperatures must be between 0 (cold) and 1 (hot).";
			throw std::logic_error("Unphysical quantity encountered");
		}
	}