#pragma once
#include <random>
#include <chrono>
template<typename T = std::mt19937>
class Random
{
	private:
		T engine;
		std::normal_distribution<double> norm{0.0,1.0};
	
	public:
		Random()
		{
			std::random_device rd;
			Seed(rd());
		};
		Random(uint32_t seed)
		{
			// std::random_device rd;
			// Seed(rd());
			Seed(seed);
		};
		void Seed(u_int32_t seed)
		{
			engine.seed(seed);
		};


		//basically no overhead for using this construction
		int UniformInteger(int min,int max)
		{
			return std::uniform_int_distribution<int>(min,max)(engine);
		};
		double UniformDouble(double min, double max)
		{
			return std::uniform_real_distribution<double>(min,max)(engine);
		};

		//2x speedup for using a transform here.
		double Normal(double mu, double sigma)
		{
			return mu + sigma * norm(engine);
		};
};