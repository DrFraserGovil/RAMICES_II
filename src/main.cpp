#include <iostream>

#include "test/catch_amalgamated.hpp"
unsigned int Factorial( unsigned int number ) {
    return number <= 1 ? number : Factorial(number-1)*number;
}

TEST_CASE( "Factorial computes correctly", "[factorial]" ) {
    REQUIRE( Factorial(1) == 1 );
    REQUIRE( Factorial(2) == 2 );
    REQUIRE( Factorial(3) == 6 );
    REQUIRE( Factorial(10) == 3628800 );
}

bool doubleEqual(double a, double b, double threshold=1e-5)
{
	return abs(a-b) < threshold;
}

TEST_CASE( "Approximation of double-equality", "[double-equal]" ) {
    REQUIRE( doubleEqual(1,1) );
    CHECK( doubleEqual(1e100,1e100 + 0.99999e-100,1e-100));
    CHECK( !doubleEqual(1e100,1e100 + 1.0001e-100,1e-100));
    REQUIRE( !doubleEqual(1,2) );
    REQUIRE( doubleEqual(1,1+1e-3,1.1e-3));
}


