#pragma once
#include "catch_amalgamated.hpp"
#include "mock/mockObjects.h"
class VectorAbsMatcher : public Catch::Matchers::MatcherBase<std::vector<double>> 
{
    std::vector<double> m_target;
    double m_tolerance;
    mutable std::string m_failureReason; // To store a detailed reason for failure

	public:
		// Constructor to initialize the matcher with the expected values and tolerance
		VectorAbsMatcher(const std::vector<double>& target, double tolerance)
			: m_target(target), m_tolerance(tolerance) {}

		// The core comparison logic
		bool match(const std::vector<double>& actual) const override {
			if (actual.size() != m_target.size()) {
				std::ostringstream oss;
				oss << "Vectors have different sizes. Expected " << m_target.size()
					<< " but got " << actual.size() << ".";
				m_failureReason = oss.str();
				return false;
			}

			for (size_t i = 0; i < actual.size(); ++i) {
				if (std::fabs(actual[i] - m_target[i]) > m_tolerance) {
					std::ostringstream oss;
					oss << "Elements at index " << i << " differ by more than tolerance."
						<< " Actual: " << actual[i] << "\n Expected: " << m_target[i]
						<< ", Difference: " << std::fabs(actual[i] - m_target[i])
						<< ", Tolerance: " << m_tolerance << ".";
					m_failureReason = oss.str();
					return false;
				}
			}
			return true;
		}

		// Description generated if the assertion fails
		std::string describe() const override {
			std::ostringstream oss;
			oss << "is approximately equal to {";
			for (size_t i = 0; i < m_target.size(); ++i) {
				oss << m_target[i] << (i == m_target.size() - 1 ? "" : ", ");
			}
			oss << "} with absolute tolerance " << m_tolerance << ". ";
			if (!m_failureReason.empty()) {
				oss << m_failureReason;
			}
			return oss.str();
		}
};

VectorAbsMatcher inline WithinVectorAbs(const std::vector<double> & target,double margin)
{
	return VectorAbsMatcher(target,margin);
}


#define REQUIRE_APPROX(argument,value) REQUIRE_THAT(argument, WithinAbs(value,1e-8))
#define REQUIRE_VEC_APPROX(argument,value) REQUIRE_THAT(argument, WithinVectorAbs(value,1e-8))

// #define _internal_catch_throw_macro(...)
#define REQUIRE_ERROR(...) \
    [&]() -> std::string { \
        std::string captured_msg = capture_stdout([&]() {REQUIRE_THROWS(__VA_ARGS__); }); \
        REQUIRE_THAT(captured_msg, Catch::Matchers::ContainsSubstring("[ERROR]")); \
        return captured_msg; \
    }() // Immediately invoke the lambda

#define REQUIRE_WARN(...) \
    [&]() -> std::string { \
        std::string captured_msg = capture_stdout([&]() { __VA_ARGS__; }); \
        REQUIRE_THAT(captured_msg, Catch::Matchers::ContainsSubstring("[WARN]")); \
        return captured_msg; \
    }() // Immediately invoke the lambda

#define REQUIRE_NO_WARN(...) \
    [&]() -> std::string { \
        std::string captured_msg = capture_stdout([&]() { __VA_ARGS__; }); \
        REQUIRE_THAT(captured_msg, !Catch::Matchers::ContainsSubstring("[WARN]")); \
        return captured_msg; \
    }() // Immediately invoke the lambda