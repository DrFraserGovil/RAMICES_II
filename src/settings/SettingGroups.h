#pragma once

#define SETTINGS_CATEGORY System
#define SETTINGS_FILE "settings_system.def"
#define SETTINGS_VALIDATE
#include "SettingsConstructor.h"
void System::Validate()
{
	if (Verbosity > 3)
	{
		Verbosity.Value = 3;
	}
}