#pragma once
	
		/*! Produces a list of element names
			@details Uses static construction and some X-macro redefinition magic to atuopopulate the strings directly from elements.def
			@param longNames If true, use the 'long names' of element (i.e. Hydrogen). If false, use the 'short names' (i.e. H)
			@return A reference to a statically constructed vector of strings
		*/
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

		//! Converts integers into ::Element::Species enumerators @details Just a wrapper for a static_cast
		inline Element::Species FromInteger(int index)
		{
			return static_cast<Element::Species>(index);
		}

		/*! Converts ::Species or numeric types into their string name
			@param s The species to be stringified, can be any type which can be cast into an integer 
			@param useLongName If true, converts to the longname, otherwise uses the shortname
			@returns A string representing the (short or long) name of the element
			@throws std::out_of_range If species < 0 or >= the number of elements
		*/
		template<typename T>
		inline std::string Name(T species,bool useLongName = true)
		{
			Element::Species s = static_cast<Element::Species>(species);
			if (s >= 0 && s < Element::Count)
			{
				return SpeciesNames(useLongName)[s];
			}
			throw std::out_of_range("Invalid Element::Species value.");
		}
	

		/*! Converts strings into Elements.
			@details An element is returned if the string matches (case-insensitive) either the long or the short name of the element on record.
			@param name A string (i.e. H, Chromium, mn) to be converted into an Element
			@returns The corresponding Element::Species enumerator
			@throws std::invalid_argument If no string match is found
		*/
		inline Element::Species FromName(std::string_view name)
		{
			for (int i = 0; i < Element::Count; ++i)
			{
				if (insensitiveEquals( SpeciesNames(true)[i], name) || insensitiveEquals(SpeciesNames(false)[i], name))
				{
					return static_cast<Element::Species>(i);
				}
			}
			throw std::invalid_argument("Unknown Element::Species name: " + std::string(name));
		}
