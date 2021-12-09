#pragma once

/*!
 * Defines a globally recognised ordering of the elements + provides them with a nice readable name.
 * The final entry should always be "ElementCount", as the numbering system inherent in enums makes this true only if it is the last entry!
*/
enum ElementID {Hydrogen,Helium,Metals,Iron,Oxygen,Magnesium,Carbon,Silicon,Calcium,Manganese,Chromium,Cobalt,Europium,ElementCount};

//! Defines a globally recognised set of aliases for sources of gas, used to label GasStream objects. As with ElementID, final entry is used to count the number of elements. This must always be the final entry!
enum SourceProcess {Primordial,Accreted,CCSN,SNIa, NSM, Collapsars,Unknown,ProcessCount};
