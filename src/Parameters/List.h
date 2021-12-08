#pragma once
#include "JSL.h"
#include <vector>
#include <string>

using JSL::Argument;

//! generic superclass structure so that I can heterogenously loop over arrays
class ParamList
{
	public:
		void Configure(int argc, char * argv[]);
		void virtual Initialise(std::string resourceRoot){};
	protected:
		std::vector<JSL::ArgumentInterface *> argPointers;
		
};
