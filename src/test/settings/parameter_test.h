#pragma once
#include "../catch_amalgamated.hpp" 
#include "../../settings/Parameter.h"
#include "../mock/MockFile.h"
#include "../mock/coutCatch.h"
TEST_CASE("Basic Parameter behaviour","[parameter][settings]")
{
	using Settings::Parameter;
	Parameter<int> P(5,"test");

	//Parameter<T> acts as if it were of type T under equality
	REQUIRE(P == 5);

	//....unless it's a string, because they're weird
	Parameter<std::string> P2("hi","test2");
	REQUIRE(P2.Value == "hi");

	P2.Value = "toast";
	REQUIRE(P2.Value == "toast"); //internal values are mutable
}

TEST_CASE("Parameters have unique identifiers","[parameter][settings][errors]")
{
	using Settings::Parameter;
	Parameter<int> P(5,"test");

	REQUIRE_THROWS(Parameter<double>(3.4,"test"));
	REQUIRE_NOTHROW(Parameter<double>(3.4,"test2"));
	
	//check destructor
	{
		Parameter<int> Q(10,"nameFreedWhenOutOfScope");
		REQUIRE_THROWS(Parameter<int>(10,"nameFreedWhenOutOfScope"));
	}
	REQUIRE_NOTHROW(Parameter<int>(10,"nameFreedWhenOutOfScope"));
}


//generates a suitable argc/argv pair from an input vector<string>
//surprisingly difficult to generate -- need to ensure the object the pointers refer to remain in scope
struct SpoofedStructure
{
	int argc;
	char** argv;
	std::vector<char*> internalVector;
	std::vector<std::string> copyVector;
	SpoofedStructure(std::vector<std::string> input)
	{
		copyVector = input;
		for (const std::string& s : copyVector) {
			internalVector.push_back(const_cast<char*>(s.c_str()));
		}
		argc = internalVector.size();
		argv = internalVector.data(); 
	}
};

TEST_CASE("Parsing values","[parameter][settings][parse][commandline]")
{
	using Settings::Parameter;
	
	std::vector<std::string> initialList = {"spoofed_name","-arg1","-5","-arg2","chicken","-arg3","0"};
	
	SECTION("Basic assignment")
	{
		SpoofedStructure cmd(initialList);

		Parameter<int> nonMatchingArg(1,"test");
		REQUIRE_NOTHROW(nonMatchingArg.Parse(cmd.argc,cmd.argv)); // don't throw if you don't find your value

		Parameter<int> arg1(1,"arg1");
		REQUIRE_NOTHROW(arg1.Parse(cmd.argc,cmd.argv)); //just for safety
		REQUIRE(arg1.Value == -5); //check that value has been updated
		
		Parameter<std::string>arg2("none","arg2");
		REQUIRE_NOTHROW(arg2.Parse(cmd.argc,cmd.argv)); //just for safety
		REQUIRE(arg2.Value == "chicken"); //check that value has been updated
		Parameter<bool>arg3(true,"arg3");
		REQUIRE_NOTHROW(arg3.Parse(cmd.argc,cmd.argv)); //just for safety
		REQUIRE(arg3.Value == false); //check that value has been updated
	}

	SECTION("Mutability - can read int-strings as doubles")
	{
		SpoofedStructure cmd(initialList);
		Parameter<double>arg1Double(1,"arg1");
		arg1Double.Parse(cmd.argc,cmd.argv);
		REQUIRE_THAT(arg1Double.Value,WithinAbs(-5.0,1e-15));
	}

	SECTION("Boolean flag behaviour")
	{
		std::vector<std::string> boolList = {"spoofed_name","-bool1","0","-bool2","1","-flag"};
		SpoofedStructure cmd(boolList);

		Parameter<bool> bool1(true,"bool1",cmd.argc,cmd.argv);
		Parameter<bool> bool2(false,"bool2",cmd.argc,cmd.argv);
		Parameter<bool> flag(false,"flag",cmd.argc,cmd.argv);
		Parameter<bool> unusuedflag(false,"unusedflag",cmd.argc,cmd.argv);

		REQUIRE(bool1==false);
		REQUIRE(bool2==true);
		REQUIRE(flag==true);
		REQUIRE(unusuedflag==false);
	}

	SECTION("Flag/trigger ordering")
	{
		std::vector<std::string> throwList = {"spoofed_name","-noargument","-int","1","-ender"};
		SpoofedStructure cmd(throwList);
		
		REQUIRE_THROWS(Parameter<double>(1.0,"noargument",cmd.argc,cmd.argv));
		REQUIRE_THROWS(Parameter<int>(1.0,"noargument",cmd.argc,cmd.argv));
		REQUIRE_NOTHROW(Parameter<bool>(1.0,"noargument",cmd.argc,cmd.argv));

		REQUIRE_THROWS(Parameter<std::string>("def","ender",cmd.argc,cmd.argv));
		REQUIRE_NOTHROW(Parameter<bool>(1.0,"ender",cmd.argc,cmd.argv));
	}

	SECTION("Config files")
	{
		MockFile file;
		file << "arg1 1\n";
		file << "arg2 hello world\n";
		file << "arg3 1 2 3 4\n";
		file << "lonearg\n";
		// SECTION("Basic parsing")
		// {
			Parameter<int> intTest(0,"arg1");
			intTest.Configure(file.Path," ");
			REQUIRE(intTest == 1);

			Parameter<std::string> stringTest("null","arg2");
			stringTest.Configure(file.Path," ");
			REQUIRE(stringTest.Value == "hello world");

			{
				Parameter<int> vecThrow(0,"arg3");
				auto cout = capture_stdout([&](){REQUIRE_THROWS(vecThrow.Configure(file.Path," "));});
				REQUIRE_THAT(cout,ContainsSubstring("ERROR"));
			}
			{
				Parameter<std::vector<int>> vecTest({},"arg3"," ");
				REQUIRE_NOTHROW(vecTest.Configure(file.Path," "));
			}

			Parameter<bool> failTest(0,"lonearg");
			auto cout = capture_stdout([&](){REQUIRE_THROWS(failTest.Configure(file.Path," "));});

	}
}