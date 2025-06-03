#include "SettingGroups.h"


bool SystemSettings::Validate()
{
	if (Verbosity > 3)
	{
		Verbosity.SetValue(3,true);
	}
	//initialise the logging system so we can begin to communicate.
	//But only do this when *not* in testing mode, as testing gets messed up otherwise!
	#ifndef UNITTEST
		LogConfig.SetLevel(Verbosity);
		LogConfig.SetHeader(UseLogHeaders);
		LogConfig.SetNewline(true);
	#endif
	return true;
}

bool AbundanceSettings::Validate()
{
	
	auto primArray = PrimordialAbundances.Value();
	
	if (primArray.size() != 0)
	{
		LOG(WARN) << "We do not recommend directly modifying the PrimordialAbundance array. Please ensure you know what you are doing";
		primArray.resize(Element::Count,0.0); //resize to the correct size, adding in zeros and truncating
	}
	else
	{
		
		primArray.resize(Element::Count,0.0);
		using namespace Element;
		primArray[Hydrogen] = PrimordialHydrogen;
		primArray[Helium] = PrimordialHelium;
		primArray[Magnesium] = PrimordialMagnesium;
		primArray[Iron] = PrimordialIron;
		if (PrimordialAbundancesFile.Value() != "__none__")
		{
			forSplitLineIn(PrimordialAbundancesFile," ",[&](auto line){
				Element::Species i = Element::FromName(line[0]);
				double value = convert<double>(line[1]);
				primArray[i] = value;
			});
		}
	}

	

	double v=0;
	for (auto el : primArray)
	{
		v += el;
	}
	if (v == 0)
	{
		primArray[Element::Hydrogen] = 1.0;
	}
	else
	{
		if (v< 1.0)
		{
			primArray[Element::UnspecifiedMetal] = 1.0 - v;
		}
		else
		{
			for (auto & el : primArray)
			{
				el /= v;
			}
		}
	}
	PrimordialAbundances.SetValue(primArray,true);
	LOG(DEBUG) << "Set primordial array to " << PrimordialAbundances.ToString();
	return true;
}