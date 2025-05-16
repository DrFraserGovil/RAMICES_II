#pragma once
#include "catch_amalgamated.hpp"
#include "../utility/Log.h"


// Helper function to capture stdout
std::string capture_stdout(std::function<void()> func) {
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

TEST_CASE("Logger", "[log]") {
    ConfigObject cfg;

    SECTION("Log Configuration Object") 
	{
		SECTION("Default Construction")
		{
			//Check that the default values are as assumed    
			REQUIRE(cfg.Level == INFO);
			REQUIRE(cfg.ShowHeaders == true);
			REQUIRE(cfg.AppendNewline == true);
		}
		
		SECTION("SetLevel(int)") 
		{
			//Check the integer assignment 
			cfg.SetLevel(0); REQUIRE(cfg.Level == ERROR);
			cfg.SetLevel(1); REQUIRE(cfg.Level == WARN);
			cfg.SetLevel(2); REQUIRE(cfg.Level == INFO);
			cfg.SetLevel(3); REQUIRE(cfg.Level == DEBUG);
	
			REQUIRE_THROWS_AS(cfg.SetLevel(4), std::runtime_error);
			REQUIRE_THROWS_AS(cfg.SetLevel(-1), std::runtime_error);
		}

		SECTION("SetLevel(LogLevel)") {
			cfg.SetLevel(INFO); REQUIRE(cfg.Level == INFO);
			cfg.SetLevel(ERROR); REQUIRE(cfg.Level == ERROR);
			cfg.SetLevel(WARN); REQUIRE(cfg.Level == WARN);
			cfg.SetLevel(DEBUG); REQUIRE(cfg.Level == DEBUG);
		}
	
		SECTION("SetHeader") {
			cfg.SetHeader(false); REQUIRE(cfg.ShowHeaders == false);
			cfg.SetHeader(true); REQUIRE(cfg.ShowHeaders == true);
		}

		SECTION("SetNewline")
		{
			cfg.SetNewline(true); REQUIRE(cfg.AppendNewline == true);
			cfg.SetNewline(false); REQUIRE(cfg.AppendNewline == false);
		}
    }
	using namespace Catch::Matchers;
	SECTION("Core Logger Output")
	{
		// Control the global config object for testing
		ConfigObject originalConfig = LogConfig; // Save original config
		
		LogConfig.TerminalOutput = false; // Assume terminal for colored tests
		LogConfig.SetHeader(false);
		
		SECTION("Check Logger Text (with newlines)") {


			std::string output = capture_stdout([&]() {
				(LoggerCore(DEBUG,0,"mock-function","mock-file")) << "Debug message";
			});
			REQUIRE(output == "Debug message\n");

			LogConfig.SetNewline(false);
			std::string noLinebreakOutput = capture_stdout([&]() {
				(LoggerCore(DEBUG,0,"mock-function","mock-file")) << "Debug message";
			});
			REQUIRE(noLinebreakOutput == "Debug message"); // as above, but with no linebreak
		}

		SECTION("Check ANSI code insertion")
		{
			auto r = {DEBUG,INFO,WARN,ERROR};
			for (LogLevel level: r)
			{
				LogConfig.TerminalOutput = true;
				std::string terminalOutput = capture_stdout([&]() {
					(LoggerCore(level,0,"mock-function","mock-file")) << "Debug message";
				});
				REQUIRE_THAT(terminalOutput, ContainsSubstring("\033[3")); //check that a (non-default) ANSI codes is present during the 
				REQUIRE_THAT(terminalOutput, EndsWith("\033[0m\n"));  //check that the default ANSI code (white) is inserted at the end

				LogConfig.TerminalOutput = false;
				std::string fileOutput = capture_stdout([&]() {
					(LoggerCore(DEBUG,0,"mock-function","mock-file")) << "Debug message";
				});
				REQUIRE_THAT(fileOutput,!ContainsSubstring("\033[3")); //check the above tests fail when terminal output deactivated
				REQUIRE_THAT(fileOutput, !EndsWith("\033[0m\n"));
			}
		}
		
		SECTION("Source information inserted")
		{
			auto r = {DEBUG,INFO,WARN,ERROR};
			std::vector<std::string> names = {"DEBUG","INFO","WARN","ERROR"};
			int i = 0;
			for (LogLevel level: r)
			{
				LogConfig.TerminalOutput = false;
				LogConfig.AppendNewline= false;
				std::string name = "mock-" + names[i] + "-";
				++i;
				std::string variedLevelOutput = capture_stdout([&]() {
					(LoggerCore(level,398,name + "function",name+"file")) << "Debug message";
				});
				if (level <= 1)
				{
					REQUIRE_THAT(variedLevelOutput,ContainsSubstring(name+"function"));
					REQUIRE_THAT(variedLevelOutput,ContainsSubstring("398"));
					REQUIRE_THAT(variedLevelOutput,ContainsSubstring(name+"file"));
				}
				else
				{
					REQUIRE_THAT(variedLevelOutput,!ContainsSubstring(name+"function"));
					REQUIRE_THAT(variedLevelOutput,!ContainsSubstring("398"));
					REQUIRE_THAT(variedLevelOutput,!ContainsSubstring(name+"file"));
				}

			}
		}

		SECTION("Check headers")
		{
			auto r = {DEBUG,INFO,WARN,ERROR};
			std::vector<std::string> names = {"DEBUG","INFO","WARN","ERROR"};
			int i = 0;
			LogConfig.TerminalOutput = false;
			LogConfig.AppendNewline= false;
			for (LogLevel level: r)
			{
				LogConfig.SetHeader(true);
				std::string headerPresent = capture_stdout([&]() {
					(LoggerCore(level,0,"mock-function","mock-file")) << "Debug message";
				});
				std::string search = "[" + names[i] + "]";
				REQUIRE_THAT(headerPresent,ContainsSubstring(search));//check that the header successfully inserted when in true mode
				++i;

				LogConfig.SetHeader(false);
				std::string headerAbsent = capture_stdout([&]() {
					(LoggerCore(level,0,"mock-function","mock-file")) << "Debug message";
				});
				REQUIRE_THAT(headerAbsent, !ContainsSubstring(search)); // check that it's absent in false mode
			}
		}

		SECTION("Empty log")
		{
			std::string output = capture_stdout([&]() {
				(LoggerCore(DEBUG,0,"mock-function","mock-file"));
				REQUIRE(output.empty());
			});
		}

		LogConfig = originalConfig;
	}	
}
