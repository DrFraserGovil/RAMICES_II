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
	
	SaveInputs();
}

void SaveInputs()
{
	std::string configOut = Output.Root.Value + "/" + Output.Config.Value;
	JSL::initialiseFile(configOut);
	
	stringstream output;
	for (int i = 0; i < ParamMembers.size(); ++i)
	{
		ParamMembers[i]->StreamContentsTo(output);
	}
	
}
