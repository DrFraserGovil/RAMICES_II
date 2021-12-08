#include "YieldRidge.h"

YieldRidge::YieldRidge()
{
	Z = 0;
}

YieldRidge::YieldRidge(int ID, double z, int nPoints)
{
	SourceID = ID;
	Z = z;
	Merged = false;

	
	Points = std::vector<YieldPoint>(nPoints, YieldPoint(0.0,0.0));
}



double YieldRidge::MassInterp(double mass,Options * opts)
{
	bool isOverweight = mass > Points[Points.size()-1].Mass;
	bool isUnderweight = mass <= Points[0].Mass;
	
	int topID = 0;
	if ( isOverweight || isUnderweight)
	{
		if (isOverweight)
		{
			topID = Points.size() - 1;
		}
		else
		{
			topID = 1;
		}
	}
	else
	{
		while ( Points[topID].Mass < mass)
		{
			++topID;
		}
	}

	double topMass = Points[topID].Mass;
	double lowMass = Points[topID-1].Mass;
	
	double interpFactor = (mass - lowMass)/(topMass - lowMass);
	double maxInterp = 1.0 + opts->Element.maxInterpolationFactor;
	double minInterp = - opts->Element.maxInterpolationFactor;
	if (interpFactor > maxInterp)
	{
		interpFactor = maxInterp;
	}
	if (interpFactor < minInterp)
	{
		interpFactor = minInterp;
	}
	
	double midYield =  Points[topID-1].Yield + (Points[topID].Yield - Points[topID - 1].Yield) * interpFactor;
	
	int interpolateSign = (midYield > 0);
	int upDataSign = (Points[topID-1].Yield > 0);
	int downDataSign = (Points[topID].Yield > 0);
	
	if ( (interpolateSign != upDataSign) && (interpolateSign != downDataSign))
	{
		midYield = 0;
	}
	
	
	return midYield;
}


YieldRidge MergeRidges(std::vector<YieldRidge> candidates, Options * opts)
{
	//order goes from left to right - first element takes highest priority
	std::vector<double> PriorityOrder = {opts->Element.OrfeoID,opts->Element.MaederID, opts->Element.MarigoID, opts->Element.LimongiID};
	

	
	std::vector<YieldPoint> mergedPoints = {};

	double sumZ = 0;
	for (int i = 0; i < candidates.size(); ++i)
	{		
		sumZ += candidates[i].Z;
		auto basePriority = std::find(PriorityOrder.begin(), PriorityOrder.end(), candidates[i].SourceID);
		for (YieldPoint point : candidates[i].Points)
		{
			bool noConflict = true;
			
			
			
			for (int j = 0; j < candidates.size(); ++j)
			{
				if (i != j)
				{	
					YieldRidge ridge2 = candidates[j];
					auto [minM2, maxM2] = std::minmax_element(ridge2.Points.begin(), ridge2.Points.end(), [](const YieldPoint& a, const YieldPoint& b) {  return a.Mass < b.Mass;});
					YieldPoint low2 = *minM2;
					YieldPoint top2= *maxM2;
				
					bool liesInsideRegion = (  point.Mass <= top2.Mass) && (point.Mass >= low2.Mass);
					
					if (liesInsideRegion)
					{
						auto comparisonPriority = std::find(PriorityOrder.begin(), PriorityOrder.end(), candidates[j].SourceID);
						
						if (comparisonPriority < basePriority)
						{
							noConflict = false;
						}
					}
					
				}
			}
			
			if (noConflict)
			{
				mergedPoints.push_back(point);
			}
		
		}
	}
	
	sort(mergedPoints.begin(), mergedPoints.end(), [](const YieldPoint& a, const YieldPoint& b) {  return a.Mass < b.Mass;});
	
	double meanZ = sumZ / candidates.size();
	YieldRidge merger;
	merger.Z = meanZ;
	merger.Points = mergedPoints;
	merger.Merged = true;
	merger.SourceID = opts->Element.MixedID;
	return merger;
}
