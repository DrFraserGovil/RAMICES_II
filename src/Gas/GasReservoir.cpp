#include "GasReservoir.h"

GasReservoir::GasReservoir() : Param(GlobalParameters())
{
	Components.resize(ProcessCount);
	//make sure each stream has its own label correctly
	for (int i = 0; i < ProcessCount; ++i)
	{
		Components[i].Source = (SourceProcess)i;
	}
}
GasReservoir::GasReservoir(const GlobalParameters & param): Param(param)
{
	Components.resize(ProcessCount);
	//make sure each stream has its own label correctly
	for (int i = 0; i < ProcessCount; ++i)
	{
		Components[i].Source = (SourceProcess)i;
	}
}

GasReservoir GasReservoir::Primordial(double mass, const GlobalParameters & param)
{
	GasReservoir prim(param);
	
	Gas g = Gas::Primordial(mass);
	double fh = param.Galaxy.PrimordialHotFraction;
	GasStream primordialStream = GasStream(SourceProcess::Primordial,g,fh);
	
	prim.Absorb(primordialStream);
	return prim;
	
}
const GasStream & GasReservoir::operator[](SourceProcess source) const
{
	return Components[source];
}
GasStream & GasReservoir::operator[](SourceProcess source)
{
	Components[source].Dirty(); //non-constant reference so have to alert the Stream that somebody has been touching its stuff
	return Components[source];
}


double GasReservoir::Mass()
{
	double sum = 0;
	for (int i = 0; i < ProcessCount; ++i)
	{
		sum += Components[i].Mass();
	}
	return sum;
}
double GasReservoir::ColdMass()
{
	double sum = 0;
	for (int i = 0; i < ProcessCount; ++i)
	{
		sum += Components[i].ColdMass();
	}
	return sum;
}
double GasReservoir::HotMass()
{
	double sum = 0;
	for (int i = 0; i < ProcessCount; ++i)
	{
		sum += Components[i].HotMass();
	}
	return sum;
}

void GasReservoir::Absorb(const GasReservoir & givingGas)
{
	for (int i = 0; i < ProcessCount; ++i)
	{
		SourceProcess source = (SourceProcess)i;
		Absorb(givingGas[source]);
	}
}

void GasReservoir::Deplete(double amountToLose)
{
	double totalMass = Mass();
	
	amountToLose = std::min(amountToLose,totalMass);
	double contFraction = amountToLose / Mass();
	for (int i = 0; i < ProcessCount; ++i)
	{
		if (Components[i].Mass() > 0)
		{
			
			double componentContribution =  Components[i].Mass() * contFraction;
			Components[i].Deplete(componentContribution);
		}
	}
	
}
void GasReservoir::Absorb(const GasStream & givingGas)
{
	SourceProcess source = givingGas.Source;
	Components[source].Absorb(givingGas);
}

GasStream GasReservoir::AccretionStream(double amountToLose)
{
	double initMass = ColdMass();
	amountToLose = std::min(amountToLose,initMass); //Can't lose more than I have!
	GasStream output(Accreted);
	double lossFraction = amountToLose / initMass;
	for (int i = 0; i < ProcessCount; ++i)
	{
		
		for (int j = 0; j < ElementCount; ++j)
		{
			ElementID elem = (ElementID)j;
			double extract = lossFraction * Components[i].Cold(elem);
			Components[i].Cold(elem) -= extract;
			output.Cold(elem) += extract; 
		}	
	}
	return output;
}

void GasReservoir::TransferFrom(GasReservoir & givingGas, double massToMove)
{
	double lossFraction = std::min(1.0,massToMove/givingGas.Mass());
	
	
	
	for (int i = 0; i < ProcessCount; ++i)
	{
		SourceProcess proc = (SourceProcess)i;
		for (int j = 0; j < ElementCount; ++j)
		{
			ElementID elem = (ElementID)j;
			double currentCold = givingGas[proc].Cold(elem);
			double currentHot = givingGas[proc].Hot(elem);
			double extractCold = lossFraction * currentCold;
			
			double extractHot = lossFraction * currentHot;
			givingGas[proc].Cold(elem) -= extractCold;
			givingGas[proc].Hot(elem) -= extractHot;
			
			Components[proc].Cold(elem) += extractCold; 
			Components[proc].Hot(elem) += extractHot;		
		}	
	}

}

void GasReservoir::PrintSelf()
{
	for (int c = 0; c < ProcessCount; ++c)
	{
		SourceProcess source = (SourceProcess)c;
		auto stream = Components[source];
		bool headerPrinted = false;
		
		for (int i = 0; i < ElementCount; ++i)
		{
			ElementID elem = (ElementID)i;
			if (stream.Cold(elem)!= 0 || stream.Hot(elem) != 0)
			{
				if (!headerPrinted)
				{
					std::cout << "From process " << c << ": \n";
					headerPrinted = true;
				}
				std::cout << "\t" << Param.Element.ElementNames[elem] << " Cold = " << stream.Cold(elem) << "  Hot = " << stream.Hot(elem) << std::endl;
			}
		}
	}
}
	
