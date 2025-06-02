#pragma once
#include "HelpMessages.h"
#include "EnumSets.h"
#define SETTINGS_CATEGORY SystemSettings
#define SETTINGS_FILE "definitions/system.def"
#define SETTINGS_VALIDATE
#include "SettingsConstructor.h"

#define SETTINGS_CATEGORY StellarSettings
#define SETTINGS_FILE "definitions/stellar.def"
#include "SettingsConstructor.h"

#define SETTINGS_CATEGORY ThermalSettings
#define SETTINGS_FILE "definitions/thermal.def"
#include "SettingsConstructor.h"

#define SETTINGS_CATEGORY YieldSettings
#define SETTINGS_FILE "definitions/yield.def"
#include "SettingsConstructor.h"

#define SETTINGS_CATEGORY AbundanceSettings
#define SETTINGS_FILE "definitions/abundance.def"
#define SETTINGS_VALIDATE
#include "SettingsConstructor.h"



//this here defines the global set which will be inserted into the application settings using a familiar X-macro pattern
//why not put into its own file? a) overkill -- parameters are far more likely to change than entire settings, and b) it's very obvious when a setting is missing 
//(i.e. it won't compile, because you can't access Settings.NonExistantGroup) 
#define SETTINGS_GROUPS \
	S_GROUP(SystemSettings, System) \
	S_GROUP(StellarSettings, Stellar)\
	S_GROUP(ThermalSettings, Thermal)\
	S_GROUP(YieldSettings, Yield)\
	S_GROUP(AbundanceSettings,Abundance)\
