#include "Star.h"

StarSet::StarSet()
{
	Mass = 0;
	NStars = 0;
	Metallicity = 0;
};

StarSet::StarSet(Options * opts)
{
	Opts = opts;
	Mass = 0;
	NStars = 0;
}
StarSet::StarSet(Options * opts, double  mass, double metallicity, int n)
{
	Opts = opts;
	Mass = mass;
	Metallicity = metallicity;
	NStars = n;
}
