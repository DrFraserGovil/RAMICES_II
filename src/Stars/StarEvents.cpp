#include "StarEvents.h"

StarEvents::StarEvents()
{
	StarMassFormed = 0;
	NStarsFormed = 0;
	CCSN = 0;
	AGBDeaths = 0;
	NSM = 0;
	SNIa = 0;
	ECSN =0;
	Efficiency = 0;
}
void StarEvents::AddHeaders(std::stringstream & output)
{
	output << "Time, Radius, StarMassFormed, FormationEfficiency, StarsFormed, CCSN_Events, AGB_Deaths, NSM_Events, SNIa_Events, ECSN_Events, ";
	output << "BirthRate, CCSNRate, AGBRate, NSMRate, SNIaRate, ECSNRate\n";
}
void StarEvents::Save(std::stringstream &output, double timestep)
{
	output << StarMassFormed << ", " << Efficiency << ", " << NStarsFormed << ", " << CCSN << ", " << AGBDeaths << ", " << NSM << ", " << SNIa << ", " << ECSN << ", ";
	output << (double)NStarsFormed/timestep << ", " << (double)CCSN/timestep << ", " << (double)AGBDeaths/timestep << ", " << (double)NSM/timestep << ", " << (double)SNIa/timestep << ", " << (double)ECSN/timestep << "\n";
}
