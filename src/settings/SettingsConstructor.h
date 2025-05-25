/*
	This is an X-Macro which constructs container-classes for Settings::Parameter objects
	Parameters should be define in a .def file using the following syntax: 

		Settings::Parameter<type> Name(defaultValue, triggerString)  -->   SETTING(type, Name, defaultValue, trigger)

	A SETTING_VECTOR also exists, which allows a 5th parameter:  SETTING_VECTOR(type, Name, defaultValue, trigger, vectorStringDelimiter)
	
	To construct the container, declare:

		#define SETTINGS_CATEGORY Example //this is the name of the resulting class
		#define SETTINGS_FILE example.def //this is the file containing the SETTINGs
		#include "/path/SettingsConstructor.h" //this file

	This constructs a class named `Example' with member variables equal to each of the Settings. 
	Calling .Parse() and .Configure() on this class will call them sequentially on all member variables.

	Optionally, you may declare:
		#define SETTINGS_VALIDATE
		void Example::Validate()
		{
			//your code here
		}


	This allows you to write a validation function which is called whenever the object is constructed, or when Parse() or Configure() are called. 
*/

#ifndef SETTINGS_FILE
	#error "Must define a settings file before constructing X-Macro file
#endif
#ifndef SETTINGS_CATEGORY
	#error "Must define a settings category
#endif
#include <string>
#include "Parameter.h"
class SETTINGS_CATEGORY
{
	public:
		#define SETTING(type, name, defaultValue, trigger) Settings::Parameter<type> name =  Settings::Parameter<type>(defaultValue,trigger);
		#define SETTING_VECTOR(type,name,defaultValue,trigger,delimiter)  Settings::Parameter<type> name =  Settings::Parameter<type>(defaultValue,trigger,delimiter) ;
		#include SETTINGS_FILE // Include the specific settings list
		#undef SETTING_DEF
		#undef SETTING
		#undef SETTING_VECTOR

		#ifdef SETTINGS_VALIDATE
			void Validate();
			SETTINGS_CATEGORY()
			{
				Validate();
			}
		#endif

		void Parse(int argc, char**argv)
		{
			#define SETTING(type, name, defaultValue, trigger) name.Parse(argc, argv);
			#define SETTING_VECTOR(type, name, defaultValue, trigger, delimiter) name.Parse(argc, argv);
			#include SETTINGS_FILE
			#undef SETTING
			#undef SETTING_VECTOR

			#ifdef SETTINGS_VALIDATE
				Validate();
			#endif
		}

		void Parse(const std::string & configFile, std::string delimiter)
		{
			#define SETTING(type, name, defaultValue, trigger) name.Configure(configFile,delimiter);
			#define SETTING_VECTOR(type, name, defaultValue, trigger, vecdelimiter) name.Configure(configFile,delimiter);
			#include SETTINGS_FILE
			#undef SETTING
			#undef SETTING_VECTOR
			#ifdef SETTINGS_VALIDATE
				Validate();
			#endif
		}


};

#undef SETTINGS_FILE
#undef SETTINGS_CATEGORY
#undef SETTINGS_VALIDATE