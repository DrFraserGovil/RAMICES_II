#pragma once
#include <string>
#include "SettingGroups.h"
#include "../utility/Log.h"
#include "HelpMessages.h"
#include "EnumSets.h" //included here as a sneaky way to include the enums everywhere that settings are included
const std::string NULLFILE= "__none__";


//This is a manager object for a number of Settings. 
//It is populated by the Settings_Groups defined in SettingGroups.h, and uses some X-macro techniques to ensure that it auto-populates itself.

class SimulationSettings
{
	public:
		
		SETTINGS_GROUPS

		int UnusuedVariableForCheckingDoxygen = 5;

		//!Default constructor
		SimulationSettings()
		{
			Register();
		}

		
		void Initialise(int argc, char**argv)
		{
			SpecialCommandParsers(argc,argv);

			//configure *first*
			if (ConfigureFile.Value() != NULLFILE)
			{
				ConfigureAll();
			}

			//then, we call the parsers -- allows for configs to be post-hoc modified by cmd-line calls
			ParseAll(argc,argv);
			ValidateAll();
		}
		
		
		template<class T>
		void ToStream(T & stream)
		{
			#define S_GROUP(type,name) name.ToStream(stream,ConfigureDelimiter);
			SETTINGS_GROUPS
			#undef S_GROUP
		}

		

		void Register()
		{
			RegisterMemberStrings();	
		}
	private:
		std::unordered_map<std::string, std::string> TriggerRegister;
		Setting::Parameter<std::string> ConfigureFile = Setting::Parameter<std::string>(NULLFILE,"config");
		Setting::Parameter<std::string> ConfigureDelimiter= Setting::Parameter<std::string>(" ","config-delimiter");

		Setting::Parameter<bool> QuickHelp = Setting::Parameter<bool>(false,"h");
		Setting::Parameter<bool> Help = Setting::Parameter<bool>(false,"help"); //need both because I'm limited to one-trigger-per parameter!

		


		void SpecialCommandParsers(int argc,char**argv)
		{
			//read these first because they tell us if a config file exists
			ConfigureFile.Parse(argc,argv);
			ConfigureDelimiter.Parse(argc,argv);
			QuickHelp.Parse(argc,argv);
			Help.Parse(argc,argv);

			//!now the most special of all
			if (QuickHelp.Value() || Help.Value())
			{
				HelpMessage();
			}
		}

		void HelpMessage()
		{
			std::cout << "\nRAMICES III Help Page\n\n\tUsage: ramices [options]\n\n";
			
			HelpMessages SpecialMessage;
			SpecialMessage.Name = "SpecialSettings";
			SpecialMessage.AddMessage("configure",std::string("__none__"),"ConfigureFile","When not equal to '__none__', the system will attempt to read this file in as a configuration file.\nConfiguration files work the same as command line arguments, each line should contain a singleflag and a value\nIMPORTANT: Flags in config files omit the '-'");
			SpecialMessage.AddMessage("configure-delimiter",std::string(" "),"ConfigureDelimiter","The string which separates the flag from the values in the config file.\nOnly the first instance of the flag is counted, subsequent occurrences are ignored.");
			SpecialMessage.AddMessage("h,  -help",false,"Help","When true, activates the help page, then exits");

			std::vector<HelpMessages> Messages({SpecialMessage});

			
			
			#define S_GROUP(type,name) Messages.push_back(name.CryForHelp());
			SETTINGS_GROUPS
			#undef S_GROUP
			
			std::pair<int,int> sizeBuffer({0,0});
			for (auto message : Messages)
			{
				message.scanSizes(sizeBuffer);
			}
			for (auto message : Messages)
			{
				message.print(sizeBuffer);
			}

			exit(1);
		}

		void ParseAll(int argc, char**argv)
		{
			ParseCheck(argc,argv);
			#define S_GROUP(type,name) name.Parse(argc,argv);
			SETTINGS_GROUPS
			#undef S_GROUP
		}
		void ConfigureAll()
		{
			ConfigCheck(ConfigureFile,ConfigureDelimiter);
			#define S_GROUP(type,name) name.Configure(ConfigureFile,ConfigureDelimiter);
			SETTINGS_GROUPS
			#undef S_GROUP
		}

		void ValidateAll(bool force=true)
		{
			#define S_GROUP(type,name) name.Validate();
			SETTINGS_GROUPS
			#undef S_GROUP
		}

		void RegisterMemberStrings()
		{
			TriggerRegister.insert({"h", "Builtin/Reserved"});
            TriggerRegister.insert({"help", "Builtin/Reserved"});
            TriggerRegister.insert({"config", "Builtin/Reserved"});
            TriggerRegister.insert({"config-delimiter", "Builtin/Reserved"});
			

			#define S_GROUP(type,name) name.ValidateTriggers(TriggerRegister);
			SETTINGS_GROUPS
			#undef S_GROUP

		}

		void ParseCheck(int argc, char ** argv)
		{
			std::vector<std::pair<std::string,int>> foundStrings;
			bool prevWasValue = false;
			bool prevMalformed = false;
			for (int i = 0; i < argc; ++i)
			{
				if (Setting::ElementIsValue(argv[i]))
				{
					if (prevWasValue && !prevMalformed)
					{
						LOG(WARN) << "Successive value-elements encountered in cmd-parse (" << argv[i-1] << " & " << argv[i] << ") this likely arises due to a malformed command.\n\tYou probably missed a '-'.";
						prevMalformed = true;
					}
					else
					{
						prevMalformed = false;
					}
					prevWasValue = true;
				}
				else
				{
					prevWasValue = false;
				}
				std::string cmd = argv[i];

				if (cmd.size() > 1 && cmd[0] == '-')
				{
					for (int j = 0; j < foundStrings.size(); ++j)
					{
						if (cmd == foundStrings[j].first)
						{
							LOG(ERROR) << "The command string '" << cmd << "' has been encountered twice in the same cmd-parse; at position " << foundStrings[j].second << " and " << i;
							throw std::runtime_error("Multiply defined parse arguments");							
						}
					}
					foundStrings.push_back({cmd,i});
				}
			}
			CheckUnusedStrings(foundStrings,"-","Command line position");
		}

		void ConfigCheck(std::string configFile, std::string configDelim)
		{
			std::vector<std::pair<std::string,int>> foundStrings;
			int fileLine = 0;
			forSplitLineIn(configFile,configDelim,[&](auto splitLine){
				if (splitLine.size() > 0)
				{
					auto cmd = splitLine[0];
					if (cmd.find("//") == std::string_view::npos && cmd.size() > 0)
					{
						for (int j = 0; j < foundStrings.size(); ++j)
						{
							if (cmd == foundStrings[j].first)
							{
								LOG(ERROR) << "The command string " << cmd << " has been encountered twice in the same cmd-config; on lines " << foundStrings[j].second << " and " << fileLine+1;
								throw std::runtime_error("Multiply defined parse arguments");							
							}
						}
						foundStrings.push_back({std::string(cmd),fileLine+1});
					}
				}
				++fileLine;

			});
			CheckUnusedStrings(foundStrings,"","Config file line");
		}

		void CheckUnusedStrings(const std::vector<std::pair<std::string,int>> & foundStrings,const std::string prefix,const std::string & locatorName)
		{
			for (auto cmd : foundStrings)
			{
				std::string_view cmdName = cmd.first;
				auto cmdSansSuffix = std::string(cmdName.substr(prefix.size(),cmdName.size()-prefix.size()));
				if (TriggerRegister.find(cmdSansSuffix) == TriggerRegister.end())
				{
					LOG(WARN) << "The command '" << cmd.first << "' (" << locatorName << " " << cmd.second <<") does not match any active Parameter objects.\n\tIt will be ignored.";
				}
			}
		}
};

extern SimulationSettings Settings;

#define S_GROUP(type,name) type name; //define here for the documentation preprocessor....