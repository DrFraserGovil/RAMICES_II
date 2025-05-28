#pragma once
#include <vector>
#include <string>
#include "../utility/convert.h"
#include "../utility/fileparser.h"
#include <cctype>
#include "../utility/MakeString.h"
extern std::vector<std::string> GlobalParameterStrings;



namespace Settings
{
	//some help metafunctions to help identify vector types
	template <typename T>
	struct is_vector : std::false_type {};
	template <typename U>
	struct is_vector<std::vector<U>> : std::true_type {};

	template<class T>
	class Parameter
	{
		public:
			T Value;
			bool hasParseDelimiter=false;
			std::string VectorParseDelimiter;
			Parameter(T defaultValue, std::string argument) : Value(defaultValue), TriggerString(argument)
			{
				ValidateTriggerString();
			}
			Parameter(T defaultValue, std::string argument,std::string vectorDelimiter) : Value(defaultValue), TriggerString(argument)
			{
				
				if constexpr (is_vector<T>::value)
				{
					hasParseDelimiter = true;
					VectorParseDelimiter = vectorDelimiter;
				}
				else
				{
					throw std::logic_error("You cannot pass a vector-delimiter to a non-vector Parameter");
				}
				ValidateTriggerString();
			}
			Parameter(T defaultValue, std::string argument,int argc, char * argv[]) : Parameter(defaultValue,argument) //calling this here means the destructor is called if Parse throws an error
			{
				Parse(argc,argv);
			}
			
			Parameter(T defaultValue, std::string argument,std::string vectorDelimiter,int argc, char * argv[]) : Parameter(defaultValue,argument,vectorDelimiter) //calling this here means the destructor is called if Parse throws an error
			{
				Parse(argc,argv);
			}

			~Parameter()
			{
				auto it = std::find(GlobalParameterStrings.begin(), GlobalParameterStrings.end(), TriggerString);
				if (it != GlobalParameterStrings.end())
				{
					GlobalParameterStrings.erase(it);
				}
				
			}

			//! Allow the Argument object to be implicitly cast into the value of #Value, and hence treated as an object of the templated type.
			operator T()
			{
				return Value;
			}
			
			//! Annoying const version
			operator T() const
			{
				return Value;
			}

			//!Iterate through a configuration file, extracting Name/Value pairs and calling Parse() in them. Each Name/Value pair should be on a new line in the file, and separated by the *configDelimiter*. \param configFile The path to the file to open and parse for configuration data \param configDelimiter The delimiter used to separate Name/Value pairs in the cofiguration file
			void Configure(std::string configFile, std::string configDelimiter)
			{
				forSplitLineIn(configFile,configDelimiter,[&](auto linevec)
				{
					if (linevec[0] == TriggerString )
					{
						size_t nItems = linevec.size();
						if (nItems == 1)
						{
							throw std::runtime_error("Configuration files do not support no-argument flags. All arguments must have an accompanying value. Flags are treated as booleans (1 or 0).");
						}
						if (nItems == 2)
						{
							Convert(linevec[1]);
						}
						else
						{
							std::string concat(linevec[1]);
							
							for (size_t i = 2; i < nItems; ++i)
							{
								concat += configDelimiter + std::string(linevec[i]);
							}
							Convert(concat);
						}
					}
				}
				);
				
			}
			
			//!Iterate through the provided commandline args, extracting Name/Value pairs and calling Parse() on them. \param argc The number of arguments passed to the program \param argv[] The argument list (argv[0] is assumed to be the the name of the program, and is ignored)
			void Parse( int argc, char * argv[])
			{
				bool foundTrigger= false;
				std::string target = "-" + TriggerString;
				for (int idx = 0; idx < argc; ++idx)
				{
					
					if (std::string(argv[idx]) == target)
					{
						foundTrigger = true;
						if (idx < argc -1 && NextElementIsValue(argv[idx+1]))
						{
							Convert(argv[idx+1]);
						}
						else
						{
							if constexpr (std::is_same_v<bool, T>)
							{
								Value = true;
							}
							else
							{
								throw std::runtime_error("The parameter " + TriggerString + " requires a value. It is either the last element in a list, or is followed by another argument");
							}
						}
					}
					if (foundTrigger)
					{
						return;
					}
				}	
				
			}

			std::string ToString()
			{
				return MakeString(Value);
			}
			std::string ToString(std::string argDelimiter)
			{
				return TriggerString + argDelimiter + MakeString(Value);
			}
			std::string ToString(std::string argDelimiter,std::string vecDelimiter)
			{
				return TriggerString + argDelimiter + MakeString(Value,vecDelimiter);
			}
		private:
			std::string TriggerString;
			
			//check no naming conflicts
			void ValidateTriggerString()
			{
				if (TriggerString == "help" || TriggerString == "h")
				{
					throw std::logic_error("'help' and 'h' are reserved strings. You cannot create a Settings::Parameter object with this trigger string");
				}

				for (std::string existingTrigger : GlobalParameterStrings)
				{
					if (TriggerString == existingTrigger)
					{
						throw std::logic_error("Settings::Parameter objects must have a unique argument identifier. '" + TriggerString + "' is already in use");
					}
				}

				//if it passed the tests, add the TriggerString to the back of the global list
				GlobalParameterStrings.push_back(TriggerString);
			}

			bool NextElementIsValue(char * nextElement)
			{
				bool hasDash = (nextElement[0] == '-'); //dashes signify commands, but also negative nos.
				bool isSingleCharacter = (strlen(nextElement) == 1);

				if (!hasDash || isSingleCharacter)
				{
					return true; //entries that are not preceeded by a dash, or are a single character long, cannot be command triggers
				}

				return isdigit(nextElement[1]); //detect if the next string is a number, if so return true. This is the case of a negative no.
				
			}

			void Convert(std::string_view sv)
			{
				if constexpr (is_vector<T>::value)
				{
					if (hasParseDelimiter)
					{
						Value = convert<T>(sv,VectorParseDelimiter);
					}
					else
					{
						Value = convert<T>(sv);
					}
				}
				else
				{
					Value = convert<T>(sv);
				}
			}
	};



};