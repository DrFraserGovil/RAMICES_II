#pragma once
#include <vector>
#include <string>
#include "../utility/convert.h"
#include "../utility/fileparser.h"
#include <cctype>
#include "../utility/MakeString.h"
#include <unordered_map> 



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
			
			bool hasParseDelimiter=false;
			std::string VectorParseDelimiter;
			Parameter(T defaultValue, std::string argument) : InternalValue(defaultValue), TriggerString(argument)
			{
			}
			Parameter(T defaultValue, std::string argument,std::string vectorDelimiter) : Parameter(defaultValue,argument)
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
			}
			Parameter(T defaultValue, std::string argument,int argc, char * argv[]) : Parameter(defaultValue,argument) //calling this here means the destructor is called if Parse throws an error
			{
				Parse(argc,argv);
			}
			
			Parameter(T defaultValue, std::string argument,std::string vectorDelimiter,int argc, char * argv[]) : Parameter(defaultValue,argument,vectorDelimiter) //calling this here means the destructor is called if Parse throws an error
			{
				Parse(argc,argv);
			}


			void SetValue(T value)
			{
				SetValue(value,false);
			}
			void SetValue(T value, bool confirmSafe)
			{
				if (!confirmSafe)
				{
					LOG(WARN) << "Parameter with signature -" << TriggerString << " modified by non-standard means to a value of " << ToString() << ".\n\tParameter values should normally only be modified via parsing/configuration.\n\tPlease ensure this was intended behaviour.";
				}
				InternalValue = value;

			}

			const T & Value()
			{
				return InternalValue;
			}

			//! Allow the Argument object to be implicitly cast into the value of #InternalValue, and hence treated as an object of the templated type.
			operator T()
			{
				return InternalValue;
			}
			
			//! Annoying const version
			operator T() const
			{
				return InternalValue;
			}

			//!Iterate through a configuration file, extracting Name/InternalValue pairs and calling Parse() in them. Each Name/InternalValue pair should be on a new line in the file, and separated by the *configDelimiter*. \param configFile The path to the file to open and parse for configuration data \param configDelimiter The delimiter used to separate Name/InternalValue pairs in the cofiguration file
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
			
			//!Iterate through the provided commandline args, extracting Name/InternalValue pairs and calling Parse() on them. \param argc The number of arguments passed to the program \param argv[] The argument list (argv[0] is assumed to be the the name of the program, and is ignored)
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
								InternalValue = true;
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
				return MakeString(InternalValue);
			}
			std::string ToString(std::string argDelimiter)
			{
				return TriggerString + argDelimiter + MakeString(InternalValue);
			}
			std::string ToString(std::string argDelimiter,std::string vecDelimiter)
			{
				return TriggerString + argDelimiter + MakeString(InternalValue,vecDelimiter);
			}

			//check no naming conflicts
			void ValidateTrigger(std::unordered_map<std::string, std::string> & triggerRegister,std::string parentName)
			{
				// Attempt to insert the TriggerString as the key and its parentName as the value
                // insert() returns a pair: {iterator to element, bool indicating if insertion happened}
                auto [it, inserted] = triggerRegister.insert({TriggerString, parentName});

                // If 'inserted' is false, it means TriggerString was already a key in the map
                if (!inserted)
                {
                    // The iterator 'it' now points to the existing element
                    throw std::logic_error("Settings::Parameter objects must have a unique argument identifier. '" + TriggerString + "' is already in use."
                                           " First defined in category '" + it->second + "'." // Access the value of the existing element
                                           " Found again in category '" + parentName + "'."); // Current category name
                }
                // If 'inserted' is true, the trigger was successfully added, no error thrown.
			}

			//the Trigger String is usually an invariant. It is useful when unit testing to be able to change them, however. 
			//This is not a part of the standard API, so ignore this!
			#ifdef UNITTEST 
				void SetTrigger(std::string newtrigger)
				{
					TriggerString = newtrigger;
				}
				std::string GetTrigger()
				{
					return TriggerString;
				}
			#endif
		private:
			T InternalValue;
			std::string TriggerString;
			
			

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
						InternalValue = convert<T>(sv,VectorParseDelimiter);
					}
					else
					{
						InternalValue = convert<T>(sv);
					}
				}
				else
				{
					InternalValue = convert<T>(sv);
				}
			}
	};



};