#pragma once
#include "../Parameters/GlobalParameters.h"
#include "Ring.h"
#include "../Gas/GasReservoir.h"
#include "../Stars/IMF.h"
#include "../Stars/SLF.h"
#include <sstream>

class Galaxy
{
	public:
		Galaxy(const GlobalParameters & param);
		void Evolve();
	
	
	private:
		std::vector<Ring> Rings;
		GasReservoir IGM;
		const GlobalParameters & Param;
		
		//Infall Stuff
		double GasScaleLength(double t);
		double InfallMass(double t);
		void InsertInfallingGas(int ring, double amount);
		void Infall(double t);
		
		//Star Formation
		void FormStars();
		void Cool();
		void KillStars(int time);
		
		double PredictSurfaceDensity(double radius,double width, double totalGasMass, double scalelength);
		double GasMass();
		double StarMass();

		double RelicMass();
		double Mass();
		void SaveState(double t);
		void SaveState_Mass(double t);
		
		static std::string MassHeaders(); 
		
		IMF_Functor IMF;
		SLF_Functor SLF;
		

};
