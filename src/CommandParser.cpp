#include "CommandParser.h"

Options opts = Options();
std::vector<std::string> prelog; //log is not initialised until after command parser has processed - store any messagtes here, to be passed on later

std::vector<std::string> integerGlobalTriggers = {};
std::vector<int *> integerGlobalPointers = {};


std::vector<std::string> doubleGlobalTriggers =  {};
std::vector<double *> doubleGlobalPointers = { };


//fractions are doubles that are constrained to be between zero and 1
std::vector<std::string> fractionGlobalTriggers = {}; 
std::vector<double *> fractionGlobalPointers= {};


//fraction pairs are pairs of doubles between 0 and 1, which sum to 1. Therefore two variables need to be passed, formatted as a std::vector.
std::vector<std::string> fractionPairGlobalTriggers = {};
std::vector<std::vector<double *>> fractionPairGlobalPointers= {};//{ {&SNIafrac, &nonSNIafrac}, {&NMfrac, &nonNMfrac}};


std::vector<std::string> booleanGlobalTriggers = {};
std::vector<bool *> booleanGlobalPointers = {};


//special triggers are those which need a dedicated function to do their job, such as -dir, which needs to create directories etc. 
//special functions are forward-declared in the .h file. 
std::vector<std::string> specialGlobalTriggers= {"-h", "-dir"};
parseFunctions specialFuncs[] = {help, changeFileRoot};


bool changeIntegerParameter(char * newValue, int * host, char * callName)
{
	try
	{
		double convertedVal = std::stoi(newValue);
		host[0] = convertedVal;
		prelog.push_back( "Changing the " + std::string(callName) + " value. Value is now: "  + std::to_string(convertedVal));
		return true;
	}
	catch (const std::exception& e)
	{
		std::string error = "\n\n\nERROR: A problem was encountered trying to parse" + std::string(callName) + ". Error message is as follows \n";
		error = error + std::string(e.what() );
		
		prelog.push_back(error);
		return false;
	}
}

bool changeDoubleParameter(char * newValue, double * host, char * callName)
{
	try
	{
		double convertedVal = std::stod(newValue);
		host[0] = convertedVal;
		prelog.push_back( "Changing the " + std::string(callName) + " value. Value is now: "  + std::to_string(convertedVal));
		return true;
	}
	catch (const std::exception& e)
	{
		std::string error = "\n\n\nERROR: A problem was encountered trying to parse" + std::string(callName) + ". Error message is as follows \n";
		error = error + std::string(e.what() );
		
		prelog.push_back(error);
		return false;
	}
}

bool changeFractionParameter(char * newValue, double * host, char * callName)
{
	try
	{
		double convertedVal = std::stod(newValue);
		
		if (convertedVal < 0 || convertedVal > 1)
		{
			prelog.push_back("\n\nERROR: The value passed to " + std::string(callName) + " was not a valid fraction." );
			return false;
		}
		
		host[0] = convertedVal;
		prelog.push_back("Changing the " +std::string(callName) + " value. Value is now: " + std::to_string(convertedVal) );
		return true;
	}
	catch (const std::exception& e)
	{
		std::string error = "\n\n\nERROR: A problem was encountered trying to parse" + std::string(callName) + ". Error message is as follows \n";
		error = error + std::string(e.what() );
		
		prelog.push_back(error);
		return false;
	}
}

bool changeFractionPairParameter(char * newValue, std::vector<double *> host, char * callName)
{
	try
	{
		double convertedVal = std::stod(newValue);
		if (convertedVal < 0 || convertedVal > 1)
		{
			prelog.push_back("\n\nERROR: The value passed to " + std::string(callName) + " was not a valid fraction." );
			return false;
		}
		
		host[0][0] = convertedVal;
		host[1][0] = 1.0 - convertedVal;
		prelog.push_back("Changing the " +std::string(callName) + " value. Value is now: " + std::to_string(convertedVal) );
		return true;
	}
	catch (const std::exception& e)
	{
		std::string error = "\n\n\nERROR: A problem was encountered trying to parse" + std::string(callName) + ". Error message is as follows \n";
		error = error + std::string(e.what() );
		
		prelog.push_back(error);
		return false;
	}
}

bool changeBooleanParameter(char * newValue, bool * host, char * callName)
{
	try
	{
		bool convertedVal = (bool)std::stoi(newValue);
		host[0] = convertedVal;
		prelog.push_back("Changing the " +std::string(callName) + " value. Value is now: " + std::to_string(convertedVal) );
		return true;
	}
	catch (const std::exception& e)
	{
		std::string error = "\n\n\nERROR: A problem was encountered trying to parse" + std::string(callName) + ". Error message is as follows \n";
		error = error + std::string(e.what() );
		
		prelog.push_back(error);
		return false;
	}
}

bool help(char* arg)
{
	std::ifstream helpFile("DataFiles/helpList.dat");
	if (!helpFile.is_open())
	{
		prelog.push_back("\n\nERROR: Could not find the help files. Something terrible has occured.\n\n");
		return false;
	}
	
	std::string rawLine;
	while (getline(helpFile, rawLine) )
	{
		std::vector<std::string> line = split(rawLine,'\t');
		std::ostringstream helptext;
		for (int i =0; i < line.size(); ++i)
		{
			
			helptext << "\t" << std::setw(15) << std::left << line[i];
		}
		
		prelog.push_back(helptext.str());
	}
	return true;
}

bool changeFileRoot(char* arg)
{

	std::string FILEROOT = (std::string)arg;
	opts.Simulation.FileRoot = FILEROOT;
	int n = FILEROOT.size();
	std::string lastCharacter =FILEROOT.substr(n -1);
	bool needsSlash = (lastCharacter.compare("/"));
	prelog.push_back("Saving Files to directory '" + FILEROOT + "'. ");
	const char *fileChar = FILEROOT.c_str();
	DIR *dir = opendir(fileChar);
	if (dir)
	{
		prelog.push_back("\tDir exists - all is good");
		if (needsSlash)
		{
			FILEROOT.append("/");
		}	
	}
	else
	{
		prelog.push_back("Dir does not exist. Constructing directory...");
		try
		{
			std::string command = "mkdir -p ";
			command.append(FILEROOT);
			const char *commandChar = command.c_str(); 
			const int dir_err = system(commandChar);
			if (needsSlash)
			{
				FILEROOT.append("/");
			}	
		}
		catch (const std::exception& e)
		{
			std::string error = "\n\n\nERROR: A problem was encountered trying to create directory. Error message is as follows\n";
			error = error + std::string(e.what());
			prelog.push_back(error);
			return false;
		}
	}
	return true;
}


bool helpNeeded(int argc, char** args)
{
	for (int i = 1; i < argc;++i)
	{
		std::string temp = args[i];
		if (temp.compare("-h")==0)
		{
			return true;
		}
	}
	return false;
}
ParserOutput parseCommandLine(int argc, char** argv)
{
	//parse command line arguments
	bool linesParsed = true;
	
	
	if (helpNeeded(argc,argv))
	{
		help(argv[0]);
		exit(0);
	}
	
	
	if (argc > 1)
	{
		prelog.push_back("Processing " + std::to_string(argc - 1) + " command line arguments");
		for (int i = 1; i < argc-1; i +=2)
		{
			std::string temp = argv[i];
			bool found = false;
			std::vector<std::vector<std::string>> triggers = {specialGlobalTriggers, integerGlobalTriggers, doubleGlobalTriggers, booleanGlobalTriggers, fractionGlobalTriggers, fractionPairGlobalTriggers};
			
			//loop through the standard assignment std::vectors
			for (int k = 0; k < triggers.size(); ++k)
			{
				int N = triggers[k].size();
				for (int p = 0; p < N; ++p)
				{
					if (temp.compare(triggers[k][p]) == 0)
					{
						//although the function calls here look identical (so could be compacted into a looped-array over a std::vector?), note that
						//the std::vector-of-pointer-arguments contain different data types, so you'd need to construct some kind of wrapper to manage it. Too much effort. Switch/case does the job.
						found = true;
						switch(k)
						{
							case 0:
								linesParsed = specialFuncs[p](argv[i+1]);	
								break;
							case 1:
								linesParsed = changeIntegerParameter(argv[i+1], integerGlobalPointers[p], argv[i]);
								break;
							case 2:
								linesParsed = changeDoubleParameter(argv[i+1], doubleGlobalPointers[p],argv[i]);
								break;
							case 3:
								linesParsed = changeBooleanParameter(argv[i+1], booleanGlobalPointers[p],argv[i]);
								break;
							case 4:
								linesParsed = changeFractionParameter(argv[i+1], fractionGlobalPointers[p], argv[i]);
								break;
							case 5:
								linesParsed = changeFractionPairParameter(argv[i+1], fractionPairGlobalPointers[p], argv[i]);
								break;
							default:
								linesParsed = false;
								found = false;
								prelog.push_back("Something went wrong in the default parameter assignment routine");
								i = argc;
								break;
						}							
						p = triggers[k].size(); // might as well jump to end of loop. Not performance critical, however. 
						k = triggers.size();
					}
				}				
			}
			
			if (!found)
			{
				prelog.push_back("\n\n ERROR: An unknown command (" +temp + ") was passed to the CLP. Quitting.");
				i = argc;
				linesParsed = false;
			}
			
		}
	}
	
	if (argc % 2 == 0 && linesParsed)
	{
		prelog.push_back("\nWARNING: An uneven number of parameters was passed, so the final command (" + std::string(argv[argc-1]) + ") was not examined. \nThis is a non-critical error, so code will continue.\n\n" ); 
	}
	
	ParserOutput output;
	output.options = opts;
	output.SuccessfulParse = linesParsed;
	output.PreLog = prelog;
	
	return output;
}

