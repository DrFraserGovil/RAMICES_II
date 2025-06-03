#pragma once
#include "../catch_amalgamated.hpp" 


#include "../mock/MockFile.h"
#include "../mock/coutCatch.h"


#define SETTINGS_CATEGORY TestSettings
#define SETTINGS_FILE "../test/settings/test.def"
#include "../../settings/SettingsConstructor.h"

TEST_CASE("Compile time constraints on ","[settings][compilation]")
{
	//Many of the macro properties are enforced at compile time -- this test case therefore serves as a compile-time unit test
	//the fact that this compiles is proof it works!
	
	
	TestSettings CompiledTest;
	int expectedInt = 3;
	REQUIRE(CompiledTest.TestInt == expectedInt);
	
	double expectedDouble = 0.0;
	REQUIRE(CompiledTest.TestDouble == expectedDouble);

	bool expectedBool = false;
	REQUIRE(CompiledTest.TestBool == expectedBool);

	//This doesn't work, because our unit tests aren't compile times, but this is just a reminder that the constructor undefs any associated values, so you can't re-call!
	//REQUIRE_COMPILER_THROWS(#include "../../settings/SettingsConstructor.h")
}

TEST_CASE("Parsing Settings-Macro Object","[settings][parsing]")
{
	ArgSpoofer cmd({"-int","-9","-double","99.9","-bool","true"});

	TestSettings ParseTest;
	ParseTest.Parse(cmd.argc,cmd.argv);

	int expectedInt = -9;
	REQUIRE(ParseTest.TestInt == expectedInt);
	
	double expectedDouble = 99.9;
	REQUIRE(ParseTest.TestDouble == expectedDouble);

	bool expectedBool = true;
	REQUIRE(ParseTest.TestBool == expectedBool);
}

TEST_CASE("Configuring Settings-Macro Object","[settings][configure]")
{
	MockFile file;
	file << "int 18 \n";
	file << "double -17e3 \n";
	file << "bool 1 \n";

	TestSettings ConfigureTest;
	ConfigureTest.Configure(file.Name()," ");

	int expectedInt = 18;
	REQUIRE(ConfigureTest.TestInt == expectedInt);
	
	double expectedDouble = -17000;
	REQUIRE(ConfigureTest.TestDouble == expectedDouble);

	bool expectedBool = true;
	REQUIRE(ConfigureTest.TestBool == expectedBool);
}

#define SETTINGS_CATEGORY SettingsValidator
#define SETTINGS_FILE "../test/settings/test.def"
#define SETTINGS_VALIDATE
#include "../../settings/SettingsConstructor.h"

bool SettingsValidator::Validate()
{
	if (TestInt < 0)
	{
		TestDouble.SetValue(8,true);
	}
	return true;
}
TEST_CASE("Settings-Macro with validation","[settings][parse][configure][validation]")
{
	SettingsValidator ValidateTest;

	SECTION("Parsing-validation")
	{
		REQUIRE_FALSE(ValidateTest.TestDouble == 8);
		ArgSpoofer cmd({"-int","-9","-double","99.9","-bool","true"});
		ValidateTest.Parse(cmd.argc,cmd.argv);
		ValidateTest.Validate();
		int valueEnforcedByValidation = 8;
		REQUIRE(ValidateTest.TestDouble == valueEnforcedByValidation);
	}

}

typedef std::vector<int> vec;
TEST_CASE("Writing Settings to file (and recovering it)","[settings][save]")
{
	std::stringstream capture;
	
	SettingsValidator SavedToFile;
	SavedToFile.TestVector.SetValue(std::vector<int>({1,2,3,4,5,6}),true);
	REQUIRE_NOTHROW(SavedToFile.ToStream(capture,"__"));
	MockFile file;
	file << capture.str();



	SettingsValidator Recovered;
	REQUIRE_NOTHROW(Recovered.Configure(file.Name(),"__"));

	REQUIRE(SavedToFile.TestInt == Recovered.TestInt);
	REQUIRE(SavedToFile.TestDouble == Recovered.TestDouble);
	REQUIRE(SavedToFile.TestBool == Recovered.TestBool);
	
	int initialSize =((vec)SavedToFile.TestVector).size();
	REQUIRE(initialSize == ((vec)Recovered.TestVector).size());

	for (int i = 0; i < initialSize; ++i)
	{
		REQUIRE(((vec)SavedToFile.TestVector)[i] == ((vec)Recovered.TestVector)[i]);
	}
}