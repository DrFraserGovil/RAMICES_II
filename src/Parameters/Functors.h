#pragma once
#include "../Stars/IMF.h"
#include "../Stars/SLF.h"
#include "GlobalParameters.h"
//! These will act like globally-defined functions, but have the scope for modifying themselves as they go along
class Functors
{
	public:
		const IMF_Functor IMF;
		SLF_Functor SLF;
		
		Functors(const GlobalParameters & Param);
	
	private:
		const GlobalParameters & Param;
	
	
};
