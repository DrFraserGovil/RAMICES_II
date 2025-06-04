#pragma once
#include <string>
#include <functional>
#include <sstream>
#include <iostream>


// Helper function to capture stdout
std::string capture_stdout(std::function<void()> func)
{
    std::stringstream ss;
    // Save the original buffer
    std::streambuf* old_buf = std::cout.rdbuf();
    // Redirect cout to the stringstream buffer
    std::cout.rdbuf(ss.rdbuf());
    // Call the function that produces output
    func();
    // Restore the original buffer
    std::cout.rdbuf(old_buf);
    // Return the captured string
    return ss.str();
}




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