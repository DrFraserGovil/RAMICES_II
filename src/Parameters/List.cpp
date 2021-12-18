#include "List.h"


void ParamList::Configure(int argc, char * argv[])
{
	//check if using config file or command line arguments
	const std::string nullIdentifier = "__null_location__";
	Argument<std::string> ConfigFile = Argument<std::string>(nullIdentifier,"config",argc,argv);
	Argument<char> ConfigDelimiter = Argument<char>(' ',"config-delim",argc,argv);
	
	bool usingConfigFile = ((std::string)ConfigFile != nullIdentifier);
	
	for (int i = 0; i < argPointers.size(); ++i)
	{
		if (usingConfigFile)
		{
			argPointers[i]->Configure(ConfigFile,ConfigDelimiter);
		}
		else
		{
			argPointers[i]->ListParse(argc,argv);
		}
	}
}
//~ virtual void ParamList::Initialise(std::string resourceRoot)
//~ {
	
//~ };
void StreamContentsTo(stringstream & stream)
{
	for (int i =0; i < argPointers.size(); ++i)
	{
		
	}
}
