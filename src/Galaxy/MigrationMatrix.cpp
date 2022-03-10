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
		totalMass = std::max(totalMass,mass[i]);
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
					term += mass[i-1];
				}
				if (i < n-1)
				{
					term += mass[i+1];
				}
				K[i][j] = -kappa * term;
				K_power[i][j] = K[i][j];
			}
			if (abs(j-i) == 1)
			{
				K[i][j] = kappa  * mass[i];
				K_power[i][j] = K[i][j];
			}
		}
	}
	
	double dt = Param.Meta.TimeStep;
	double factorialInverse = 1;
	for (int order = 1; order <= Param.Migration.DispersionOrder; ++order)
	{
		if (order > 1)
		{
			//multiply up
			K_power = DiagonalMultiply(K_power,K,order);
		}
		factorialInverse /= order;
		for (int i = 0; i < n; ++i)
		{
			int lower = std::max(0,i - order-1);
			int upper = std::min(n, i + order+1);
			for (int j = lower; j < upper; ++j)
			{
				Grid[i][j] += pow(dt,order) * factorialInverse * K_power[i][j];
			}
		}
	}
	//prevent negative values in migration matrix -- shouldn't happen in analytical case, but might happen due to finite precision errors
	int maxOrder = Param.Migration.DispersionOrder;
	for (int i = 0; i < n; ++i)
	{
		int lower = std::max(0,i - maxOrder-2);
		int upper = std::min(n, i + maxOrder+2);
		double sum = 0;
		for (int j = lower; j < upper; ++j)
		{
			Grid[i][j] = std::max(Grid[i][j],0.0);
			sum += Grid[j][i];
		}
		//normalise to ensure true stochasticity
		for (int j = lower; j < upper; ++j)
		{
			Grid[i][j] /= sum;
		}
	}
	//~ exit(5);
	
	
	//~ std::cout << "New migration matrix: " << std::endl;
	//~ for (int i = 0; i < n; ++i)
	//~ {
		//~ int lower = 0;
		//~ int upper = n;
		//~ for (int j = lower; j < upper; ++j)
		//~ {
			//~ std::cout <<std::setw(10) << Grid[i][j];
		//~ }
		//~ std::cout << "\n";
	//~ }
	
}

void MigrationMatrix::Compound(const MigrationMatrix & newMatrix)
{
	int n = Param.Galaxy.RingCount;
	int maxOffDiagonal = Param.Migration.DispersionOrder +1;
	std::vector<std::vector<double>> temp(n,std::vector<double>(n,0.0));
	for (int i = 0; i < n; ++i)
	{
		int lower = std::max(0, i - maxOffDiagonal-1);
		int upper = std::min(n, i + maxOffDiagonal+1);
		for (int j = lower; j < upper; ++j)
		{
			
			for (int k = lower; k < upper; ++k)
			{
				temp[i][j] += newMatrix.Grid[i][k] * Grid[k][j];
			}
			
		}
		
	}
	Grid = temp;
	
	//~ std::cout << "New compounded matrix: " << std::endl;
	//~ for (int i = 0; i < n; ++i)
	//~ {
		//~ int lower = 0;
		//~ int upper = n;
		//~ for (int j = lower; j < upper; ++j)
		//~ {
			//~ std::cout <<std::setw(10) << Grid[i][j];
		//~ }
		//~ std::cout << "\n";
	//~ }
	
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

void MigrationMatrix::Print()
{
	int n = Grid.size();
	
	for(int i = 0; i < n; ++i)
	{
		for (int k = 0; k < n; ++k)
		{
			std::cout << std::setw(15) << Grid[i][k];
		}
		std::cout << "\n";
	}
	
	
}
