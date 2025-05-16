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

//     std::filesystem::path test_archive_path = get_temp_tar_path();

//     // Ensure the test file is removed after the section finishes
//     // Catch::ScopeGuard cleanup = [&]() {
//     //     std::filesystem::remove(test_archive_path);
//     // };

//     SECTION("Write a single small file") {
//         Archive archive_writer(test_archive_path, std::ios::out);

//         std::string file_name = "hello.txt";
//         std::string file_content = "Hello, TAR!";
//         std::vector<std::byte> file_data(reinterpret_cast<const std::byte*>(file_content.data()), reinterpret_cast<const std::byte*>(file_content.data()) + file_content.size());

//         WriteMetaData metadata = {
//             .filename = file_name,
//             .data = file_data
//         };

//         REQUIRE_NOTHROW(archive_writer.Write(std::move(metadata)));

//         // Destruct the archive_writer to trigger WriteCleanup
//         // The file is written when archive_writer goes out of scope

//         archive_writer.~Archive(); // Explicitly call destructor for testing flow (or just let it go out of scope)

//         // --- Verification ---
//         std::vector<char> buffer = read_file_to_buffer(test_archive_path);

//         // Expected size: 512 (header) + data + padding + 1024 (tail)
//         size_t expected_data_size = file_content.size();
//         size_t padding_size = (512u - expected_data_size % 512) & 511u;
//         size_t expected_total_size = 512 + expected_data_size + padding_size + 1024; // 1024 for default WriteCleanup tail

//         REQUIRE(buffer.size() == expected_total_size);

//         // Verify Header
//         const char* header_buffer = buffer.data();
//         REQUIRE(std::string(header_buffer, file_name.size()) == file_name); // Check filename prefix
//         REQUIRE(header_buffer[99] == '\0'); // Check filename null termination

//         REQUIRE(parse_tar_octal_field(header_buffer, 124, 11) == expected_data_size); // Check size field
//         REQUIRE(header_buffer[135] == '\0'); // Check size null termination

//         REQUIRE(header_buffer[156] == '0'); // Check typeflag

//         // Verify Checksum
//         unsigned int recorded_checksum = parse_tar_octal_field(header_buffer, 148, 6); // Read checksum from header
//         unsigned int calculated_checksum = calculate_tar_checksum(header_buffer); // Calculate checksum from buffer
//         REQUIRE(recorded_checksum == calculated_checksum);

//         // Verify Data
//         const char* data_buffer = header_buffer + 512;
//         REQUIRE(std::string(data_buffer, expected_data_size) == file_content);

//         // Verify Padding
//         const char* padding_buffer = data_buffer + expected_data_size;
//         for(size_t i = 0; i < padding_size; ++i) {
//             REQUIRE(padding_buffer[i] == '\0');
//         }

//         // Verify Tail (two blocks of zeros)
//         const char* tail_buffer = padding_buffer + padding_size;
//         for(size_t i = 0; i < 1024; ++i) { // Default cleanup tail is 1024 bytes
//             REQUIRE(tail_buffer[i] == '\0');
//         }
//     }

//     SECTION("Write a file exactly 512 bytes") {
//          Archive archive_writer(test_archive_path, std::ios::out);

//         std::string file_name = "exact.bin";
//         std::string file_content(512, 'X'); // Exactly 512 bytes
//         std::vector<std::byte> file_data(reinterpret_cast<const std::byte*>(file_content.data()), reinterpret_cast<const std::byte*>(file_content.data()) + file_content.size());

//         WriteMetaData metadata = {
//             .filename = file_name,
//             .data = file_data
//         };

//         REQUIRE_NOTHROW(archive_writer.Write(std::move(metadata)));
//         archive_writer.~Archive(); // Trigger WriteCleanup

//         // --- Verification ---
//         std::vector<char> buffer = read_file_to_buffer(test_archive_path);

//         // Expected size: 512 (header) + 512 (data) + 0 (padding) + 1024 (tail)
//         size_t expected_data_size = file_content.size(); // 512
//         size_t padding_size = (512u - expected_data_size % 512) & 511u; // 0
//         size_t expected_total_size = 512 + expected_data_size + padding_size + 1024;

//         REQUIRE(buffer.size() == expected_total_size);

//         // Verify Header (similar checks as above)
//         const char* header_buffer = buffer.data();
//         REQUIRE(std::string(header_buffer, file_name.size()) == file_name);
//         REQUIRE(parse_tar_octal_field(header_buffer, 124, 11) == expected_data_size);
//         // ... checksum verification ...

//         // Verify Data
//         const char* data_buffer = header_buffer + 512;
//         REQUIRE(std::string(data_buffer, expected_data_size) == file_content);

//         // Verify Padding (should be zero bytes)
//         REQUIRE(padding_size == 0);

//         // Verify Tail
//          const char* tail_buffer = data_buffer + expected_data_size;
//          for(size_t i = 0; i < 1024; ++i) {
//              REQUIRE(tail_buffer[i] == '\0');
//          }
//     }

//      SECTION("Write multiple files") {
//         Archive archive_writer(test_archive_path, std::ios::out);

//         std::string file1_name = "file1.txt";
//         std::string file1_content = "Content 1";
//         std::vector<std::byte> file1_data(reinterpret_cast<const std::byte*>(file1_content.data()), reinterpret_cast<const std::byte*>(file1_content.data()) + file1_content.size());

//         std::string file2_name = "data.bin";
//         std::string file2_content(600, 'Y'); // Data requiring padding
//         std::vector<std::byte> file2_data(reinterpret_cast<const std::byte*>(file2_content.data()), reinterpret_cast<const std::byte*>(file2_content.data()) + file2_content.size());

//         REQUIRE_NOTHROW(archive_writer.Write({.filename = file1_name, .data = file1_data}));
//         REQUIRE_NOTHROW(archive_writer.Write({.filename = file2_name, .data = file2_data}));

//         archive_writer.~Archive(); // Trigger WriteCleanup

//         // --- Verification ---
//         std::vector<char> buffer = read_file_to_buffer(test_archive_path);

//         // Manually parse the buffer sequentially to verify headers, data, and padding
//         size_t current_offset = 0;

//         // Verify File 1
//         const char* header1_buffer = buffer.data() + current_offset;
//         REQUIRE(std::string(header1_buffer, file1_name.size()) == file1_name);
//         size_t file1_size = parse_tar_octal_field(header1_buffer, 124, 11);
//         REQUIRE(file1_size == file1_content.size());
//         // ... Verify checksum for header1_buffer ...
//         current_offset += 512; // Skip header 1

//         const char* data1_buffer = buffer.data() + current_offset;
//         REQUIRE(std::string(data1_buffer, file1_size) == file1_content);
//         current_offset += file1_size;

//         size_t file1_padding = (512u - file1_size % 512) & 511u;
//          const char* padding1_buffer = buffer.data() + current_offset;
//          for(size_t i = 0; i < file1_padding; ++i) {
//              REQUIRE(padding1_buffer[i] == '\0');
//          }
//         current_offset += file1_padding;

//         // Verify File 2
//         const char* header2_buffer = buffer.data() + current_offset;
//         REQUIRE(std::string(header2_buffer, file2_name.size()) == file2_name);
//         size_t file2_size = parse_tar_octal_field(header2_buffer, 124, 11);
//         REQUIRE(file2_size == file2_content.size());
//         // ... Verify checksum for header2_buffer ...
//          current_offset += 512; // Skip header 2

//         const char* data2_buffer = buffer.data() + current_offset;
//         REQUIRE(std::string(data2_buffer, file2_size) == file2_content);
//         current_offset += file2_size;

//         size_t file2_padding = (512u - file2_size % 512) & 511u;
//         const char* padding2_buffer = buffer.data() + current_offset;
//         for(size_t i = 0; i < file2_padding; ++i) {
//             REQUIRE(padding2_buffer[i] == '\0');
//         }
//         current_offset += file2_padding;

//         // Verify Tail
//         const char* tail_buffer = buffer.data() + current_offset;
//         for(size_t i = 0; i < 1024; ++i) { // Default cleanup tail
//             REQUIRE(tail_buffer[i] == '\0');
//         }
//          current_offset += 1024;

//          REQUIRE(current_offset == buffer.size()); // Ensure all bytes were accounted for
//     }

//     // TODO: Add tests for different WriteMetaData parameters (mtime, uid, gid, etc.)
//     // TODO: Add tests for Write(const std::string&, const std::string&) overload


// }

// // Helper to create a simple test archive for reading tests
// std::filesystem::path create_simple_read_archive(const std::filesystem::path& path) {
//      Archive archive_writer(path, std::ios::out);

//     std::string file1_name = "read_test1.txt";
//     std::string file1_content = "This is the content of the first file.\nIt has two lines.";
//     std::vector<std::byte> file1_data(reinterpret_cast<const std::byte*>(file1_content.data()), reinterpret_cast<const std::byte*>(file1_content.data()) + file1_content.size());

//     std::string file2_name = "read_test2.txt";
//     std::string file2_content = "Second file content.";
//     std::vector<std::byte> file2_data(reinterpret_cast<const std::byte*>(file2_content.data()), reinterpret_cast<const std::byte*>(file2_content.data()) + file2_content.size());

//      archive_writer.Write({.filename = file1_name, .data = file1_data});
//      archive_writer.Write({.filename = file2_name, .data = file2_data});

//     archive_writer.~Archive(); // Trigger WriteCleanup
//     return path;
// }


// TEST_CASE("Archive Reading", "[archive][read]") {
//     std::filesystem::path test_archive_path = get_temp_tar_path();
//     create_simple_read_archive(test_archive_path); // Create the archive for reading tests

//     // Ensure the test file is removed after the section finishes
//     // Catch::ScopeGuard cleanup = [&]() {
//     //     std::filesystem::remove(test_archive_path);
//     // };

//     SECTION("BuildIndex and ListFiles") {
//         Archive archive_reader(test_archive_path, std::ios::in);

//         REQUIRE_NOTHROW(archive_reader.ListFiles()); // ListFiles triggers BuildIndex

//         std::vector<std::string> filenames = archive_reader.ListFiles(); // Call again to check cached index

//         REQUIRE(filenames.size() == 2);
//         // Check if filenames are in the list (order might not be guaranteed by unordered_map)
//         bool found1 = false, found2 = false;
//         for(const auto& name : filenames) {
//             if (name == "read_test1.txt") found1 = true;
//             if (name == "read_test2.txt") found2 = true;
//         }
//         REQUIRE(found1);
//         REQUIRE(found2);

//         // TODO: If FileIndex were accessible (e.g., via a test-only helper or friend class),
//         // you could verify the ReadMetaData (size, data_offset) here.
//     }

//     SECTION("Text extraction") {
//         Archive archive_reader(test_archive_path, std::ios::in);

//         std::string content1;
//         REQUIRE_NOTHROW(content1 = archive_reader.Text("read_test1.txt"));
//         REQUIRE(content1 == "This is the content of the first file.\nIt has two lines.");

//         std::string content2;
//         REQUIRE_NOTHROW(content2 = archive_reader.Text("read_test2.txt"));
//         REQUIRE(content2 == "Second file content.");

//         // Test extracting a non-existent file
//         REQUIRE_THROWS_AS(archive_reader.Text("non_existent.txt"), std::runtime_error);
//     }

//     SECTION("StreamFile extraction") {
//         Archive archive_reader(test_archive_path, std::ios::in);

//         std::string streamed_content1;
//         REQUIRE_NOTHROW(archive_reader.StreamFile<std::string>("read_test1.txt", [&](const std::string& block){
//             streamed_content1 += block; // Append blocks
//         }));
//         REQUIRE(streamed_content1 == "This is the content of the first file.\nIt has two lines.");

//          std::vector<char> streamed_content2_buffer;
//         REQUIRE_NOTHROW(archive_reader.StreamFile<std::vector<char>>("read_test2.txt", [&](const std::vector<char>& block_vec){
//             streamed_content2_buffer.insert(streamed_content2_buffer.end(), block_vec.begin(), block_vec.end()); // Append block vector
//         }));
//         REQUIRE(std::string(streamed_content2_buffer.begin(), streamed_content2_buffer.end()) == "Second file content.");

//          // Test streaming a non-existent file
//         REQUIRE_THROWS_AS(archive_reader.StreamFile<std::string>("non_existent.txt", [&](const std::string&){}), std::runtime_error);

//          // TODO: Test StreamFile with a file size > BLOCK_SIZE to ensure chunking works.
//     }

//      TEST_CASE("Archive ReadTabular", "[archive][read][tabular]") {
//         std::filesystem::path test_archive_path = get_temp_tar_path();

//         // Create an archive with a tabular file
//         std::string tabular_content = "1 true 3.14 hello\n2 false 6.28 world\n";
//         std::vector<std::byte> tabular_data(reinterpret_cast<const std::byte*>(tabular_content.data()), reinterpret_cast<const std::byte*>(tabular_content.data()) + tabular_content.size());

//         Archive archive_writer(test_archive_path, std::ios::out);
//         archive_writer.Write({.filename = "data.tsv", .data = tabular_data});
//         archive_writer.~Archive(); // Trigger cleanup

//         // Catch::ScopeGuard cleanup = [&]() {
//         //     std::filesystem::remove(test_archive_path);
//         // };

//         SECTION("ReadTabular basic parsing") {
//             Archive archive_reader(test_archive_path, std::ios::in);

//             std::vector<std::tuple<int, bool, double, std::string>> rows;
//             REQUIRE_NOTHROW(archive_reader.ReadTabular("data.tsv", rows, ' '));

//             REQUIRE(rows.size() == 2);

//             // Verify row 1
//             REQUIRE(std::get(rows[0]) == 1);
//             REQUIRE(std::get(rows[0]) == true); // Note: stob for bool not standard, might read "true" as 0? Need to check behavior. std::stod might read 3.14 correctly. String should be fine.
//             REQUIRE(std::get(rows[0]) == Approx(3.14)); // Use Approx for floating points
//             REQUIRE(std::get(rows[0]) == "hello");

//              // Verify row 2
//             REQUIRE(std::get(rows[1]) == 2);
//              // REQUIRE(std::get(rows[1]) == false); // Test parsing "false"
//              // REQUIRE(std::get(rows[1]) == Approx(6.28)); // Test parsing 6.28
//              // REQUIRE(std::get(rows[1]) == "world");

//              // TODO: Test parsing bools more reliably if stob is not used/reliable. Maybe parse bools as strings "true"/"false" and convert manually.
//         }

//          SECTION("ReadTabular error cases") {
//              // Create an archive with malformed tabular data
//              std::string malformed_content = "1 true 3.14\n2 false\n"; // Missing column in second row
//              std::vector<std::byte> malformed_data(reinterpret_cast<const std::byte*>(malformed_content.data()), reinterpret_cast<const std::byte*>(malformed_content.data()) + malformed_content.size());
//              std::filesystem::path malformed_archive_path = get_temp_tar_path();
//              Archive malformed_writer(malformed_archive_path, std::ios::out);
//              malformed_writer.Write({.filename = "malformed.tsv", .data = malformed_data});
//              malformed_writer.~Archive();
//              Catch::ScopeGuard malformed_cleanup = [&]() { std::filesystem::remove(malformed_archive_path); };

//              Archive malformed_reader(malformed_archive_path, std::ios::in);
//              std::vector<std::tuple<int, bool, double, std::string>> rows;

//              // Test insufficient columns
//              REQUIRE_THROWS_AS(malformed_reader.ReadTabular("malformed.tsv", rows, ' '), std::runtime_error);

//              // TODO: Add test for too many columns
//              // TODO: Add test for invalid integer/double format in column
//              // TODO: Test non-existent file (already covered by Text/StreamFile, but good to be explicit)
//          }
//      }

//      TEST_CASE("Archive State and Error Handling", "[archive][state]") {
//          std::filesystem::path test_archive_path = get_temp_tar_path();

//          // No cleanup needed for this specific test case as we test failure scenarios

//          SECTION("Open non-existent file in read mode") {
//              // Opening non-existent file in read mode typically *succeeds* but is_open() is false
//              // Your OpenStream should throw.
//              REQUIRE_THROWS_AS(Archive reader(test_archive_path, std::ios::in), std::runtime_error);
//              // Note: Your current constructor doesn't open the stream, OpenStream does.
//              // So test should call a method that triggers OpenStream. ListFiles is good.
//              Archive reader(test_archive_path, std::ios::in);
//              REQUIRE_THROWS_AS(reader.ListFiles(), std::runtime_error); // ListFiles triggers OpenStream
//          }

//          SECTION("Write to file opened in read mode (should fail)") {
//               // Create an empty file first
//              std::ofstream empty_file(test_archive_path);
//              empty_file.close();
//             //  Catch::ScopeGuard cleanup = [&]() { std::filesystem::remove(test_archive_path); };


//              Archive reader(test_archive_path, std::ios::in); // Open read-only
//              std::string file_name = "should_fail.txt";
//              std::string file_content = "fail";
//              std::vector<std::byte> file_data(reinterpret_cast<const std::byte*>(file_content.data()), reinterpret_cast<const std::byte*>(file_content.data()) + file_content.size());

//              // WriteMetaData metadata = {.filename = file_name, .data = file_data};
//              // Writing to a fstream opened in ios::in should typically fail stream operations
//              // You might not get an exception from Write itself, but stream flags should be bad.

//              // This test might be tricky - how to check for stream failure inside Write?
//              // You could test that calling Write doesn't *corrupt* memory, or check stream state after calling Write (if stream were accessible).
//              // Or test that reading after attempting to write to a read-only archive is unaffected/fails appropriately.

//              // A simpler test is:
//              Archive read_only_archive(test_archive_path, std::ios::in);
//              std::string test_content = "test";
//               std::vector<std::byte> test_data(reinterpret_cast<const std::byte*>(test_content.data()), reinterpret_cast<const std::byte*>(test_content.data()) + test_content.size());

//               // Calling write should not succeed or should set error flags
//               // It won't throw unless you add explicit checks in Write
//               read_only_archive.Write({.filename = "fail.txt", .data = test_data});
//               // Hard to assert failure here without access to stream state or added checks.
//               // Let's focus on error paths you *do* handle (missing file, malformed data parsing).
//          }

//          // TODO: Test reading from an archive opened in write mode (should fail)
//          // TODO: Test BuildIndex on a truncated or malformed archive (should handle gracefully or throw)
//          // TODO: Test calling OpenStream() multiple times
//      }