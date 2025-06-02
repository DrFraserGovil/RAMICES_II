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
		
		#define S_GROUP(type,name) type name;
		SETTINGS_GROUPS
		#undef S_GROUP

		SimulationSettings()
		{
			
			Validate();
		}

		void Validate()
		{
			RegisterMemberStrings();
			
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


			#define S_GROUP(type,name) name.Validate();
			SETTINGS_GROUPS
			#undef S_GROUP
		}
		
		template<class T>
		void ToStream(T & stream)
		{
			#define S_GROUP(type,name) name.ToStream(stream,ConfigureDelimiter);
			SETTINGS_GROUPS
			#undef S_GROUP
		}
	private:
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
			#define S_GROUP(type,name) name.Parse(argc,argv);
			SETTINGS_GROUPS
			#undef S_GROUP
		}
		void ConfigureAll()
		{
			#define S_GROUP(type,name) name.Configure(ConfigureFile,ConfigureDelimiter);
			SETTINGS_GROUPS
			#undef S_GROUP
		}

		void RegisterMemberStrings()
		{
			std::unordered_map<std::string, std::string> triggersInUse;
			triggersInUse.insert({"h", "Builtin/Reserved"});
            triggersInUse.insert({"help", "Builtin/Reserved"});
            triggersInUse.insert({"config", "Builtin/Reserved"});
            triggersInUse.insert({"config-delimiter", "Builtin/Reserved"});
			

			#define S_GROUP(type,name) name.ValidateTriggers(triggersInUse);
			SETTINGS_GROUPS
			#undef S_GROUP

		}
};

extern SimulationSettings Settings;