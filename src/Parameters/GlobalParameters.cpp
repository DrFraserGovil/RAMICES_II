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
		
	}
	
	
	SaveInputs();
	
	for (int i = 0; i < ParamMembers.size(); ++i)
	{
		ParamMembers[i]->Initialise(Resources.ResourceRoot);
	}
}

void GlobalParameters::SaveInputs()
{
	
	std::string configOut = Output.Root.Value + "/" + Output.Config.Value;
	JSL::initialiseFile(configOut);
	
	std::stringstream output;
	for (int i = 0; i < ParamMembers.size(); ++i)
	{
		
		ParamMembers[i]->StreamContentsTo(output);
	}
	JSL::initialiseFile(configOut);
	JSL::writeStringToFile(configOut,output.str());
}
