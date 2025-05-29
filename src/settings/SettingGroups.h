#pragma once

#define SETTINGS_CATEGORY SystemSettings
#define SETTINGS_FILE "settings_system.def"
#define SETTINGS_VALIDATE
#include "SettingsConstructor.h"
void SystemSettings::Validate()
{
	if (Verbosity > 3)
	{
		Verbosity.Value = 3;
	}
}

#define SETTINGS_CATEGORY StellarSettings
#define SETTINGS_FILE "settings_stellar.def"
#include "SettingsConstructor.h"

#define SETTINGS_CATEGORY ThermalSettings
#define SETTINGS_FILE "settings_thermal.def"
#include "SettingsConstructor.h"

#define SETTINGS_CATEGORY YieldSettings
#define SETTINGS_FILE "settings_thermal.def"
#include "SettingsConstructor.h"

