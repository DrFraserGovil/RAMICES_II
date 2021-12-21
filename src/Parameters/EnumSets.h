#pragma once

/*!
 * Defines a globally recognised ordering of the elements + provides them with a nice readable name.
 * 
*/
enum ElementID {Hydrogen,Helium,Metals,Iron,Oxygen,Magnesium,Carbon,Silicon,Calcium,Manganese,Chromium,Cobalt,Europium,
	
	ElementCount//!<The final entry should always be ElementCount, as the numbering system inherent in enums makes this true only if it is the last entry!
	
	};

//!Defines a globally recognised set of aliases for sources of gas, used to label GasStream objects. 
enum SourceProcess {Accreted, Stellar, Remnant,
	
	ProcessCount//!<As with ElementID, final entry is used to count the number of elements. This must always be the final entry
	
	};
	
enum YieldProcess {CCSN, SNIa, NSM, AGB, YieldCount};

enum RemnantType {WhiteDwarf, DormantDwarf, SNIaDwarf, NeutronStar, DormantNS, MergerNS, BlackHole};

