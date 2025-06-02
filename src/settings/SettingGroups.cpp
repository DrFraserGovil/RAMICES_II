#include "SettingGroups.h"


void SystemSettings::Validate()
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
}

void AbundanceSettings::Validate()
{
	// LOG()
	auto primArray = PrimordialAbundances.Value();
	primArray.resize(Element::Count,0.0); //resize to the correct size, adding in zeros and truncating
	
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
		for (auto & el : primArray)
		{
			el /= v;
		}
	}
	PrimordialAbundances.SetValue(primArray,true);
	LOG(DEBUG) << "Set primordial array to " << PrimordialAbundances.ToString();
}