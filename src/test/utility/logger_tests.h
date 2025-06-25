#pragma once
#include "../../utility/Log.h"
#include "../mock/coutCatch.h"

using namespace Catch::Matchers;
TEST_CASE("Logger Core", "[log][utility]") {
    GlobalLog::ConfigObject cfg;

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
	
		
    }
	
	SECTION("Core Logger Output")
	{
		// Control the global config object for testing
		auto originalConfig = GlobalLog::Config; // Save original config
		
		GlobalLog::Config.TerminalOutput = false; // Assume terminal for colored tests
		GlobalLog::Config.ShowHeaders = (false);
		
		SECTION("Check Logger Text (with newlines)") {


			std::string output = capture_stdout([&]() {
				(GlobalLog::LoggerCore(DEBUG,0,"mock-function","mock-file")) << "Debug message";
			});
			REQUIRE(output == "Debug message\n");

			GlobalLog::Config.AppendNewline = (false);
			std::string noLinebreakOutput = capture_stdout([&]() {
				(GlobalLog::LoggerCore(DEBUG,0,"mock-function","mock-file")) << "Debug message";
			});
			REQUIRE(noLinebreakOutput == "Debug message"); // as above, but with no linebreak
		}

		SECTION("Check ANSI code insertion")
		{
			auto r = {DEBUG,INFO,WARN,ERROR};
			for (LogLevel level: r)
			{
				GlobalLog::Config.TerminalOutput = true;
				std::string terminalOutput = capture_stdout([&]() {
					(GlobalLog::LoggerCore(level,0,"mock-function","mock-file")) << "Debug message";
				});
				REQUIRE_THAT(terminalOutput, StartsWith("\033[")); //check that an ANSI codes is inserted at the beginning of input. We don't care which -- that's an implementation detail that can be changed
				REQUIRE_THAT(terminalOutput, EndsWith("\033[0m\n"));  //check that the default ANSI code (white) is inserted at the end

				GlobalLog::Config.TerminalOutput = false;
				std::string fileOutput = capture_stdout([&]() {
					(GlobalLog::LoggerCore(DEBUG,0,"mock-function","mock-file")) << "Debug message";
				});
				REQUIRE_THAT(fileOutput,!StartsWith("\033[")); //check the above tests fail when terminal output deactivated
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
				GlobalLog::Config.TerminalOutput = false;
				GlobalLog::Config.AppendNewline= false;
				std::string name = "mock-" + names[i] + "-";
				++i;
				std::string variedLevelOutput = capture_stdout([&]() {
					(GlobalLog::LoggerCore(level,398,name + "function",name+"file")) << "Debug message";
				});
				if (level <= 1)
				{
					REQUIRE_THAT(variedLevelOutput,ContainsSubstring(name+"function"));
					REQUIRE_THAT(variedLevelOutput,ContainsSubstring("Line 398"));
					REQUIRE_THAT(variedLevelOutput,ContainsSubstring(name+"file"));
				}
				else
				{
					REQUIRE_THAT(variedLevelOutput,!ContainsSubstring(name+"function"));
					REQUIRE_THAT(variedLevelOutput,!ContainsSubstring("Line 398"));
					REQUIRE_THAT(variedLevelOutput,!ContainsSubstring(name+"file"));
				}

			}
		}

		SECTION("Check headers")
		{
			auto r = {DEBUG,INFO,WARN,ERROR};
			std::vector<std::string> names = {"DEBUG","INFO","WARN","ERROR"};
			int i = 0;
			GlobalLog::Config.TerminalOutput = false;
			GlobalLog::Config.AppendNewline= false;
			for (LogLevel level: r)
			{
				GlobalLog::Config.ShowHeaders = (true);
				std::string headerPresent = capture_stdout([&]() {
					(GlobalLog::LoggerCore(level,0,"mock-function","mock-file")) << "Debug message";
				});
				std::string search = "[" + names[i] + "]";
				REQUIRE_THAT(headerPresent,ContainsSubstring(search));//check that the header successfully inserted when in true mode
				++i;

				GlobalLog::Config.ShowHeaders = (false);
				std::string headerAbsent = capture_stdout([&]() {
					(GlobalLog::LoggerCore(level,0,"mock-function","mock-file")) << "Debug message";
				});
				REQUIRE_THAT(headerAbsent, !ContainsSubstring(search)); // check that it's absent in false mode
			}
		}

		SECTION("Empty log")
		{
			std::string output = capture_stdout([&]() {
				(GlobalLog::LoggerCore(DEBUG,0,"mock-function","mock-file"));
			});
			REQUIRE(output.empty());
		}

		GlobalLog::Config = originalConfig;
	}	
}

TEST_CASE("Logger Macro","[log][utility]")
{
	GlobalLog::Config.ShowHeaders = (true);
	GlobalLog::Config.AppendNewline = (false); //set these so no linebreaks in unit test output. Purely for human readability.
	GlobalLog::Config.TerminalOutput = false; // Also supress ANSI codes.
	std::vector<std::string> names = {"ERROR","WARN","INFO","DEBUG"};
	for (int trueLevel = 0; trueLevel < 4; ++trueLevel)
	{
		GlobalLog::Config.SetLevel(trueLevel);

		for (int mockLevel = 0; mockLevel < 4; ++mockLevel)
		{
			auto mock = LogLevelConvert(mockLevel);
			std::string logOutput = capture_stdout([&]() {
				LOG(mock) << "Test log" << " secondary log" << "tertiary log"; // deliberately ruined spaces
			});

			if (mockLevel > trueLevel)
			{
				REQUIRE(logOutput.empty());
			}
			else
			{
				std::string expectedHeader = "[" + names[mockLevel] + "]";
				REQUIRE_THAT(logOutput,ContainsSubstring(expectedHeader));
				std::string expectedBody = "Test log secondary logtertiary log"; //testing the deliberately ruined spaces
				REQUIRE_THAT(logOutput,ContainsSubstring(expectedBody));
			}
		}
	}

	GlobalLog::Config.SetLevel(1);
}