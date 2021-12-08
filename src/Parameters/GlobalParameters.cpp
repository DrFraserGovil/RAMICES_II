#include "GlobalParameters.h"

GlobalParameters::GlobalParameters()
{
		//anything here?
}

void GlobalParameters::Initialise(int argc, char * argv[])
{
	for (int i = 0; i < ParamMembers.size(); ++i)
	{
		ParamMembers[i]->Configure(argc,argv);
		ParamMembers[i]->Initialise(Meta.ResourceRoot);
	}
}
