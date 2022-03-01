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
	
	return order0 + order1 + order2;
}

void Ring::ComputeSelectionFunction(double minMv,double maxMv)
{
	MinMv = minMv;
	MaxMv = maxMv;
	
	int Nm = Param.Catalogue.IsochroneMagnitudeResolution;
	double dt = Param.Catalogue.IsochroneTimeStep;
	int Nt = ceil((double)Param.Meta.SimulationDuration / dt) + 1;
	double deltaM = (maxMv - minMv)/(Nm - 1);
	SelectionGrid = std::vector<std::vector<double>>(Nt,std::vector<double>(Nm,0.0));
	
	int Nr = Param.Catalogue.RadialResolution;
	int Nphi = Param.Catalogue.AzimuthalResolution;
	
	double dSol = Param.Catalogue.SolarRadius;
	double dr = Width/(Nr);
	double dphi = (2*M_PI)/(Nphi-1); 
	
	double z0 = Param.Catalogue.VerticalHeightStart;
	double kappa = Param.Catalogue.VerticalHeightScaling;
	double tauN = Param.Catalogue.VerticalHeightPower;
	
	
	bool printy = (RadiusIndex == 40);
	
	for (int t = 0; t < Nt; ++t)
	{
		double time = t* dt; 
		double zBar = z0 + kappa * pow(time,tauN);
		
		//~ if (printy)
			//~ std::cout << "\n\n\tEntry for age " << time << " has scale height " << zBar << std::endl;
		
		for (int i = 0; i < Nm; ++i)
		{
			double Mv = minMv + i * deltaM;
			
			double maxDistance = pow(10, (4.0 - Mv)/5);
			double minDistance = pow(10, (2.0 - Mv)/5);
			
			//~ if (printy)
				//~ std::cout << "\t\tEntry for mag " << Mv << " must lie between " << minDistance << "  and " << maxDistance << std::endl;
			
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
						double discCut_Degrees = 10.0;
						double discCut = inPlaneDistance * tan( discCut_Degrees * M_PI/180);
						
						double upperCut = inPlaneDistance * tan( 30.0 * M_PI/180);
						
						double bPlus = std::min(upperCut,sqrt(maxDistance * maxDistance - dpSq));
						double aMinus = sqrt( std::max( minDistance * minDistance - dpSq, discCut * discCut));
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
						//~ if (printy)
						//~ {
							//~ std::vector<std::string> n = {"r","phi","d_plane", "zCut","discCut","aminus","aplus","bminus","bplus","s"};
							//~ std::vector<double> v = {r,phi,inPlaneDistance,zCut,discCut,aMinus,aPlus,bMinus,bPlus,s};
							
							//~ std::cout << "\t";
							//~ for (int h = 0; h < n.size(); ++h)
							//~ {
								//~ std::cout << std::setw(20) << n[h] + "="  + std::to_string(v[h]);
							//~ }
							//~ std::cout << "\n";
						//~ }
					}
					
					
					phiVal += r * s * dr * dphi;
					normVal += r * dr * dphi;
				}
				val += phiVal;
			}
			val /= normVal;
			
			//~ if (printy)
				//~ std::cout << "\t\tGiving a selection value of " << val << std::endl;
			SelectionGrid[t][i] = val;
			
		}
	}
	
	
	//~ if (RadiusIndex == 99)
	//~ {
		//~ std::cout << "Outputting selection grid data for the ring " << Radius << "  " << Width << ": " << std::endl;
		
		//~ for (int j = 0; j < Nm; ++j)
		//~ {
			//~ for (int i = 0; i < Nt; ++i)
			//~ {
			
				//~ double t = i * dt;
				//~ double Mv = minMv + j * deltaM;
				//~ double maxDistance = pow(10, (4.0 - Mv)/5);
				//~ double minDistance = pow(10, (2.0 - Mv)/5);
				//~ std::cout << t << "  " << Mv<< "  " << SelectionGrid[i][j] <<"  " << minDistance << "  " << maxDistance << std::endl;
			//~ }
		//~ }
		
	//~ }
}

double Ring::SelectionEffect(double Mv, double age)
{
	int Nm = Param.Catalogue.IsochroneMagnitudeResolution;
	double dt = Param.Catalogue.IsochroneTimeStep;
	int Nt = ceil((double)Param.Meta.SimulationDuration / dt) + 1;
	double deltaM = (MaxMv - MinMv)/(Nm - 1);

	double mvProgress = (Mv - MinMv)/deltaM;
	double tProgress = age/dt;
	int mv_id = std::min(Nm - 2,std::max(0,(int)mvProgress));
	int t_id = std::min(Nt - 2,std::max(0,(int)tProgress));
	
	double mvInterp = (mvProgress - mv_id);
	double tInterp = (tProgress - t_id);
	
	double lowTVal = SelectionGrid[t_id][mv_id] + mvInterp * (SelectionGrid[t_id][mv_id+1]-SelectionGrid[t_id][mv_id]);
	double highTVal = SelectionGrid[t_id+1][mv_id] + mvInterp * (SelectionGrid[t_id+1][mv_id+1] - SelectionGrid[t_id+1][mv_id]);
	double val = lowTVal + tInterp * (highTVal - lowTVal);	
	
	//~ if (RadiusIndex == 99)
	//~ {
		//~ std::cout << "\tSelection effect call for ring " << RadiusIndex << " for Mv = " << Mv << " age " << age << " my grid coords are:\n";
		//~ std::cout << "\t\t M is: " << mv_id
		//~ std::cout << "\t\t LowT: " << t_id * dt << ": " << SelectionGrid[t_id][mv_id] << "-> " <<
		//~ std::cout << "\t\t For final value: " << val << std::endl;
	//~ }
	return val;
}

std::string Ring::Synthesis(const StellarPopulation & targetPopulation, double migrateFrac, double originRadius, double & totalSynthesised)
{
	std::string output = "";
	double age = targetPopulation.Age;
	for (int m = 0; m < Param.Stellar.MassResolution; ++m)
	{
		//~ std::cout << "\t Mass " << m << std::endl;
		if (targetPopulation.Distribution[m].Count > 0)
		{		
			const IsochroneCube & iso = targetPopulation.Distribution[m].Isochrone;
			int n = iso.Count();
			std::vector<int> numberSynthesised(n,0);
			double mass = Param.Stellar.MassGrid[m];
			int totalObs = 0;
			for (int entry = 0; entry < n; ++entry)
			{
				
				double Mv = iso.Value(entry,VMag);
				double populationWeighting = iso.Weighting[entry];
				double observeFrac = SelectionEffect(Mv,age);
				double count = migrateFrac * targetPopulation.Distribution[m].Count * populationWeighting;
				
				double crowdingFactor =0.1;

				double obs = observeFrac * count * crowdingFactor;
				
				int intObs = obs;

				double targetRoll = (obs - intObs);
				double diceRoll = (double)rand() / RAND_MAX;
				if (diceRoll < targetRoll)
				{
					++intObs;
				}
				numberSynthesised[entry] = intObs;
				totalObs += intObs;
			}
			
			
			if (totalObs > 0)
			{
				output += targetPopulation.CatalogueEntry(numberSynthesised,m,Radius,originRadius);
				//~ SynthesisOutput[i] += output;
				totalSynthesised += totalObs;
			}
		
			
		}
	}
	return output;
}
