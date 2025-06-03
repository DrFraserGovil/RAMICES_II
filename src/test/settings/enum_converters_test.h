#pragma once
#include "../catch_amalgamated.hpp" 
#include "../../settings/EnumSets.h"


TEST_CASE("Basic int-enum casting","[enum][elements]")
{
	REQUIRE(Element::FromInteger(Element::Hydrogen) == Element::Hydrogen);
	REQUIRE(Element::FromInteger(Element::Iron) == Element::Iron);
	REQUIRE(Element::FromInteger(Element::UnspecifiedMetal) == Element::UnspecifiedMetal);


}

TEST_CASE("Element-name translators","[enum][elements]")
{
	SECTION("Element to Full names")
	{
		REQUIRE(Element::Name(Element::Hydrogen) == "Hydrogen");
		REQUIRE_FALSE(Element::Name(Element::Iron) == "Magnesium");
		REQUIRE(Element::Name(Element::Cobalt) == "Cobalt");

		
	}

	SECTION("Element to Short names")
	{
		REQUIRE(Element::Name(Element::Helium,false) == "He");
		REQUIRE(Element::Name(Element::Chromium,false) == "Cr");
		REQUIRE(Element::Name(Element::Carbon,false) == "C");
		REQUIRE(Element::Name(Element::UnspecifiedMetal,false) == "Zu");
	}

	

	SECTION("Names to Element")
	{
		REQUIRE(Element::FromName("Hydrogen") == Element::Hydrogen);
		REQUIRE(Element::FromName("H") == Element::Hydrogen);
		REQUIRE(Element::FromName("Mn") == Element::Manganese);
		REQUIRE(Element::FromName("Magnesium") == Element::Magnesium);
		REQUIRE(Element::FromName("silicon") == Element::Silicon);	//note case insensitive
		REQUIRE(Element::FromName("eu") == Element::Europium);
	}
	
}

TEST_CASE("Error throwing","[enum][elements][errors]")
{
	REQUIRE_THROWS_AS(Element::Name(Element::Count), std::out_of_range); 
	REQUIRE_THROWS_AS(Element::Name(Element::FromInteger(-1)), std::out_of_range);

	REQUIRE_THROWS_AS(Element::FromName("NotAnElement"), std::invalid_argument);
	REQUIRE_THROWS_AS(Element::FromName(""), std::invalid_argument); // Empty string
}