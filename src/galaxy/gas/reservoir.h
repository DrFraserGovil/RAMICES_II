#pragma once
#include "gas.h"

enum class Temperature
{
	Cold=0, //temperature `scale' is a fractional mapping between cold and hot. Temperature::Cold is essentially an alias for temperature = 0..
	Hot=1
};

class GasReservoir
{
	public:

		Gas Hot;
		Gas Cold;

		double TotalMass() const;

		void Absorb(const GasReservoir & sourceGas);
		
		void Absorb(const Gas & sourceGas, double sourceTemp);

		void Deplete(double amount=1, Move type = Move::Fraction);
		void Deplete(Temperature temperature,double amount=1, Move type = Move::Fraction); //an alias for Hot.Deplete / Cold.Deplete
		
		double GetTemperature() const;
		

		static void Transfer(Gas & source, GasReservoir & destination, double sourceTemperature, double amount = 1,Move type=Move::Fraction);
		static void Transfer(GasReservoir & source, GasReservoir & destination, double amount = 1,Move type=Move::Fraction);

		GasReservoir(); //equiavelnt to calling Empty
		GasReservoir(const Gas & source, double temp);
		GasReservoir(const Gas & cold, const Gas & hot); //equivalent to calling FromGas with two gas arguments
		static GasReservoir Empty();
		static GasReservoir Primordial(double mass,double temperature = Settings.Thermal.PrimordialTemperature);
		static GasReservoir FromGas(const Gas & sourceGas, double temperature = 0);
		static GasReservoir FromGas(const Gas & cold, const Gas & hot);
		
		
		//duplicate functions that static_cast Temperature into a double. Put them down here to avoid cluttering the main interface
		GasReservoir(const Gas & source, Temperature temp) : GasReservoir(source,static_cast<double>(temp)){};
		void Absorb(const Gas & sourceGas, Temperature sourceTemp){Absorb(sourceGas,static_cast<double>(sourceTemp));};
		static GasReservoir Primordial(double mass, Temperature temperature){return Primordial(mass,static_cast<double>(temperature));};
		static GasReservoir FromGas(const Gas & sourceGas, Temperature temperature){return FromGas(sourceGas,static_cast<double>(temperature));};
		static void Transfer(Gas & source, GasReservoir & destination, Temperature temperature, double amount = 1,Move type=Move::Fraction){Transfer(source,destination,static_cast<double>(temperature),amount,type);};
	private:
		static std::pair<double,double> TemperatureBalancer(double amount, Move type,double temperature);
		static void inline TemperatureValidate(double temperature);
};