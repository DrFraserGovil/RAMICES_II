#pragma once
#include <filesystem> // C++17 for temporary paths and removal
#include <iostream>
extern int MockFileID;
std::filesystem::path inline makeTemp() {
    ++MockFileID;
	return std::filesystem::temp_directory_path() / ("test_archive_" + std::to_string(MockFileID) + ".test"); 
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