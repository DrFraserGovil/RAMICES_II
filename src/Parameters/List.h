#pragma once
#include "JSL.h"
#include <vector>
#include <string>
#include <sstream>
using JSL::Argument;

//! A Generic superclass structure so that I can heterogenously loop over the various members of GlobalParameters without writing it all out arduously. Also provides a consistent interface with the JSL::Argument environment. 
class ParamList
{
	public:
		//! Loops over all members of the argPointers array and calls the configuration/command line API on them to initialise the members of the child classes
		void Configure(int argc, char * argv[]);
		
		//! A (hopefully) rarely used function which calls any additional functions which can only be called *after* the configuration has been run. For most members, this is an empty function.
		void virtual Initialise(std::string resourceRoot){};
		
		void StreamContentsTo(stringstream & stream);
	protected:
	
		//! A list of pointers to member variables of the child classes which want to be initialised against command line / config-file values. Any Argument objects not added to this array will not be initialised! This array should be allocated during the individual subclass constructors. 
		std::vector<JSL::ArgumentInterface *> argPointers;
		
};
