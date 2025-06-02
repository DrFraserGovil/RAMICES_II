
	
	const inline std::vector<std::string>& SpeciesNames(bool longNames)
		{
			static const std::vector<std::string> fullnames = []() {
				std::vector<std::string> temp_names;
				temp_names.resize(Element::Count); ///s Pre-allocate for efficiency
	

				#define Element(FullName, ShortName) temp_names[FullName] = STRINGIFY(FullName); // Define X to push string names
				#include "../settings/definitions/elements.def"                                     // Include again
				#undef Element                                                          // Undefine X
	
				return temp_names;
			}(); 

			static const std::vector<std::string> shortnames = []() {
				LOG(WARN) << "Constructing shorts!";
				std::vector<std::string> temp_names;
				temp_names.resize(Element::Count); ///s Pre-allocate for efficiency
	

				#define Element(FullName, ShortName) temp_names[FullName] = STRINGIFY(ShortName); // Define X to push string names
				#include "../settings/definitions/elements.def"                                     // Include again
				#undef Element                                                          // Undefine X
	
				return temp_names;
			}(); // Immediately invoke the lambda to initialize 'names'
	
			if (longNames) return fullnames;
			else return shortnames;
	
		};

		inline std::string Name(Element::Species s,bool useLongName = true)
		{
			if (s >= 0 && s < Element::Count)
			{
				return SpeciesNames(useLongName)[s];
			}
			throw std::out_of_range("Invalid Element::Species value.");
		}
		inline std::string Name(int s,bool useLongName = true)
		{
			return Name(static_cast<Element::Species>(s),useLongName);
		}

		// Optional: String to Enum (no change needed here as it calls GetSpeciesNames())
		inline Element::Species FromName(const std::string& name)
		{
			for (int i = 0; i < Element::Count; ++i)
			{
				if (SpeciesNames(true)[i] == name || SpeciesNames(false)[i] == name)
				{
					return static_cast<Element::Species>(i);
				}
			}
			throw std::invalid_argument("Unknown Element::Species name: " + name);
		}
