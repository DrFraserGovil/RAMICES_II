#pragma once
#include "../catch_extended.h" 
#include "../../galaxy/gas/reservoir.h"
#include "../mock/mockObjects.h"
#include "../../utility/Random.h"
#include "gas_tests.h" //inherit the random composition vector

TEST_CASE("Reservoir constructors","[reservoir][gas][physics][constructors]")
{
	SECTION("Reservoir::Empty")
	{
		REQUIRE_NOTHROW(GasReservoir::Empty());//check to make sure it works
		auto reservoir = GasReservoir::Empty();

		REQUIRE(reservoir.TotalMass() == 0);
		REQUIRE(reservoir.Cold.Mass() == 0);
		REQUIRE(reservoir.Hot.Mass() == 0);
		REQUIRE(reservoir.GetTemperature() == 0); //we define empty reservoirs to have 0 temp

	}

	SECTION("Reservoir::Primordial")
	{
		auto expectedComp = Settings.Abundance.PrimordialAbundances.Value();
		double primordialMass = 10;		
		
		SECTION("Double-Temperatures")
		{
			std::vector<double> temps = {0,0.2,0.8,1.0};
			for (auto temp : temps)
			{
				auto reservoir = GasReservoir::Primordial(primordialMass,temp);
				REQUIRE_APPROX(reservoir.TotalMass(),primordialMass);
				REQUIRE_APPROX(reservoir.Cold.Mass(),primordialMass*(1.0 - temp));
				REQUIRE_APPROX(reservoir.Hot.Mass(),primordialMass*temp);
				REQUIRE_APPROX(reservoir.GetTemperature(),temp);
				if (temp < 1.0)
				{
					REQUIRE(reservoir.Cold.Composition() == expectedComp);
				}
				if (temp > 0)
				{
					REQUIRE(reservoir.Hot.Composition() == expectedComp);
				}
			}
		}

		SECTION("Enum Temperatures")
		{
			//now for the enums
			auto coldReservoir = GasReservoir::Primordial(primordialMass,Temperature::Cold);
			REQUIRE_APPROX(coldReservoir.TotalMass(), coldReservoir.Cold.Mass());
			REQUIRE_APPROX(coldReservoir.Hot.Mass(), 0.0);
			REQUIRE(coldReservoir.GetTemperature() == 0);
			REQUIRE(coldReservoir.Cold.Composition() == expectedComp);
			
			auto hotReservoir = GasReservoir::Primordial(primordialMass,Temperature::Hot);
			REQUIRE_APPROX(hotReservoir.TotalMass(), hotReservoir.Hot.Mass());
			REQUIRE_APPROX(hotReservoir.Cold.Mass(), 0.0);
			REQUIRE(hotReservoir.GetTemperature() == 1.0);
			REQUIRE(hotReservoir.Hot.Composition() == expectedComp);
		}

	}


	SECTION("Reservoir::FromGas(source,temp)")
	{
		auto expectedComp = RandomComposition();

		double originalMass = 10;		
		Gas originalGas = Gas::WithComposition(originalMass,expectedComp);

		SECTION("Double-Temperatures")
		{
			std::vector<double> temps = {0,0.2,0.8,1.0};
			for (auto temp : temps)
			{
				auto reservoir = GasReservoir::FromGas(originalGas,temp);
				REQUIRE_APPROX(reservoir.TotalMass(),originalMass);
				REQUIRE_APPROX(reservoir.Cold.Mass(),originalMass*(1.0 - temp));
				REQUIRE_APPROX(reservoir.Hot.Mass(),originalMass*temp);
				REQUIRE_APPROX(reservoir.GetTemperature(),temp);
				if (temp < 1.0)
				{
					REQUIRE(reservoir.Cold.Composition() == expectedComp);
				}
				if (temp > 0)
				{
					REQUIRE(reservoir.Hot.Composition() == expectedComp);
				}
			}
		}

		SECTION("Enum Temperatures")
		{
			//now for the enums
			auto coldReservoir = GasReservoir::FromGas(originalGas,Temperature::Cold);
			REQUIRE_APPROX(coldReservoir.TotalMass(), coldReservoir.Cold.Mass());
			REQUIRE_APPROX(coldReservoir.Hot.Mass(), 0.0);
			REQUIRE(coldReservoir.GetTemperature() == 0);
			REQUIRE(coldReservoir.Cold.Composition() == expectedComp);
			
			auto hotReservoir = GasReservoir::FromGas(originalGas,Temperature::Hot);
			REQUIRE_APPROX(hotReservoir.TotalMass(), hotReservoir.Hot.Mass());
			REQUIRE_APPROX(hotReservoir.Cold.Mass(), 0.0);
			REQUIRE(hotReservoir.GetTemperature() == 1.0);
			REQUIRE(hotReservoir.Hot.Composition() == expectedComp);
		}

	}
	SECTION("Reservoir::FromGas(gas,gas)")
	{
		auto coldComp= RandomComposition();
		auto hotComp= RandomComposition();

		std::vector<double> masses = {0,1,2,3,4};

		for (auto coldMass : masses)
		{
			auto coldGas = Gas::WithComposition(coldMass,coldComp,true);
			for (auto hotMass : masses)
			{
				auto hotGas = Gas::WithComposition(hotMass,hotComp,true);
				auto reservoir = GasReservoir::FromGas(coldGas,hotGas);

				double expectedTotalMass = hotMass + coldMass;
				REQUIRE_APPROX(reservoir.TotalMass(),expectedTotalMass);

				double expectedTemp = hotMass/expectedTotalMass;
				if (expectedTotalMass > 0) REQUIRE_APPROX(reservoir.GetTemperature(),expectedTemp);
				else REQUIRE(reservoir.GetTemperature() == 0);
				if (coldMass > 0)	REQUIRE(reservoir.Cold.Composition() == coldComp);
				if (hotMass > 0) REQUIRE(reservoir.Hot.Composition() == hotComp);
				REQUIRE_APPROX(reservoir.Cold.Mass(),coldMass);
				REQUIRE_APPROX(reservoir.Hot.Mass(),hotMass);

			}
		}
		

	}
}

TEST_CASE("Reservoir Depletion","[reservoir][gas][physics][depletion]")
{
	auto coldComp= RandomComposition();
	auto hotComp= RandomComposition();
	double totalMass = 10;
	double temp = R.UniformDouble(0.1,0.9);

	auto cold = Gas::WithComposition(totalMass*(1.0 - temp),coldComp);
	auto hot = Gas::WithComposition(totalMass * temp,hotComp);

	auto reservoir = GasReservoir::FromGas(cold,hot);

	SECTION("Uniform-fraction depletion")
	{
		double depleteFrac = 0.1;
		double expectedMass = totalMass;
		for (int repeat = 0; repeat < 5; ++repeat)
		{
			expectedMass*=(1.0 - depleteFrac);
			reservoir.Deplete(depleteFrac);
			REQUIRE_APPROX(reservoir.TotalMass(),expectedMass); //mass is gone
			REQUIRE_VEC_APPROX(reservoir.Cold.Composition(), coldComp); //cold composition remains the same
			REQUIRE_VEC_APPROX(reservoir.Hot.Composition(), hotComp); //hot composition remains the same
			REQUIRE_APPROX(reservoir.GetTemperature(),temp); //temperature remains the same

		}
	}

	SECTION("Uniform-mass depletion")
	{
		double depleteMass = 0.4;
		double expectedMass = totalMass;
		for (int repeat = 0; repeat < 5; ++repeat)
		{
			expectedMass -= depleteMass;
			reservoir.Deplete(depleteMass,Move::Mass);
			REQUIRE_APPROX(reservoir.TotalMass(),expectedMass); //mass is gone
			REQUIRE_VEC_APPROX(reservoir.Cold.Composition(), coldComp); //cold composition remains the same
			REQUIRE_VEC_APPROX(reservoir.Hot.Composition(), hotComp); //hot composition remains the same
			REQUIRE_APPROX(reservoir.GetTemperature(),temp); //temperature remains the same
		}
	}

	SECTION("Cold depletion")
	{
		double depleteFrac = 0.1;
		double expectedHotMass = totalMass * temp;
		double expectedColdMass = totalMass * (1.0 - temp);
		for (int repeat = 0; repeat < 5; ++repeat)
		{
			expectedColdMass*=(1.0 - depleteFrac);
			reservoir.Deplete(Temperature::Cold,depleteFrac);
			REQUIRE_APPROX(reservoir.TotalMass(),expectedColdMass+expectedHotMass); //mass is gone

			REQUIRE_VEC_APPROX(reservoir.Cold.Composition(), coldComp); //cold composition remains the same
			REQUIRE_VEC_APPROX(reservoir.Hot.Composition(), hotComp); //hot composition remains the same

			double expectedTemp = expectedHotMass/(expectedColdMass + expectedHotMass);
			REQUIRE_APPROX(reservoir.GetTemperature(),expectedTemp); //temperature is changing due to asymmetric depletion, but to predicted value

		}
	}

	SECTION("Hot depletion")
	{
		double depleteFrac = 0.1;
		double expectedHotMass = totalMass * temp;
		double expectedColdMass = totalMass * (1.0 - temp);
		for (int repeat = 0; repeat < 5; ++repeat)
		{
			expectedHotMass*=(1.0 - depleteFrac);
			reservoir.Deplete(Temperature::Hot,depleteFrac);
			REQUIRE_APPROX(reservoir.TotalMass(),expectedColdMass+expectedHotMass); //mass is gone

			REQUIRE_VEC_APPROX(reservoir.Cold.Composition(), coldComp); //cold composition remains the same
			REQUIRE_VEC_APPROX(reservoir.Hot.Composition(), hotComp); //hot composition remains the same

			double expectedTemp = expectedHotMass/(expectedColdMass + expectedHotMass);
			REQUIRE_APPROX(reservoir.GetTemperature(),expectedTemp); //temperature is changing due to asymmetric depletion, but to predicted value

		}
	}
}


std::vector<double> expectedComposition(std::vector<double> base, std::vector<double> mixer,double mix)
{
	auto copy = std::vector<double>(base.size(),0);
	for (int i = 0; i < base.size(); ++i)
	{
		copy[i] = base[i] * (1.0 - mix) + mixer[i] * mix;
	}
	return copy;
}

TEST_CASE("Reservoir transfers")
{
	auto coldComp= RandomComposition();
	auto hotComp= RandomComposition();
	double totalMass = 10;
	double temp = R.UniformDouble(0.1,0.9);

	auto cold = Gas::WithComposition(totalMass*(1.0 - temp),coldComp);
	auto hot = Gas::WithComposition(totalMass * temp,hotComp);

	
	SECTION("Transfer in a gas object")
	{
		double inputMass = 5;
		auto newComp = RandomComposition();
		
		std::vector<double> temps = {0.0,0.2,0.5,0.6,1.0};
		std::vector<double> transferFrac = {0.1,0.5,1};
		SECTION("Double-temps")
		{
			for (auto transfer : transferFrac)
			{
				for (auto sourceTemp : temps)
				{
					auto reservoir = GasReservoir::FromGas(cold,hot);
					auto source = Gas::WithComposition(inputMass,newComp);

					GasReservoir::Transfer(source,reservoir,sourceTemp,transfer,Move::Fraction);

					double expectedColdMass = totalMass * (1.0 - temp) + inputMass*transfer*(1.0 - sourceTemp);
					double expectedHotMass = totalMass * ( temp) + inputMass*transfer*(sourceTemp);
					REQUIRE_APPROX(reservoir.Cold.Mass(),expectedColdMass);
					REQUIRE_APPROX(reservoir.Hot.Mass(),expectedHotMass);

					REQUIRE_APPROX(source.Mass(),inputMass*(1.0-transfer));//check mass lost from source
					REQUIRE_VEC_APPROX(source.Composition(),newComp);//but mass unchanged

					REQUIRE_VEC_APPROX(reservoir.Cold.Composition(),expectedComposition(coldComp,newComp,(1.0-sourceTemp)*transfer));
					REQUIRE_VEC_APPROX(reservoir.Hot.Composition(),expectedComposition(hotComp,newComp,(sourceTemp)*transfer));
					
				}
			}
		}
	}
}