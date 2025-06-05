#pragma once
#include "../catch_extended.h" 
#include "../../galaxy/gas/gas.h"
#include "../../utility/Random.h"
using namespace Catch::Matchers;
Random R;
std::vector<double> RandomComposition(double mass = 1)
{
	std::vector<double> vec(Element::Count,0.0);
	double v =0;
	for (int i = 0; i < Element::Count; ++i)
	{
		double r = std::max(0.0,R.Normal(0.1,0.05));
		v+=r;
		vec[i] = r;
	}
	for (auto & element: vec)
	{
		element*=mass/v;
	}
	return vec;
}


TEST_CASE("Gas Constructors","[gas][physics][galaxy][constructors]")
{
	// Settings.ValidateAll();
	SECTION("Factory Functions")
	{
		SECTION("Gas::Empty")
		{
			Gas gas = Gas::Empty();

			REQUIRE(gas.Mass() == 0); //default initialisation is of a zero-mass entity

			for (int element = 0; element < Element::Count; ++element)
			{
				REQUIRE(gas.FractionOf(Element::FromInteger(element)) == 0); //We require that lim(total mass->0) (element mass)/(total mass) = 0 by definition
			}
		}

		SECTION("Gas::WithComposition")
		{
			// Gas gas = Gas();
			std::vector<double> masses = {1,10,100};
			
			
			
			for (auto mass : masses)
			{
				auto composition = RandomComposition();
				
				
				Gas gas = Gas::WithComposition(mass,composition);

				REQUIRE_THAT(gas.Mass(), WithinAbs(mass,1e-8));
				for (int element = 0; element < Element::Count; ++element)
				{
					double targetFraction = composition[element];
					REQUIRE_THAT(gas.FractionOf(Element::FromInteger(element)), WithinAbs(targetFraction,1e-8));
				}
			}

			//unusual mass checks
			auto composition = RandomComposition();
			auto msg = capture_stdout([&](){REQUIRE_THROWS(Gas::WithComposition(-1,composition));});
			std::vector<double> badComposition(0,4);
			msg = capture_stdout([&](){REQUIRE_THROWS(Gas::WithComposition(1,badComposition));});

			msg = capture_stdout([&](){Gas::WithComposition(0,composition);});
			REQUIRE_THAT(msg,ContainsSubstring("[WARN]"));
			capture_stdout([&](){auto g = Gas::WithComposition(0,composition); 
			
			for (int i = 0; i < Element::Count; ++i)
			{
				REQUIRE_THAT(g.FractionOf(Element::FromInteger(i)),WithinAbs(0.0,1e-8));
			}
			
			});

		}

		SECTION("Gas::WithMass")
		{
			std::vector<double> masses = {1,10,100};

			for (auto mass : masses)
			{
				auto composition = RandomComposition(mass);
				Gas gas = Gas::WithMass(composition);

				REQUIRE_THAT(gas.Mass(), WithinAbs(mass,1e-8));
				for (int element = 0; element < Element::Count; ++element)
				{
					REQUIRE_THAT(gas.MassOf(Element::FromInteger(element)), WithinAbs(composition[element],1e-8));
				}
			}
		}
		SECTION("Gas::Primordial")
		{
			
			std::vector<double> masses = {1,10,100};

			for (auto mass : masses)
			{
				auto composition = Settings.Abundance.PrimordialAbundances.Value();
				Gas gas = Gas::Primordial(mass);

				REQUIRE_THAT(gas.Mass(), WithinAbs(mass,1e-8));
				for (int element = 0; element < Element::Count; ++element)
				{
					double targetFraction = composition[element];


					REQUIRE_THAT(gas.FractionOf(Element::FromInteger(element)), WithinAbs(targetFraction,1e-8));
				}
			}
		}

		SECTION("Gas::WithSameComposition")
		{
			auto composition = RandomComposition();
			Gas baseGas = Gas::WithComposition(1,composition);

			Gas CopiedGas = Gas::WithSameComposition(100,baseGas);
			REQUIRE(CopiedGas.Mass() == 100);
			
			for (int i = 0; i < Element::Count; ++i)
			{
				auto element = Element::FromInteger(i);
				REQUIRE_THAT(CopiedGas.FractionOf(element), WithinAbs(baseGas.FractionOf(element),1e-8));
			}

			auto emptyGas = Gas::Empty();
			auto msg = capture_stdout([&](){REQUIRE_THROWS(Gas::WithSameComposition(10,emptyGas));});
			REQUIRE_THAT(msg,ContainsSubstring("[ERROR]"));
		}
	}
}

TEST_CASE("Basic Gas modification & updates","[gas][physics][galaxy][interface]")
{
	auto g = Gas::Empty();

	SECTION("Basic access changes")
	{
		double hMass = 4;
		g[Element::Hydrogen] = hMass;
		REQUIRE(g.MassOf(Element::Hydrogen) == hMass);
		REQUIRE(g.Mass() == hMass);
		
		double zMass = 1;
		g[Element::Cobalt] += zMass;
		REQUIRE_THAT(g.Metallicity(), WithinAbs((zMass)/(hMass + zMass),1e-8));
	}

	SECTION("Const/mutable checks")
	{
		double heMass = 2;
		g[Element::Helium] += heMass; //place g into a dirty state
		
		const Gas g2 = g; //g2 inherits g's dirty state

		REQUIRE(g2.Mass() == heMass);
		REQUIRE(g2.Metallicity() == 0);
	}



	SECTION("Gas::Absorb(element,amount) is precise insertion")
	{
		auto g = Gas::Empty();
		
		double expectedMass = 0;
		for (int i = 0; i < Element::Count; ++i)
		{
			expectedMass += i+1;
			auto element = Element::FromInteger(i);
			g.Absorb(element,i+1);
			REQUIRE(g.MassOf(element) == i+1);
			REQUIRE(g.Mass() == expectedMass);
		}
	}


	SECTION("Gas::Absorb(Gas) adds two gases together")
	{
		auto g = Gas::Empty();
		g[Element::Europium] = 2;

		auto absorbingGas = Gas::Empty();

		REQUIRE_NOTHROW(absorbingGas.Absorb(g));
		REQUIRE(g.Mass() == 2); //absorbing does not change mass of absorbed gas, is not a transfer
		REQUIRE(absorbingGas.Mass() == 2); 
		REQUIRE(absorbingGas[Element::Europium] == 2);
		REQUIRE_THAT(absorbingGas.FractionOf(Element::Europium), WithinAbs(1,1e-8));

		g[Element::Hydrogen] += 2;
		absorbingGas.Absorb(g); //absorb again, should have Eu = 4, H = 2
		REQUIRE_THAT(absorbingGas[Element::Hydrogen],WithinAbs(2,1e-8));
		REQUIRE_THAT(absorbingGas[Element::Europium],WithinAbs(4,1e-8));
		
	}
}


TEST_CASE("Gas depletion","[gas][physics][galaxy][depletion]")
{
	SECTION("Gas::DepleteByFraction")
	{
		auto composition = RandomComposition();
		Gas baseGas = Gas::WithComposition(1,composition);


		SECTION("Check error throwing")
		{
			//now check errors
			//test the extreme ends of the spectrum
			REQUIRE_NOTHROW(baseGas.Deplete(1,Move::Fraction));
			baseGas = Gas::WithComposition(1,composition); //refill after depletion
			REQUIRE_NO_WARN(baseGas.Deplete(0,Move::Fraction));

			//and now outside those domains
			auto msg = REQUIRE_ERROR(baseGas.Deplete(2,Move::Fraction));
			REQUIRE_THAT(msg,ContainsSubstring("must be in range [0,1]"));
			msg = REQUIRE_ERROR(baseGas.Deplete(-1,Move::Fraction));
			REQUIRE_THAT(msg,ContainsSubstring("must be in range [0,1]"));

		}

		SECTION("Check composition remains unchanged, and that object is permanently changed")
		{
			double reducingFraction = 0.5;
			for (int repeat = 0; repeat < 5; ++repeat)
			{
				baseGas.Deplete(reducingFraction,Move::Fraction);
				for (int element = 0; element < Element::Count; ++element)
				{
					auto species = Element::FromInteger(element);
					double targetFrac = composition[species];
					double targetMass = targetFrac * pow(reducingFraction,repeat+1);
					REQUIRE_THAT(baseGas.FractionOf(species), WithinAbs(targetFrac,1e-8));
					REQUIRE_THAT(baseGas.MassOf(species), WithinAbs(targetMass,1e-8));
				}
			}
		}

		
	}

	SECTION("Gas::DepleteByMass;")
	{
		auto composition = RandomComposition();
		double startMass = 3;
		Gas baseGas = Gas::WithComposition(3,composition);

		SECTION("Check error throwing")
		{
			//now check errors
			//test the extreme ends of the spectrum
			REQUIRE_NOTHROW(baseGas.Deplete(startMass,Move::Mass));
			baseGas = Gas::WithComposition(startMass,composition);//refill after depletion
			REQUIRE_NOTHROW(baseGas.Deplete(0,Move::Mass));


			//and now outside those domains
			std::string msg = REQUIRE_ERROR(baseGas.Deplete(startMass + 1,Move::Mass));
			REQUIRE_THAT(msg,ContainsSubstring("must be in range [0,3]"));
			msg = REQUIRE_ERROR(baseGas.Deplete(-2,Move::Mass));
			REQUIRE_THAT(msg,ContainsSubstring("must be in range [0,3]"));
		}
		
		SECTION("Check composition remains unchanged, and that object is permanently changed")
		{
			double reducingMass = 0.5;
			for (int repeat = 0; repeat < 5; ++repeat)
			{
				baseGas.Deplete(reducingMass,Move::Mass);
				double expectedMass = startMass - reducingMass*(repeat+1);
				REQUIRE_THAT(baseGas.Mass(), WithinAbs(expectedMass,1e-8));
				for (int element = 0; element < Element::Count; ++element)
				{
					auto species = Element::FromInteger(element);
					double targetFrac = composition[species];
					double targetMass = targetFrac * expectedMass;
					REQUIRE_THAT(baseGas.FractionOf(species), WithinAbs(targetFrac,1e-8));
					REQUIRE_THAT(baseGas.MassOf(species), WithinAbs(targetMass,1e-8));
				}
			}
		}
	}
}

TEST_CASE("Gas transfer mechanisms","[physics][gas][galaxy][transfer]")
{
	SECTION("Fractional transfer")
	{
		auto acceptingGas = Gas::Empty();

		SECTION("Standard usage")
		{
			double gasMass = 1;
			double transferFraction = 0.5;
			for (int repeat = 0; repeat < 5; ++repeat)
			{
				auto comp = RandomComposition();
				auto adderGas = Gas::WithComposition(gasMass,comp);
				
				Gas::Transfer(adderGas,acceptingGas,transferFraction);

				double expectedMass = gasMass * transferFraction * (repeat +1);
				REQUIRE_THAT(acceptingGas.Mass(),WithinAbs(expectedMass,1e-8));

				if (repeat == 0)
				{
					REQUIRE(acceptingGas.Composition() == adderGas.Composition()); //check that the first time copies the composition properly....
				}
				else
				{
					REQUIRE_FALSE(acceptingGas.Composition() == adderGas.Composition()); //but after that, it's a mixture of previous compositions
				}
			}
		}

		SECTION("Warnings and errors")
		{
			auto nonEmptyGas = Gas::Empty(); nonEmptyGas.Absorb(Element::Hydrogen,5);

			//note using the default overload here -- no Move::Fraction call. This ensures the expected default behaviour is maintained!
			REQUIRE_ERROR(Gas::Transfer(nonEmptyGas,acceptingGas,1.5)); //move more than 100%
			REQUIRE_ERROR(Gas::Transfer(nonEmptyGas,acceptingGas,-1)); //move less than 0%

			auto msg = REQUIRE_WARN(Gas::Transfer(acceptingGas,nonEmptyGas,0.5)); //gives a warning that moving any fraction > 0 of a zero-mass is pointless
			REQUIRE_THAT(msg,ContainsSubstring("Nothing happened"));

			REQUIRE_NO_WARN(Gas::Transfer(acceptingGas,nonEmptyGas,0)); //no warning if you move 0% of a 0-mass object
		}
	}

	SECTION("Mass transfer")
	{
		auto acceptingGas = Gas::Empty();

		SECTION("Standard usage")
		{
			double gasMass = 3;
			double transferMass = 1.5;
			for (int repeat = 0; repeat < 5; ++repeat)
			{
				auto comp = RandomComposition();
				auto adderGas = Gas::WithComposition(gasMass,comp);
				
				Gas::Transfer(adderGas,acceptingGas,transferMass,Move::Mass);

				double expectedMass = transferMass * (repeat +1);
				REQUIRE_THAT(acceptingGas.Mass(),WithinAbs(expectedMass,1e-8));

				if (repeat == 0)
				{
					REQUIRE(acceptingGas.Composition() == adderGas.Composition()); //check that the first time copies the composition properly....
				}
				else
				{
					REQUIRE_FALSE(acceptingGas.Composition() == adderGas.Composition()); //but after that, it's a mixture of previous compositions
				}
			}
		}

		SECTION("Warnings and errors")
		{
			auto nonEmptyGas = Gas::Empty(); nonEmptyGas.Absorb(Element::Hydrogen,5);

			REQUIRE_ERROR(Gas::Transfer(nonEmptyGas,acceptingGas,-1,Move::Mass)); //move less than 0%
			REQUIRE_ERROR(Gas::Transfer(acceptingGas,nonEmptyGas,0.5,Move::Mass)); //gives a warning that moving any fraction > 0 of a zero-mass is pointless
			REQUIRE_ERROR(Gas::Transfer(nonEmptyGas,acceptingGas,6,Move::Mass)); //moving more than 100% throws

			// REQUIRE_THAT(msg,ContainsSubstring("Nothing happened"));

			REQUIRE_NO_WARN(Gas::Transfer(acceptingGas,nonEmptyGas,0)); //no warning if you move 0% of a 0-mass object
		}
	}


	SECTION("Full transfer using Gas::Transfer()")
	{
		SECTION("Standard usage")
		{
			auto comp = RandomComposition();
			auto sourceGas = Gas::WithComposition(50.0, comp); // Source with 50 mass
			auto destinationGas = Gas::Empty();

			Gas::Transfer(sourceGas, destinationGas); // Transfer all from source to destination

			REQUIRE_THAT(sourceGas.Mass(), WithinAbs(0.0, 1e-8)); // Source should be empty
			REQUIRE_THAT(destinationGas.Mass(), WithinAbs(50.0, 1e-8)); // Destination should have full mass

			// Composition should be identical to original source
			for (int i = 0; i < Element::Count; ++i)
			{
				REQUIRE_THAT(destinationGas.FractionOf(Element::FromInteger(i)), WithinAbs(comp[i], 1e-8));
			}
		}

		SECTION("Transfer from empty source")
		{
			auto emptySource = Gas::Empty();
			auto destinationGas = Gas::WithComposition(10.0, RandomComposition()); // Non-empty destination

			double initialDestMass = destinationGas.Mass();
			auto msg = REQUIRE_WARN(Gas::Transfer(emptySource, destinationGas)); // Transfer from empty source
			REQUIRE_THAT(msg,ContainsSubstring("Nothing happened"));

			REQUIRE_THAT(emptySource.Mass(), WithinAbs(0.0, 1e-8)); // Source remains empty
			REQUIRE_THAT(destinationGas.Mass(), WithinAbs(initialDestMass, 1e-8)); // Destination mass unchanged
			// Composition of destination should also remain unchanged, but harder to assert without storing initial composition.
			// If your test suite provides a way to deep-compare Gas objects, that would be ideal here.
		}
	}
}