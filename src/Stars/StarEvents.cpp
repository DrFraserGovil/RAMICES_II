#include "StarEvents.h"

StarEvents::StarEvents()
{
	StarMassFormed = 0;
	NStarsFormed = 0;
	CCSN = 0;
	AGBDeaths = 0;
	NSM = 0;
	SNIa = 0;
}
void StarEvents::AddHeaders(std::stringstream & output)
{
	output << "Time, Radius, StarMassFormed, StarsFormed, CCSN_Events, AGB_Deaths, NSM_Events, SNIa_Events, ";
	output << "BirthRate, CCSNRate, AGBRate, NSMRate, SNIaRate\n";
}
void StarEvents::Save(std::stringstream &output, double timestep)
{
	output << StarMassFormed << ", " << NStarsFormed << ", " << CCSN << ", " << AGBDeaths << ", " << NSM << ", " << SNIa << ", ";
	output << (double)NStarsFormed/timestep << ", " << (double)CCSN/timestep << ", " << (double)AGBDeaths/timestep << ", " << (double)NSM/timestep << ", " << (double)SNIa/timestep << "\n";
}
