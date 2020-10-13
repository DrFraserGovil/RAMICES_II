#include "FileHandler.h"

mkdirReturn mkdirSafely(std::string directory)
{
	mkdirReturn output;
	output.Successful = true;
	output.Message = "";
	const char * dirChar = directory.c_str();

	DIR *dir = opendir(dirChar);
	if (dir)
	{
		output.Message += "\tDirectory '" + directory + "' already exists. Request to mkdir ignored.\n";
	}
	else
	{
		output.Message += "Directory '" + directory +"'  does not exist. Constructing directory...\n";
		try
		{
			std::string command = "mkdir -p ";
			command.append(directory);
			const char *commandChar = command.c_str(); 
			const int dir_err = system(commandChar);
		}
		catch (const std::exception& e)
		{
			std::string error = "\n\n\nERROR: A problem was encountered trying to create directory. Error message is as follows\n";
			error = error + std::string(e.what());
			output.Message += error;
			output.Successful = false;
		}
	}
	
	return output;
}
