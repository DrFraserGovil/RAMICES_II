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