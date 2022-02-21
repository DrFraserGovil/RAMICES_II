#include "Ring.h"

const double pi = 3.141592654;
//! Initialises itself into a primordial state
Ring::Ring(int index, double mass,InitialisedData & data): Data(data), Param(data.Param), Width(data.Param.Galaxy.RingWidth[index]), Radius(data.Param.Galaxy.RingRadius[index]), Gas(GasReservoir::Primordial(mass,data.Param)), Stars(index,data)
{
	RadiusIndex = index;
	Area = 2 * pi * Radius * Width;
	//~ PreviousEnrichment.resize(Param.Meta.SimulationSteps);
	//~ PreviousEnrichment[0] = Gas;
	Data.Log("\tRing " + std::to_string(index) + " initialised\n",3);
}

double Ring::Mass()
{
	MassReport Mrr = Stars.DeadMass();
	return Gas.Mass() + Stars.AliveMass() + Mrr.Total /1e9; 
}


void Ring::TimeStep(int t)
{
	//~ std::cout << "Ring " << RadiusIndex << " at " << t << std::endl;
	MetCheck("Start of internal loop");
	Cool();
	
	MakeStars();

	if (t < Param.Meta.SimulationSteps-2)
	{
		KillStars(t);
	}

	MetCheck("End of internal loop");
}

void Ring::MakeStars()
{
	Stars.Form(Gas);
}
void Ring::KillStars(int time)
{
	Stars.Death(time);

	double m = 0;
	for (int t = 0; t < time; ++t)
	{
		m += Stars.YieldOutput[t][Remnant].ColdMass();
	}
}

void Ring::Cool()
{
	Gas.PassiveCool(Param.Meta.TimeStep,false);
}
void Ring::UpdateMemory(int t)
{
	//~ Gas.UpdateMemory(t);
}

void neatLog(double value, std::stringstream & stream)
{
	if (isinf(value) || isnan(value))
	{
		stream << ",-";
	}
	else
	{
		stream << ", " << value;
	}
}

void Ring::SaveChemicalHistory(int t, std::stringstream & absoluteStreamCold, std::stringstream & logarithmicStreamCold, std::stringstream & absoluteStreamHot, std::stringstream & logarithmicStreamHot)
{
	std::string basic = "";
	if (t == 0)
	{
		HotBuffer = std::vector<std::vector<double>>(ProcessCount + 1, std::vector<double>(ElementCount,0.0));
		ColdBuffer = std::vector<std::vector<double>>(ProcessCount + 1, std::vector<double>(ElementCount,0.0));
		
		if (RadiusIndex == 0)
		{
			std::string headers = "Time, RingIndex, RingRadius";
			for (int p = -1; p < ProcessCount; ++p)
			{
				std::string processName;
				if (p > -1)
				{
					 processName = Param.Yield.ProcessNames[p];
				}
				else
				{
					processName = "Total";
				}
				for (int e = 0; e < ElementCount; ++e)
				{
					std::string elementName = Param.Element.ElementNames[e];
				
				
					headers += ", " + processName+ "_" + elementName;
				}
			}
			basic = headers + "\n";
		}
	}
	basic += std::to_string(t*Param.Meta.TimeStep) + ", " + std::to_string(RadiusIndex) + ", " + std::to_string(Radius);
	
	absoluteStreamCold << basic;
	logarithmicStreamCold  << basic;
	absoluteStreamHot   << basic;
	logarithmicStreamHot   << basic;
	
	const std::vector<GasStream> & target = Stars.Population[t].BirthGas;
	
	double coldMass = 0;
	double hotMass = 0;
	for (int p = 0; p < ProcessCount; ++p)
	{
		double processCold = target[p].ColdMass();
		double processHot = target[p].HotMass();
		
		coldMass += processCold;
		hotMass += processHot;
		for (int e = 0; e < ElementCount; ++e)
		{
			ElementID elem = (ElementID)e;
			double cold = target[p].Cold(elem);
			double hot = target[p].Hot(elem);

			if (p == 0)
			{
				ColdBuffer[p][e] = 0;
				HotBuffer[p][e] = 0;
			}
			ColdBuffer[0][e] += cold;
			HotBuffer[0][e] += hot;
			ColdBuffer[p+1][e] = cold/processCold;
			HotBuffer[p+1][e] = hot/processHot;
		}
	} 
	for (int e = 0; e < ElementCount; ++e)
	{
		ColdBuffer[0][e] /= (coldMass+1e-88);
		HotBuffer[0][e] /= (hotMass+1e-88);
	}
	

	for (int p = 0; p < ProcessCount + 1; ++p)
	{
		for (int e = 0; e < ElementCount; ++e)
		{
			double coldCorrect = coldMass;
			double hotCorrect = hotMass;
			if (p > 0)
			{
				coldCorrect = target[p-1].ColdMass();
				hotCorrect = target[p-1].HotMass();
			}
			neatLog(ColdBuffer[p][e] * coldCorrect, absoluteStreamCold);
			neatLog(HotBuffer[p][e] * hotCorrect, absoluteStreamHot);
					
			double logValueCold = log10(ColdBuffer[p][e] / Param.Element.SolarAbundances[e]);
			double logValueHot = log10(HotBuffer[p][e]/Param.Element.SolarAbundances[e]);
			
			neatLog(logValueCold,logarithmicStreamCold);
			neatLog(logValueHot,logarithmicStreamHot);
	
		}
	}
	absoluteStreamCold << "\n";
	logarithmicStreamCold  << "\n";
	absoluteStreamHot   << "\n";
	logarithmicStreamHot   << "\n";
}


void Ring::MetCheck(const std::string & location)
{
	if (Gas.Mass() < 0)
	{
		std::cout << "\n\nThe gas in Ring " << RadiusIndex << " has negative mass -- something has gone very wrong!" << std::endl;
		exit(5);
	}
	double z = Gas.ColdGasMetallicity();
	if (z < 0)
	{
		std::cout << "\n\nThe gas in Ring " << RadiusIndex << " had a negative metallicity at " << location << "\n Critical Error!" << std::endl;
		std::cout << "The components of the ring are: \n";
		for (int p = 0; p < ProcessCount; ++p)
		{
			SourceProcess proc = (SourceProcess)p;
			for (int e = 0; e < ElementCount; ++e)
			{
				ElementID elem = (ElementID)e;
				std::cout << Gas[proc].Cold(elem) << "\t";
			}
			std::cout << "\n";
		}
		exit(5);
		
	}
	
}



double tan_SkyCut(double theta)
{

	double order0 = 0.715;
	double order1 = -1.24 * cos(theta) + 1.915 * sin(theta);
	double order2 = -0.114 * cos(2*theta) - 0.2553*sin(2*theta);
	
	return 9999;//order0 + order1 + order2;
}

void Ring::ComputeSelectionFunction(double minMv,double maxMv, const std::vector<double> & times)
{
	IsoTimes = times;
	int Nm = Param.Catalogue.IsochroneMagnitudeResolution;
	int Nt = times.size();
	SelectionGrid = std::vector<std::vector<double>>(Nt,std::vector<double>(Nm,0.0));
	
	int Nr = Param.Catalogue.RadialResolution;
	int Nphi = Param.Catalogue.AzimuthalResolution;
	
	double dSol = Param.Catalogue.SolarRadius;
	double dr = Width/(Nr);
	double dphi = (2*M_PI)/(Nphi-1); 
	
	double z0 = Param.Catalogue.VerticalHeightStart;
	double kappa = Param.Catalogue.VerticalHeightScaling;
	double tauN = Param.Catalogue.VerticalHeightPower;
	for (int t = 0; t < Nt; ++t)
	{
		double zBar = z0 + kappa * pow(times[t],tauN);
		
		
		double time = times[t];
		
		double minM = bright[t];
		double maxM = dim[t];
		
		double deltaM = (maxM - minM)/(Nm - 1);
		
		for (int i = 0; i < Nm; ++i)
		{
			double Mv = minM + i * deltaM;
			
			double maxDistance = pow(10, (4.0 - Mv)/5);
			double minDistance = pow(10, (2.0 - Mv)/5);
			
			double val = 0;
			double normVal = 0;
			for (int ri = 0; ri < Nr; ++ri)
			{
				double r = Radius - Width/2 + ri * dr;
				double phiVal =0;
				for (int angle = 0; angle < Nphi; ++angle)
				{
					double phi= angle * dphi;
					
					double inPlaneDistance = sqrt(dSol * dSol + r*r - 2 * dSol * r * cos(phi));
					
					double s = 0;
					

					
					if (inPlaneDistance <= maxDistance)
					{
						double dpSq = inPlaneDistance * inPlaneDistance;
						double ell = asin(r / inPlaneDistance * sin(phi));
					
						double zCut = inPlaneDistance * tan_SkyCut(ell);
						double discCut_Degrees = 0.0;
						double discCut = inPlaneDistance * tan( discCut_Degrees * M_PI/180);
						
						double bPlus = sqrt(maxDistance * maxDistance - dpSq);
						double aMinus = sqrt( std::max( dpSq - minDistance * minDistance, discCut * discCut));
						double aPlus = std::min(zCut, bPlus);
						
						if (aPlus > aMinus)
						{
							s += 0.5 * (exp(-aMinus/zBar) - exp(-aPlus/zBar));
						}
						
						double bMinus = abs( std::min(-aMinus,zCut));
						if (bPlus > bMinus)
						{
							s += 0.5 * (exp( -bMinus/zBar) - exp( - bPlus/zBar));
						}
	
					}
					
					
					phiVal += r * s * dr * dphi;
					normVal += r * dr * dphi;
				}
				val += phiVal;
			}
			val /= normVal;
			
			
			SelectionGrid[t][i] = val;
			
		}
	}
}

double Ring::SelectionEffect(double Mv, double age)
{
	double dt = IsoTimes[1] - IsoTimes[0];
	
	int IDt = floor(age/dt);
	if (IDt >= IsoTimes.size()-1)
	{
		IDt = IsoTimes.size() - 2;
	}
	
	int Nm = Param.Catalogue.IsochroneMagnitudeResolution;
	double dM_Old = (IsoDim[IDt] - IsoBright[IDt])/(Nm - 1);
	double dM_New = (IsoDim[IDt+1] - IsoBright[IDt+1])/(Nm - 1);
	
	int IDm_Old = floor((Mv - IsoBright[IDt])/dM_Old);
	int IDm_New = floor((Mv - IsoBright[IDt+1])/dM_New);
	
	IDm_Old = std::max(0,std::min(IDm_Old,Nm - 2));
	IDm_New = std::max(0,std::min(IDm_New,Nm - 2));
	
	double old_Interp = std::min(1.0,std::max(0.0,(Mv - IDm_Old * dM_Old)/dM_Old));
	double new_Interp = std::min(1.0,std::max(0.0,(Mv - IDm_New * dM_New)/dM_New));
	
	
	double oldVal = SelectionGrid[IDt][IDm_Old] + old_Interp * (SelectionGrid[IDt][IDm_Old+1] - SelectionGrid[IDt][IDm_Old]);
	double newVal = SelectionGrid[IDt+1][IDm_New] + new_Interp * (SelectionGrid[IDt+1][IDm_New+1] - SelectionGrid[IDt+1][IDm_New]);
	
	double tInterp = std::min(1.0, std::max(0.0, (age - IsoTimes[IDt])/dt));
	

	double val = oldVal + tInterp * (newVal - oldVal);
	
	return val;
}
