#pragma once
#include "../Stars/IMF.h"
#include "../Stars/SLF.h"
#include "GlobalParameters.h"

//! These will act like globally-defined functions, but have the scope for modifying themselves as they go along
class InitialisedData
{
	public:
		const IMF_Functor IMF;
		SLF_Functor SLF;
		const GlobalParameters & Param;
		
		
		InitialisedData(const GlobalParameters & param);
	
		void Log(const std::string & input) const;
		void Log(const std::string & input, int importance) const;
		void LogFlush() const;
		void UrgentLog(const std::string & input) const;
	
		void ProgressBar(int & currentBars, int currentStep, int totalSteps);
	
	private:
		
	
	
};
