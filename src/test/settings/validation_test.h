#pragma once
#include "../catch_amalgamated.hpp" 
#include "../mock/mockObjects.h"

#include "../../settings/SettingGroups.h"


TEST_CASE("Non-Validating objects","[settings][validation]")
{
	//this mostly exists to throw errors when validators are defined and the unit tests not updated!
	REQUIRE_FALSE(StellarSettings().Validate());
	REQUIRE_FALSE(ThermalSettings().Validate());
	REQUIRE_FALSE(YieldSettings().Validate());
}


TEST_CASE("System Settings Object","[settings][validation][system]")
{
	REQUIRE_NOTHROW(SystemSettings()); //ensure the object can initialise properly

	SystemSettings Tester;


	auto cmd = ArgSpoofer({"-v","5"});
	REQUIRE_NOTHROW(Tester.Parse(cmd.argc,cmd.argv));

	REQUIRE(Tester.Verbosity == 5); // Validation command is not run immediately. This matters because it allows all values to be set before validation, so values can be compared
	REQUIRE(Tester.Validate()); //a default validator returns false -- this checks that custom code is actually running
	REQUIRE(Tester.Verbosity == 3); // Validation hems in the value to the proper amount


}

TEST_CASE("AbundanceSettings Object","[settings][validation][abundance]")
{
	REQUIRE_NOTHROW(AbundanceSettings()); //ensure the object can initialise properly

	 AbundanceSettings Tester;

	

	SECTION("Default Primordial initialisation")
	{
		REQUIRE(Tester.Validate());//check that a manual validation is running
		using namespace Element;
		auto & primordial = Tester.PrimordialAbundances.Value();

		double norm = 0;
		std::vector<Element::Species> alreadyTested = {Element::UnspecifiedMetal};//add unspecified metal here -- it default initialises to a non-zero value (the overflow)
		#define REQUIRE_FRAC(element,comparison) REQUIRE_THAT(primordial[element]/comparison,WithinAbs(1.0,1e-3)); alreadyTested.push_back(element); norm += primordial[element];

		REQUIRE_FRAC(Hydrogen,Tester.PrimordialHydrogen);
		REQUIRE_FRAC(Helium,Tester.PrimordialHelium);
		REQUIRE_FRAC(Magnesium,Tester.PrimordialMagnesium);
		REQUIRE_FRAC(Iron,Tester.PrimordialIron);

		for (int i = 0; i < Element::Count; ++i)
		{
			auto el = Element::FromInteger(i);
			if (std::find(alreadyTested.begin(),alreadyTested.end(),el) == alreadyTested.end())
			{
				if (primordial[el] > 0)
				{
					LOG(WARN) << Element::Name(el) << " is expected to default initialise to 0. You probably need to update your unit tests, because it's about to fail.";
				}
				REQUIRE(primordial[el] == 0);
			}
		}
		#undef REQUIRE_FRAC

		REQUIRE_THAT(primordial[Element::UnspecifiedMetal],WithinAbs(1.0 - norm,1e-5)); //now the unspecified metal check
	}

	 SECTION("Direct modification of primordial abundance array")
	 {
		auto cmd = ArgSpoofer({"-primordial-abundances","[0,1,2]"});
		REQUIRE_NOTHROW(Tester.Parse(cmd.argc,cmd.argv));

		//check the vector was read in properly, exactly as specified
		REQUIRE(Tester.PrimordialAbundances.Value() == std::vector<double>({0,1,2}));
		
		auto msg = capture_stdout([&](){Tester.Validate();});
		REQUIRE_THAT(msg,ContainsSubstring("[WARN]")); //check that a warning is thrown

		REQUIRE(Tester.PrimordialAbundances.Value().size() == Element::Count) ;//check that the vector is now properly resized
		
		//check the normalisation here as well as later -- we explicitly passed values greater than 1 in, so healthy to check
		auto & array = Tester.PrimordialAbundances.Value();
		double normalisation = std::accumulate(array.begin(),array.end(),0.0);
		REQUIRE_THAT(normalisation,WithinAbs(1.0,1e-9));
	 }

	SECTION("Testing the primordial read-from-file")
	{
		std::vector<std::pair<std::string,double>> spoofedVals;
		double v = 0;

		for (int i = 0; i < Element::Count; ++i)
		{
			auto el = Element::FromInteger(i);
			bool useLongName = (i < Element::Count/2); //deliberately mix long and short names to test robustness
			std::string name = Element::Name(el,useLongName);
			std::pair<std::string,double> q(name,i+1);
			v += q.second;
			spoofedVals.push_back(q);
		}

		MockFile file;
		for (auto & pair : spoofedVals)
		{
			file << pair.first + " " + std::to_string(pair.second) + "\n";
			pair.second/= v; //normalise *after* loading in, to ensure that the code normalises itself properly 
		}

		auto cmd = ArgSpoofer({"-primordial-abundances-source",file.Name()});
		REQUIRE_NOTHROW(Tester.Parse(cmd.argc,cmd.argv));
		REQUIRE_NOTHROW(Tester.Validate());

		for (auto & pair : spoofedVals)
		{
			auto el = Element::FromName(pair.first);
			REQUIRE_THAT(Tester.PrimordialAbundances.Value()[el],WithinAbs(pair.second,1e-8));
		}
	}

}