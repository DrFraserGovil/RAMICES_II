#include "MigrationMatrix.h"

MigrationMatrix::MigrationMatrix(InitialisedData & Data) : Param(Data.Param)
{
	int n = Param.Galaxy.RingCount;
	Grid = std::vector<std::vector<double>>(n,std::vector<double>(n,0.0));
	for (int i = 0; i < n; ++i)
	{
		Grid[i][i] = 1.0;
	}
}

void MigrationMatrix::Create(const std::vector<double> & mass)
{
	if (Param.Migration.DispersionOrder == 0)
	{
		return;
	}
	int n = Param.Galaxy.RingCount;
	double totalMass = 0;
	for (int i = 0; i < n; ++i)
	{
		totalMass += mass[i];
	}
	
	double kappa = Param.Migration.MarkovDispersionStrength / totalMass;
	std::vector<std::vector<double>> K(n,std::vector<double>(n,0.0));
	std::vector<std::vector<double>> K_power(n,std::vector<double>(n,0.0));
	std::vector<std::vector<double>> K_power_temp(n,std::vector<double>(n,0.0));
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			if (i == j)
			{
				double term = 0;
				if (i > 0)
				{
					term += mass[i-1]/Param.Galaxy.RingWidth[i-1];
				}
				if (i < n-1)
				{
					term += mass[i+1]/Param.Galaxy.RingWidth[i+1];
				}
				K[i][j] = -kappa * term;
				K_power[i][j] = K[i][j];
			}
			if (abs(j-i) == 1)
			{
				K[i][j] = kappa/(Param.Galaxy.RingWidth[i]) * mass[i];
				K_power[i][j] = K[i][j];
			}
		}
	}
	
	double dt = Param.Meta.TimeStep;
	double factorial = 1;
	for (int order = 1; order <= Param.Migration.DispersionOrder; ++order)
	{
		if (order > 1)
		{
			//multiply up
			K_power = DiagonalMultiply(K_power,K,order);
		}
		factorial = factorial * order;
		for (int i = 0; i < n; ++i)
		{
			int lower = std::max(0,i - order-1);
			int upper = std::min(n, i + order+1);
			for (int j = lower; j < upper; ++j)
			{
				Grid[i][j] += pow(dt,order)/factorial * K_power[i][j];
			}
		}
	}
	//prevent negative values in migration matrix -- shouldn't happen in analytical case, but might happen due to finite precision errors
	int maxOrder = Param.Migration.DispersionOrder;
	for (int i = 0; i < n; ++i)
	{
		int lower = std::max(0,i - maxOrder-1);
		int upper = std::min(n, i + maxOrder+1);
		for (int j = lower; j < upper; ++j)
		{
			Grid[i][j] = std::max(Grid[i][j],0.0);
		}
	}
	//~ exit(5);
	
}

void MigrationMatrix::Compound(const MigrationMatrix & newMatrix)
{
	int n = Param.Galaxy.RingCount;
	int maxOffDiagonal = Param.Migration.DispersionOrder +1;
	std::vector<std::vector<double>> temp(n,std::vector<double>(n,0.0));
	for (int i = 0; i < n; ++i)
	{
		int lower = std::max(0, i - maxOffDiagonal);
		int upper = std::max(0, i + maxOffDiagonal);
		for (int j = 0; j < n; ++j)
		{
			
			for (int k = lower; k < upper; ++k)
			{
				temp[i][j] = newMatrix.Grid[i][k] * Grid[i][j];
			}
			
		}
		
	}
	Grid = temp;
	
}

std::vector<std::vector<double>> MigrationMatrix::DiagonalMultiply(const std::vector<std::vector<double>> & a, const std::vector<std::vector<double>> & b, int diagonalDistance)
{
	
	int n = a.size();
	std::vector<std::vector<double>> output(n,std::vector<double>(n,0.0));
	
	for (int i = 0; i < n; ++i)
	{
		int lower = std::max(0,i - diagonalDistance - 1);
		int upper = std::min(n, i + diagonalDistance + 1);
		for (int j = lower; j < upper; ++j)
		{
			for (int k = lower; k < upper; ++k)
			{
				output[i][j] += a[i][k] * b[k][j];
			}
		}
	}
	return output;
}
