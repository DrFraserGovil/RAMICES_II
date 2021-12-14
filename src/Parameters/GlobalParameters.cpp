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

void GlobalParameters::Log(const std::string & input) const
{
	Log(input,1);
}
void GlobalParameters::Log(const std::string & input, int importance) const
{
	if (Meta.Verbosity > 0 && importance <= Meta.Verbosity)
	{
		std::cout << input;
	}
}
void GlobalParameters::LogFlush() const
{
	std::cout << std::flush;
}
void GlobalParameters::UrgentLog(const std::string & input) const
{
	Log(input,1);
	LogFlush();
}
