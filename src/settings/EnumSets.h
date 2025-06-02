#pragma once

/*!
 * Defines a globally recognised ordering of the elements + provides them with a nice readable name.
 * 
*/
namespace  Element
{
	enum Species {Hydrogen,Helium,Metals,Iron,Oxygen,Magnesium,Carbon,Silicon,Calcium,Manganese,Chromium,Cobalt,Europium,
		
		Count//!<The final entry should always be Count, this allows you to iterate over all previous elements
		
		};
}

//! Defines a globally recognised registry of yield processes
namespace StarDeath
{
	enum YieldProcess {CCSN, ECSN,SNIa, NSM, AGB,
		
		Count};//as with elements,

	enum RemnantType {DormantDwarf, MergerDwarf, DormantNS, MergerNS, BlackHole};
}

