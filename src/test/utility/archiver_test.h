#pragma once
#include "../catch_amalgamated.hpp" 
#include "../../utility/Archiver.h" // Adjust path as needed
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <fstream>
#include <stdio.h>
#include "../mock/MockFile.h"
#include "../mock/coutCatch.h"
using namespace Archiver;

// Helper to generate a temporary file path


TEST_CASE("Archive Writing", "[archive][filesystem][utility][write]")
{
	MockFile file;
	auto A = Archive(file.Name(),Write);

	SECTION("Basic writing")
	{
		REQUIRE(std::filesystem::exists(file.Path)); // archive comes into being when written to
		REQUIRE_NOTHROW(A.WriteFile("test-file","some data"));
	}


}

TEST_CASE("Archive Modes detect valid states","[archive][filesystem][utility][errors]")
{
	MockFile file;
	Archive A;
	SECTION("Cannot call Read on write-opened archive")
	{

		REQUIRE_NOTHROW(A.Open(file.Name(),Write));
		auto msg = capture_stdout([&](){REQUIRE_THROWS(A.GetText("non_existant_file.dat"));});
		REQUIRE_THAT(msg,ContainsSubstring("ERROR"));
		REQUIRE_THAT(msg,ContainsSubstring("Cannot call read functions whilst in write mode"));

		//check that empty-written archives are allowed - but with a warning message
		msg = capture_stdout([&](){REQUIRE_NOTHROW(A.Close());});
		REQUIRE(std::filesystem::exists(file.Path));
		REQUIRE_THAT(msg,ContainsSubstring("[WARN]"));
	}
	SECTION("Cannot read non-existant or corrupted file")
	{
		//throws an error because no file found at that location 
		auto msg = capture_stdout([&](){REQUIRE_THROWS(A.Open(file.Name(),Read));});
		REQUIRE_THAT(msg,ContainsSubstring("could not be opened"));

		//file exists, but is not an archive
		file << "non-terminated data";
		msg = capture_stdout([&](){REQUIRE_THROWS(A.Open(file.Name(),Read));});
		REQUIRE_THAT(msg,ContainsSubstring("corrupted"));
		REQUIRE_THAT(msg,ContainsSubstring("[ERROR]"));
	}

	

	SECTION("Cannot call write on read-opened archive")
	{
		Archive B(file.Name(),Write);
		B.WriteFile("test","test data");
		B.Close();
		

		REQUIRE_NOTHROW(A.Open(file.Name(),Read));
		auto msg = capture_stdout([&](){REQUIRE_THROWS(A.ActivateStream("badfile"));});

		REQUIRE_THAT(msg,ContainsSubstring("ERROR"));
		REQUIRE_THAT(msg,ContainsSubstring("Cannot call write functions whilst in read mode"));
	}

	SECTION("Invalid state")
	{
		MockFile file;
		Archive A;
		auto msg = capture_stdout([&](){REQUIRE_THROWS(A.ActivateStream("badfile"));});
		REQUIRE_THAT(msg,ContainsSubstring("Uninitialised"));
		msg = capture_stdout([&](){REQUIRE_THROWS(A.GetText("badfile"));});
		REQUIRE_THAT(msg,ContainsSubstring("Uninitialised"));
	}
}

TEST_CASE("Archive Reading","[archive][filesystem][utility][read]")
{
	MockFile file;
	auto A = Archive(file.Name(),Write);
	int NtestFiles = 5;
	for (int i = 0; i <NtestFiles; ++i)
	{
		A.WriteFile("test-file-" + std::to_string(i+1),"Test_data_" + std::to_string(10*i+5));
	}

	SECTION("Check errors when reading a non-terminated file")
	{
		std::string msg = capture_stdout([&](){REQUIRE_THROWS(Archive((std::string)file.Name(),Read));});

		REQUIRE_THAT(msg,ContainsSubstring("[ERROR]"));
		REQUIRE_THAT(msg,ContainsSubstring("corrupted"));
	}

	A.Close(); //now close it properly

	SECTION("File Listing")
	{

		auto B = Archive((std::string)file.Name(),Read);
		std::vector<std::string> filelist = B.ListFiles();
		for (int i = 0; i <NtestFiles; ++i)
		{
			std::string expectedFile = "test-file-" + std::to_string(i+1);
			REQUIRE_THAT(filelist,Catch::Matchers::VectorContains(expectedFile));


		}
	}

	SECTION("Opening files")
	{
		auto B = Archive((std::string)file.Name(),Read);
		// std::vector<std::string> filelist = B.ListFiles();
		for (int i = 0; i <NtestFiles; ++i)
		{
			std::string expectedFile = "test-file-" + std::to_string(i+1);
			// REQUIRE_THAT(filelist,Catch::Matchers::VectorContains(expectedFile));
			auto extractedText = B.GetText(expectedFile);
			std::string expectedText = "Test_data_" + std::to_string(10*i+5);
			REQUIRE(expectedText == extractedText);


		}
	}
}



TEST_CASE("External tar test","[archive][filesystem][system][utility]")
{
	MockFile file;
	auto A = Archive((std::string)file.Name(),Write);
	int NtestFiles = 5;
	for (int i = 0; i <NtestFiles; ++i)
	{
		A.WriteFile("test-file-" + std::to_string(i+1),"Test_data_" + std::to_string(10*i+5));
	}
	A.Close();

	auto Dir = MockDirectory(file);

	std::string tarCommand = "tar -xf \"" + file.Path.string() + "\" -C " + Dir.Path.string(); // Use quotes for path with spaces
	FILE* pipe = popen(tarCommand.c_str(), "r");
    if (pclose(pipe)!=0) 
	{
        FAIL("Failed to run tar command: " << tarCommand);
    }

	std::vector<bool> found(5,false);
    for (const auto & entry : std::filesystem::directory_iterator(Dir.Path))
	{
		REQUIRE_THAT(entry.path(),ContainsSubstring("test-file"));
		auto final = entry.path().string().back();
		int i = (int)final - '0' -1;
		REQUIRE(i >=0);REQUIRE(i < 5);
		found[i] = true;
		std::string expectedText = "Test_data_" + std::to_string(10*i+5);
		std::ifstream f(entry.path());
		// f.open(f,std::ios::in);
		std::stringstream textFromFile;
		std::string temp;
		while (getline (f, temp)) {
			// Output the text from the file
			textFromFile << temp;
		  }
		  REQUIRE(textFromFile.str() == expectedText);
		
	}
	bool expectedFilesAllPresent = (found == std::vector<bool>(5,true));
	REQUIRE(expectedFilesAllPresent);
}


TEST_CASE("Write using file-streams","[archive][filesystem][system][utility]")
{
	MockFile file;
	Archive A(file.Name(),Write);

	SECTION("Simple writing")
	{
		REQUIRE_NOTHROW(A.ActivateStream("testfile"));
		REQUIRE_NOTHROW(A << "test data");
		A.Close();

		Archive B(file.Name(),Read);
		REQUIRE(B.ListFiles().size() == 1);
		REQUIRE(B.ListFiles()[0] == "testfile");
		REQUIRE(B.GetText("testfile") == "test data");
		
	}

	SECTION("Chained writing")
	{
		A.ActivateStream("testfile") << "test" << " " << "data";
		A << " on \n\t" << 1 << " new " << "line!";
		A.Close();
		Archive B(file.Name(),Read);
		REQUIRE(B.ListFiles().size() == 1);
		REQUIRE(B.ListFiles()[0] == "testfile");
		REQUIRE(B.GetText("testfile") == "test data on \n\t1 new line!");
	}
}

TEST_CASE("Reading templates allow custom interpreters","[archive][filesystem][system][utility]")
{
	MockFile file;
	Archive A(file.Name(),Write);

	for (int j = 0; j < 10; ++j)
	{
		A.ActivateStream("file_"+std::to_string(j+1) + ".tst");
		for (int i = 0; i < 26; ++i)
		{
			A << i << " " << j << " " << char(65+i) << "\n";
		}
		A.DeactivateStream();
	}
	A.Close();

	Archive B(file.Name(),Read);
	std::vector<char> alphabet = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
	SECTION("Basic ForLineIn")
	{
		for (int j = 0; j < 10; ++j)
		{
			int i = 0;
			B.ForLineIn("file_"+std::to_string(j+1) + ".tst",[&](auto line)
			{
				std::string expectedLine = std::to_string(i) + " " + std::to_string(j) + " " + alphabet[i];
				REQUIRE(line == expectedLine);
				++i;
			});
			REQUIRE(i==26);
		}
	}

	SECTION("GetTabular")
	{
		for (int j = 0; j < 10; ++j)
		{
			auto msg = capture_stdout([&](){REQUIRE_THROWS(B.GetTabular<int,double>("file_"+std::to_string(j+1) + ".tst"));});
			REQUIRE_THAT(msg,ContainsSubstring("Token count in vector"));

			std::vector<std::tuple<int,double,char>> grid;
			REQUIRE_NOTHROW(grid = B.GetTabular<int,double,char>("file_"+std::to_string(j+1) + ".tst"));

			REQUIRE(grid.size() == 26);
			for (int i = 0; i < grid.size(); ++i)
			{
				REQUIRE(std::get<0>(grid[i]) == i);
				REQUIRE(std::get<1>(grid[i]) == (double)j);
				REQUIRE(std::get<2>(grid[i]) == alphabet[i]);
			}
		}
	}

	SECTION("forTabularLineIn")
	{
		for (int j = 0; j < 10; ++j)
		{
			int expected_i = 0; // Use a distinct name to avoid shadowing loop var
			B.ForTabularLineIn<int, double, char>("file_" + std::to_string(j + 1) + ".tst", " ",
				[&](std::tuple<int, double, char> row) // Lambda takes a tuple
			{
				REQUIRE(std::get<0>(row) == expected_i);
				REQUIRE(std::get<1>(row) == (double)j);
				REQUIRE(std::get<2>(row) == alphabet[expected_i]);
				expected_i++;
			});
			REQUIRE(expected_i == 26); // Ensure all lines were processed
		}
	}
}