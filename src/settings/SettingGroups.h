#pragma once

#define SETTINGS_CATEGORY SystemSettings
#define SETTINGS_FILE "definitions/system.def"
#define SETTINGS_VALIDATE
#include "SettingsConstructor.h"
void SystemSettings::Validate()
{
	if (Verbosity > 3)
	{
		Verbosity.SetValue(3,true);
	}
}

#define SETTINGS_CATEGORY StellarSettings
#define SETTINGS_FILE "definitions/stellar.def"
#include "SettingsConstructor.h"

#define SETTINGS_CATEGORY ThermalSettings
#define SETTINGS_FILE "definitions/thermal.def"
#include "SettingsConstructor.h"

#define SETTINGS_CATEGORY YieldSettings
#define SETTINGS_FILE "definitions/thermal.def"
#include "SettingsConstructor.h"

