#pragma once

#include <vector>
#include <string>

/*!
 * Defines a globally recognised ordering of the elements + provides them with a nice readable name.
 * 
*/
//allows conversion of the macro into a string with a layer of indirection
#ifndef STRINGIFY
	#define STRINGIFY0(v) #v
	#define STRINGIFY(v) STRINGIFY0(v)
#endif


namespace  Element
{



	enum Species {

		#define Element(name,shortname) name,
		#include "definitions/elements.def"
		#undef Element

		Count//!<The final entry should always be Count, this allows you to iterate over all previous elements
		};


	#include "../utility/ElementConverters.h"

}

//! Defines a globally recognised registry of yield processes
namespace StarDeath
{
	enum YieldProcess {CCSN, ECSN,SNIa, NSM, AGB,
		
		Count};//as with elements,

	enum RemnantType {DormantDwarf, MergerDwarf, DormantNS, MergerNS, BlackHole};
}

