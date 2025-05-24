#pragma once
#include "../catch_amalgamated.hpp" 
#include <string>
#include <string_view>
#include "../../utility/fileparser.h"
#include "coutCatch.h"

TEST_CASE("Basic conversion","[utility][convert]")
{

	SECTION("Check errors")
	{
		std::string errorMessage = capture_stdout([&](){
			REQUIRE_THROWS(convert<int>(""));
		});
		REQUIRE_THAT(errorMessage,ContainsSubstring("ERROR"));
		REQUIRE_THAT(errorMessage,ContainsSubstring("empty"));

		errorMessage = capture_stdout([&](){
			REQUIRE_THROWS(convert<int>("1.5"));
		});
		REQUIRE_THAT(errorMessage,ContainsSubstring("ERROR"));
		REQUIRE_THAT(errorMessage,ContainsSubstring("Partial conversion"));


		errorMessage = capture_stdout([&](){
			REQUIRE_THROWS(convert<int>("99999999999"));
		});
		REQUIRE_THAT(errorMessage,ContainsSubstring("ERROR"));
		REQUIRE_THAT(errorMessage,ContainsSubstring("out of range"));


		errorMessage = capture_stdout([&](){
			REQUIRE_THROWS(convert<int>("true"));
		});
		REQUIRE_THAT(errorMessage,ContainsSubstring("ERROR"));
		REQUIRE_THAT(errorMessage,ContainsSubstring("Invalid argument"));

		errorMessage = capture_stdout([&](){
			REQUIRE_THROWS(convert<int>("-"));
		});
		REQUIRE_THAT(errorMessage,ContainsSubstring("ERROR"));
		REQUIRE_THAT(errorMessage,ContainsSubstring("Invalid argument"));

		
		errorMessage = capture_stdout([&](){
			REQUIRE_NOTHROW(convert<std::vector<double>>("(1,2,3,4"));
		});
		REQUIRE_THAT(errorMessage,ContainsSubstring("WARN"));
	}

	SECTION("Consistent whitespace trimming")
	{
		REQUIRE_NOTHROW(convert<int>("1 "));
		REQUIRE_NOTHROW(convert<bool>("1 "));
		REQUIRE_NOTHROW(convert<double>("1 "));
		REQUIRE_NOTHROW(convert<std::vector<int>>("1 "));

		//check that string does *not* trim!s
		REQUIRE(convert<std::string>(" hi") == " hi");
	}

	SECTION("Integral types")
	{
		// Basic Valid Cases
        REQUIRE(convert<int>("1") == 1);
        REQUIRE(convert<int>("-1") == -1);
        REQUIRE(convert<int>("101") == 101);
        REQUIRE(convert<unsigned int>("123") == 123u);
        REQUIRE(convert<long>("12345678901") == 12345678901L);
        REQUIRE(convert<long long>("999999999991") == 999999999991LL);
	}

	SECTION("doubles")
	{
		REQUIRE(convert<double>("15") == 15.0);
        REQUIRE(convert<double>("1.5") == 1.5);
        REQUIRE(convert<double>("-1.5") == -1.5);
        REQUIRE(convert<float>("3.14") == 3.14f);
        REQUIRE(convert<double>("0.0") == 0.0);
        REQUIRE(convert<double>(".5") == 0.5);
        REQUIRE(convert<double>("1.") == 1.0);

		 // Scientific Notation
		 REQUIRE(convert<double>("1e3") == 1000.0);
		 REQUIRE(convert<double>("1.23e-5") == 0.0000123);
		 REQUIRE(convert<double>("-2.5E+2") == -250.0); // Test uppercase E
 
		 // Special Values (std::stod/stof handle these)
		 REQUIRE(std::isinf(convert<double>("inf")));
		 REQUIRE(std::isinf(convert<double>("-inf")));
		 REQUIRE(std::isnan(convert<double>("nan")));
		 REQUIRE(std::isinf(convert<float>("INF"))); // std::stod/stof are case-insensitive for inf/nan
 
	}

	SECTION("Booleans")
	{
		REQUIRE(convert<bool>("1") == true );
		REQUIRE(convert<bool>("TRUE") == true );
		REQUIRE(convert<bool>("TRUE") == true );
		REQUIRE(convert<bool>("FAlse") == false );
		REQUIRE(convert<bool>("0") == false );
		auto errorMessage = capture_stdout([&](){
			REQUIRE_THROWS(convert<bool>("2"));
		});
		REQUIRE_THAT(errorMessage,ContainsSubstring("ERROR"));
	}

	SECTION("Vector -ints")
	{
		// Basic Valid Cases
        REQUIRE(convert<std::vector<int>>("1,2,3") == std::vector<int>{1, 2, 3});
        REQUIRE(convert<std::vector<int>>("[1,2,3]") == std::vector<int>{1, 2, 3});
        REQUIRE(convert<std::vector<int>>("{1,2,3}") == std::vector<int>{1, 2, 3});
        REQUIRE(convert<std::vector<int>>("(1,2,3)") == std::vector<int>{1, 2, 3});

        // With custom delimiter
        REQUIRE(convert<std::vector<int>>("1;2;3", ";") == std::vector<int>{1, 2, 3});
        REQUIRE(convert<std::vector<int>>("[1;2;3]", ";") == std::vector<int>{1, 2, 3});

        // With whitespace in elements (requires `trim` in vector loop for non-floating types)
        REQUIRE(convert<std::vector<int>>(" 1 , 2 , 3 ") == std::vector<int>{1, 2, 3});
        REQUIRE(convert<std::vector<int>>("[ 1 , 2 , 3 ]") == std::vector<int>{1, 2, 3});
        REQUIRE(convert<std::vector<int>>(" 1 ; 2 ; 3 ", ";") == std::vector<int>{1, 2, 3});

        // Empty vector representations
        REQUIRE(convert<std::vector<int>>("[]") == std::vector<int>{});
        REQUIRE(convert<std::vector<int>>("{}") == std::vector<int>{});
        REQUIRE(convert<std::vector<int>>("()") == std::vector<int>{});

		// Error Cases
		std::string errorMessage;

		// Empty input string to vector (throws due to vector specialization's RejectEmpty)
		errorMessage = capture_stdout([&]() {
			REQUIRE_THROWS_AS(convert<std::vector<int>>(""), std::logic_error);
		});
		REQUIRE_THAT(errorMessage, ContainsSubstring("ERROR"));
		REQUIRE_THAT(errorMessage, ContainsSubstring("empty string"));

		// Malformed elements (inner conversion failure)
		errorMessage = capture_stdout([&]() {
			REQUIRE_THROWS_AS(convert<std::vector<int>>("1,abc,3"), std::logic_error);
		});
		REQUIRE_THAT(errorMessage, ContainsSubstring("ERROR"));
		REQUIRE_THAT(errorMessage, ContainsSubstring("Invalid argument"));

		// Empty elements (e.g. "1,,3")
		errorMessage = capture_stdout([&]() {
			REQUIRE_THROWS_AS(convert<std::vector<int>>("1,,3"), std::logic_error);
		});
		REQUIRE_THAT(errorMessage, ContainsSubstring("ERROR"));
		REQUIRE_THAT(errorMessage, ContainsSubstring("empty")); // From inner int conversion's RejectEmpty
	}

	SECTION("Vector Conversions - Doubles")
	{
		// Basic Valid Cases
		REQUIRE(convert<std::vector<double>>("1.1,2.2,3.3") == std::vector<double>{1.1, 2.2, 3.3});
		REQUIRE(convert<std::vector<double>>("[1.1,2.2,3.3]") == std::vector<double>{1.1, 2.2, 3.3});

		// With whitespace in elements (should work fine due to std::stod skipping whitespace AND trim)
		REQUIRE(convert<std::vector<double>>(" 1.1 , 2.2 , 3.3 ") == std::vector<double>{1.1, 2.2, 3.3});

		// Error Cases
		std::string errorMessage;

		// Malformed elements
		errorMessage = capture_stdout([&]() {
			REQUIRE_THROWS_AS(convert<std::vector<double>>("1.0,invalid,3.0"), std::logic_error);
		});
		REQUIRE_THAT(errorMessage, ContainsSubstring("ERROR"));
		REQUIRE_THAT(errorMessage, ContainsSubstring("Invalid argument"));

		// Empty elements
		errorMessage = capture_stdout([&]() {
			REQUIRE_THROWS_AS(convert<std::vector<double>>("1.0,,3.0"), std::logic_error);
		});
		REQUIRE_THAT(errorMessage, ContainsSubstring("ERROR"));
		REQUIRE_THAT(errorMessage, ContainsSubstring("empty"));
		}

		SECTION("Vector Conversions - Strings")
		{
		// Basic Valid Cases
		REQUIRE(convert<std::vector<std::string>>("a,b,c") == std::vector<std::string>{"a", "b", "c"});
		REQUIRE(convert<std::vector<std::string>>("[hello,world]") == std::vector<std::string>{"hello", "world"});

		// Empty string elements (valid for std::string type)
		

		// Empty input string to vector (throws due to vector specialization's RejectEmpty)
		std::string errorMessage = capture_stdout([&]() {
			REQUIRE_THROWS_AS(convert<std::vector<std::string>>("a,,c"),std::logic_error);
			REQUIRE_THROWS_AS(convert<std::vector<std::string>>(",a,"),std::logic_error);
			REQUIRE_THROWS_AS(convert<std::vector<std::string>>(""), std::logic_error);
		});
		REQUIRE_THAT(errorMessage, ContainsSubstring("ERROR"));
		REQUIRE_THAT(errorMessage, ContainsSubstring("empty string"));


		// Empty bracket representation
		REQUIRE(convert<std::vector<std::string>>("[]") == std::vector<std::string>{});
	}
	


	
}