#pragma once
#include "../../utility/Random.h" // Path to your Random.h

#include <vector>
#include <numeric> // For std::accumulate or similar if needed

// Helper function to compare sequences (optional, can do inline)
template<typename T>
bool SequencesEqual(const std::vector<T>& seq1, const std::vector<T>& seq2) {
    return seq1.size() == seq2.size() && std::equal(seq1.begin(), seq1.end(), seq2.begin());
}

TEST_CASE("Random class seeding and bounds", "[random][utility]") {

    // A fixed seed for reproducible tests
    const uint32_t test_seed = 12345;
    const int num_samples = 100; // Number of samples to generate for sequence/bounds checks

    SECTION("Seeding with the same value produces the same sequence") {
        Random<> r1(test_seed);
        Random<> r2(test_seed);

        std::vector<int> sameSeed_int1;
        std::vector<int> sameSeed_int2;
        for(int i = 0; i < num_samples; ++i) {
            sameSeed_int1.push_back(r1.UniformInteger(0, 100));
            sameSeed_int2.push_back(r2.UniformInteger(0, 100));
        }
        CHECK(SequencesEqual(sameSeed_int1, sameSeed_int2));
        for (int i = 0; i <num_samples;++i)
        {
            REQUIRE(sameSeed_int1[i]==sameSeed_int2[i]);
        }
        std::vector<double> sameSeed_double1;
        std::vector<double> sameSeed_double2;
        for(int i = 0; i < num_samples; ++i) {
            sameSeed_double1.push_back(r1.UniformDouble(0.0, 1.0));
            sameSeed_double2.push_back(r2.UniformDouble(0.0, 1.0));
        }
        REQUIRE(SequencesEqual(sameSeed_double1, sameSeed_double2));

         std::vector<double> sameSeed_normal1;
        std::vector<double> sameSeed_normal2;
        for(int i = 0; i < num_samples; ++i) {
            sameSeed_normal1.push_back(r1.Normal(10.0, 2.0));
            sameSeed_normal2.push_back(r2.Normal(10.0, 2.0));
        }
        REQUIRE(SequencesEqual(sameSeed_normal1, sameSeed_normal2));
    }

     SECTION("Default constructor is seeded differently each time (likely)") {
        // This test is less strict due to random_device variability,
        // but checks that default seeding doesn't produce identical sequences easily.
        Random<> r1_default;
        Random<> r2_default;

        std::vector<int> differentSeed_int1;
        std::vector<int> differentSeed_int2;
        for(int i = 0; i < num_samples; ++i) {
            differentSeed_int1.push_back(r1_default.UniformInteger(0, 100));
            differentSeed_int2.push_back(r2_default.UniformInteger(0, 100));
        }
        // With high probability, sequences from default-seeded engines should not be equal
        REQUIRE_FALSE(SequencesEqual(differentSeed_int1, differentSeed_int2));
    }


    SECTION("UniformInteger respects min and max bounds") {
        Random<> r(test_seed);
        int min_val = -50;
        int max_val = 150;
        for(int i = 0; i < num_samples * 10; ++i) { // Use more samples for range check
            int value = r.UniformInteger(min_val, max_val);
            REQUIRE(value >= min_val);
            REQUIRE(value <= max_val);
        }
         // Test edge case where min == max
         int edge_val = r.UniformInteger(10, 10);
         REQUIRE(edge_val == 10);
    }

    SECTION("UniformDouble respects min and max bounds") {
        Random<> r(test_seed);
        double min_val = -10.5;
        double max_val = 20.5;
        for(int i = 0; i < num_samples * 10; ++i) { // Use more samples
            double value = r.UniformDouble(min_val, max_val);
            REQUIRE(value >= min_val);
            REQUIRE(value <= max_val);
        }
        // Test edge case where min == max
         double edge_val_double = r.UniformDouble(3.14, 3.14);
         REQUIRE(edge_val_double == 3.14);
    }

    // Note: Testing Normal distribution bounds is not feasible as it's theoretically infinite.
    // Testing its parameters (mu, sigma) would require statistical tests, which are not unit tests.
    // Testing that it *runs* and produces *some* double value is implicitly done by the seeding test.

}
