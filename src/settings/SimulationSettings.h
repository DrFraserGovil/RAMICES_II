#pragma once
#include "SettingGroups.h"
#include "../utility/Log.h"
const std::string NULLFILE= "__none__";
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
			//read these first because they tell us if a config file exists
			ConfigureFile.Parse(argc,argv);
			ConfigureDelimiter.Parse(argc,argv);

			//configure *first*
			if (ConfigureFile.Value() != NULLFILE)
			{
				ConfigureAll();
			}

			//then, we call the parsers -- allows for configs to be post-hoc modified by cmd-line calls
			ParseAll(argc,argv);


			//initialise the logging system so we can begin to communicate
			LogConfig.SetLevel(System.Verbosity);
		}
		
		template<class T>
		void ToStream(T & stream)
		{
			#define S_GROUP(type,name) name.ToStream(stream,ConfigureDelimiter);
			SETTINGS_GROUPS
			#undef S_GROUP
		}
	private:
		Settings::Parameter<std::string> ConfigureFile = Settings::Parameter<std::string>(NULLFILE,"config");
		Settings::Parameter<std::string> ConfigureDelimiter= Settings::Parameter<std::string>(" ","config-delimiter");

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