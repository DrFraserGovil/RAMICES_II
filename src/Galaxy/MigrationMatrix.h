#pragma once
#include <vector>
#include "../Parameters/InitialisedData.h"
#include <iomanip>
class MigrationMatrix
{
	public:
		MigrationMatrix(InitialisedData & Data);
		
		std::vector<std::vector<double>> Grid;
		
		void Create(const std::vector<double> & masses);
		
		void Compound(const MigrationMatrix & newTime);
	private:
		int NRings;
		const GlobalParameters & Param;
		std::vector<std::vector<double>> DiagonalMultiply(const std::vector<std::vector<double>> & a, const std::vector<std::vector<double>> & b, int diagonalDistance);
		
};
