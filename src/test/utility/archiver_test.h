#pragma once
#include "../catch_amalgamated.hpp" 
#include "../../utility/Archiver.h" // Adjust path as needed
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem> // C++17 for temporary paths and removal
#include <fstream>
#include <stdio.h>
using namespace JAR; // Use your namespace

// Helper to generate a temporary file path
int s = 0;
std::filesystem::path makeTemp() {
    s+=1;
	return std::filesystem::temp_directory_path() / ("test_archive_" + std::to_string(s) + ".test"); 
}

struct MockFile
{
	std::filesystem::path Path;
	MockFile()
	{
		Path = makeTemp();
	}
	std::string Name()
	{
		return (std::string)Path;
	}
	~MockFile()
	{
		std::filesystem::remove_all(Path);
	}
};

class MockDirectory {
	public:
		std::filesystem::path Path;
		MockDirectory(MockFile & file) {
			std::filesystem::path parent = file.Path.parent_path();

			// Get the filename without the extension ("file")
			std::filesystem::path stem = file.Path.stem();
		
			// Combine the parent path and the stem to create the new path
			Path  = parent / stem;

			std::filesystem::create_directories(Path);
		}
		~MockDirectory() {
			std::error_code ec; // Use error code version to avoid throwing from destructor
			std::filesystem::remove_all(Path, ec);
			if (ec) {
				// Optionally log the error if removal fails
				std::cerr << "Error removing temporary directory " << Path << ": " << ec.message() << std::endl;
			}
		}
	};

TEST_CASE("Archive Writing", "[archive][filesystem]")
{
	MockFile file;
	auto A = JAR::Archive(file.Name(),std::ios::out);
	CHECK_FALSE(std::filesystem::exists(file.Path)); // archive doesn't exist until something is written to it
	REQUIRE_NOTHROW(A.Write("test-file","some data"));
	CHECK(std::filesystem::exists(file.Path));
	// std::filesystem::remove_all(file);
}


TEST_CASE("Archive Reading","[archive][filesystem]")
{
	MockFile file;
	auto A = JAR::Archive((std::string)file.Name(),std::ios::out);
	int NtestFiles = 5;
	for (int i = 0; i <NtestFiles; ++i)
	{
		A.Write("test-file-" + std::to_string(i+1),"Test_data_" + std::to_string(10*i+5));
	}
	A.Close();

	SECTION("File Listing")
	{

		auto B = JAR::Archive((std::string)file.Name(),std::ios::in);
		std::vector<std::string> filelist = B.ListFiles();
		for (int i = 0; i <NtestFiles; ++i)
		{
			std::string expectedFile = "test-file-" + std::to_string(i+1);
			REQUIRE_THAT(filelist,Catch::Matchers::VectorContains(expectedFile));


		}
	}

	SECTION("Opening files")
	{
		auto B = JAR::Archive((std::string)file.Name(),std::ios::in);
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



TEST_CASE("External tar test","[archive][filesystem][system]")
{
	MockFile file;
	auto A = JAR::Archive((std::string)file.Name(),std::ios::out);
	int NtestFiles = 5;
	for (int i = 0; i <NtestFiles; ++i)
	{
		A.Write("test-file-" + std::to_string(i+1),"Test_data_" + std::to_string(10*i+5));
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
