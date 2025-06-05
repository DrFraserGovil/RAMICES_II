#include "reservoir.h"


//Constructors & factories
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
		return Cold.Mass() + Hot.Mass();
	}

	double GasReservoir::GetTemperature() const
	{
		double tm = TotalMass();
		if (tm > 0)
		{
			return Hot.Mass() / tm;
		}
		else
		{
			return 0;
		}
	}
	void GasReservoir::Absorb(const GasReservoir & source)
	{
		Hot.Absorb(source.Hot);
		Cold.Absorb(source.Cold);
	}

	void GasReservoir::Absorb(const Gas & source, double temp)
	{
		TemperatureValidate(temp);
		Hot.Absorb(source,temp);
		Cold.Absorb(source,1.0-temp);
	}

	void GasReservoir::Deplete(double amount, Move type)
	{
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
		TemperatureValidate(sourceTemperature);
		auto [coldTransfer,hotTransfer] = TemperatureBalancer(amount,type,sourceTemperature);
		if (coldTransfer> 0)
		{
			Gas::Transfer(source,destination.Cold,coldTransfer,type);
		}
		if (hotTransfer > 0)
		{
			Gas::Transfer(source,destination.Hot,hotTransfer,type);
		}
	}
	void GasReservoir::Transfer(GasReservoir & source, GasReservoir & destination, double amount, Move type)
	{
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
		TemperatureValidate(temperature);
		double coldAmount;
		double hotAmount;
		switch (type)
		{
			case (Move::Mass) :
			{
				coldAmount = amount * (1.0 - temperature);
				hotAmount = amount * temperature;
				break;
			}
			case (Move::Fraction):
			{
				coldAmount = amount;
				hotAmount = amount;
				break;
			}
		}
		return std::pair<double,double>{coldAmount,hotAmount};
	}

	void inline GasReservoir::TemperatureValidate(double temperature)
	{
		if (temperature < 0 || temperature > 1)
		{
			LOG(ERROR) << "Gas temperature of " << temperature << " is meaningless; temperatures must be between 0 (cold) and 1 (hot).";
			throw std::logic_error("Unphysical quantity encountered");
		}
	}