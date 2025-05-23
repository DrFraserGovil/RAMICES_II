#pragma once
#include "../catch_amalgamated.hpp" 
#include <string>
#include <string_view>
#include "../../utility/fileparser.h"


TEST_CASE("Reads files correctly","[file][read]")
{
	MockFile F;
	int linesWritten = 100;
	for (int i = 0; i < linesWritten; ++i)
	{
		F << "test\n";
	}

	int linesRead = 0;
	forLineIn(F.Path,
		[&](const std::string line)
		{
			REQUIRE(line == "test");
			linesRead++;
		}
	);
	REQUIRE(linesRead == linesWritten);
}

void GenerateVectorFile(MockFile & file, std::string delimiter, int linesWritten)
{
	for (int i =0 ; i < linesWritten; ++i)
	{
		file << i;
		file << delimiter;
		file << "test";
		file << delimiter;
		file << "gumbo!\n";
	}
}


TEST_CASE("Reads vector input correctly","[file]")
{
	
	MockFile F;
	int linesWritten = 100;

	SECTION("Simple delimiter")
	{
		int i = 0;
		GenerateVectorFile(F,",",linesWritten);
		forSplitLineIn(F.Path,",",[&](auto vec){
			REQUIRE(convert<int>(vec[0])==i);
			++i;
		});
	}

	SECTION("Complex delimiter")
	{
		int i = 0;
		GenerateVectorFile(F,"_-_",linesWritten);
		forSplitLineIn(F.Path,"_-_",[&](auto vec){
			REQUIRE(convert<int>(vec[0])==i);
			++i;
		});
	}

	SECTION("Tuple Reader")
	{
		int i = 0;
		GenerateVectorFile(F,",",linesWritten);
		forLineTupleIn<int,std::string,std::string>(F.Path,",",[&](auto vec){
			// REQUIRE(convert<int>(vec[0])==i);
			REQUIRE(std::get<0>(vec) == i);
			REQUIRE(std::get<1>(vec) == "test");
			REQUIRE(std::get<2>(vec) == "gumbo!");
			++i;
		});
	}

	SECTION("Vector parse")
	{
		std::string s = "0,1,2,3,4,5,6";
		auto vec = convert<std::vector<int>>(s);
		for (int i =0; i < 7; ++i)
		{
			REQUIRE(vec[i] == i);
		}
	}
	SECTION("Vector parse, different delimiters")
	{
		std::string s = "0_1_2_3_4_5_6";
		auto vec = convert<std::vector<int>>(s,"_");
		for (int i =0; i < 7; ++i)
		{
			REQUIRE(vec[i] == i);
		}
	}
}