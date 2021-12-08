#include "GasReservoir.h"

GasReservoir::GasReservoir()
{
	Components.resize(ProcessCount);
}

GasReservoir GasReservoir::Primordial(double mass, GlobalParameters & param)
{
	GasReservoir prim;
	
	Gas g = Gas::Primordial(mass);
	double fh = param.Galaxy.PrimordialHotFraction;
	GasStream primordialStream = GasStream(SourceProcess::Primordial,g,fh);
	
	prim.Absorb(primordialStream);
	return prim;
	
}
const GasStream & GasReservoir::Component(SourceProcess source) const
{
	return Components[source];
}


void GasReservoir::Absorb(const GasReservoir & givingGas)
{
	for (int i = 0; i < ProcessCount; ++i)
	{
		SourceProcess source = (SourceProcess)i;
		Absorb(givingGas.Component(source));
	}
}
void GasReservoir::Absorb(const GasStream & givingGas)
{
	SourceProcess source = givingGas.Source;
	Components[source].Absorb(givingGas);
}
