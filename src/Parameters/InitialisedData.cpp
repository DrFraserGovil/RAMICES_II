#include "InitialisedData.h"

InitialisedData::InitialisedData(const GlobalParameters & param) : Param(param), IMF(param), SLF(param)
{
	
	UrgentLog("\tPortable data structure initialisation complete\n");
}


void InitialisedData::Log(const std::string & input) const
{
	Log(input,1);
}
void InitialisedData::Log(const std::string & input, int importance) const
{
	if (Param.Meta.Verbosity > 0 && importance <= Param.Meta.Verbosity)
	{
		std::cout << input;
	}
}
void InitialisedData::LogFlush() const
{
	std::cout << std::flush;
}
void InitialisedData::UrgentLog(const std::string & input) const
{
	Log(input,1);
	LogFlush();
}


void InitialisedData::ProgressBar(int & currentBars, int i,int N)
{
	if (Param.Meta.Verbosity > 0)
	{
		int fullBar = Param.Meta.ProgressHashes;
		double progress = (double)i/(N-2);
		int predictedBars = floor(fullBar * progress);
		int neededBars = predictedBars - currentBars;
		std::string s = "";
		if (i == 0)
		{
			s = "[";
		}
		while (neededBars > 0)
		{
			s += "#";
			--neededBars;
			++currentBars;
			
		}
		if (currentBars == fullBar)
		{
			s+="]\n";
			currentBars = fullBar + 2;
		}
		UrgentLog(s);
	}
}
