// The Write command and WriteMetaData structure are (verbatim, aside from some name changes) derived from 
// the `tar_to_stream` project by Armchair-Software, available at:
// https://github.com/Armchair-Software/tar_to_stream/
//
// The original code is provided under the MIT License (https://opensource.org/licenses/MIT).
// The Read functions and object-oriented implementation were written by JFG-2025.

#pragma once
#include <span>
#include <string_view>
#include <iostream>
#include <fstream>
#include <charconv>
#include <sstream>
#include <string>
#include <unordered_map>
#include <functional>
#include <tuple>
#include <cstring>
#include <stdexcept>
#include "Log.h"
#include "convert.h"
namespace JAR
{
	struct ReadMetaData
	{
		std::string filename;
		size_t file_size;
		std::streampos data_offset; // Byte offset to the file's data in the archive
	};

	struct WriteMetaData {
		/// Properties of the file to enter into the stream
		std::string const &filename;                                                  /// name of the file to write
		std::span<std::byte const> data;                                              /// the location of the file's contents in memory
		uint64_t mtime{0u};                                                           /// file modification time, in seconds since epoch
		std::string filemode{"644"};                                                  /// file mode
		unsigned int uid{0u};                                                         /// file owner user ID
		unsigned int gid{0u};                                                         /// file owner group ID
		std::string const &uname{"root"};                                             /// file owner username
		std::string const &gname{"root"};                                             /// file owner group name
	};


	class Archive
	{
		private:
			std::fstream Stream;
			std::unordered_map<std::string, ReadMetaData> FileIndex;
			static constexpr size_t BLOCK_SIZE = 512;
			bool IndexBuilt;
			bool HasWritten;
			bool OpenedStream;
			std::ios_base::openmode Mode;
			std::string Name;
			void BuildIndex();
			void OpenStream();
			bool ReadBlock(char*buffer);
			void WriteCleanup(unsigned int tail_length = 512u * 2u);
			bool HasClosed;
			
			//this is templated in case you ever want to stream things directly into non-string lines for whatever reason
			//(i.e. string_views which have less memory overhead, but have to be careful with lifetimes)
			template<class T>
			void StreamBlocks(std::string fileName, std::function<void(T)> data_callback)
			{
				static_assert(std::is_constructible<T, const char*, size_t>::value, "T must be constructible from char* and size_t");
				if (!IndexBuilt || !OpenedStream)
				{
					BuildIndex();
				}

				auto it = FileIndex.find(fileName);
				if (it == FileIndex.end())
				{
					throw std::runtime_error("File not found in archive. File: " + fileName + " Archive: " + Name + "\n");
				}
				ReadMetaData meta = it->second;

				Stream.seekg(meta.data_offset);
				size_t remainingSize = meta.file_size;
				char buffer[10*BLOCK_SIZE];

				while (remainingSize > 0)
				{
					size_t chunk_size = std::min(remainingSize, BLOCK_SIZE);
					Stream.read(buffer, chunk_size);
					data_callback(T(buffer,buffer+chunk_size));
					remainingSize -= chunk_size;
				}
			};
		public:
			Archive();
			Archive(std::string archivePath);
			Archive(std::string archivePath, std::ios_base::openmode mode);

			~Archive();
			std::vector<std::string> ListFiles();
			void Open(std::string archivePath, std::ios_base::openmode mode);
			void Write(WriteMetaData &&input);
			void Write(const std::string & fileName, const std::string & data);
			std::string GetText(std::string file);

			

			void Close();

			//iterates through the blocks of a file, and calls the callback function *on every complete line*
			template<class T>
			void ForLineIn(const std::string & fileName,std::function<void(T)> perLineFunction)
			{
				//have to be clever because StreamBlocks (and the tar protocol in general) does not guarantee blocks are complete lines
				std::string overflow; //entity for holding overflow from the previous block
				StreamBlocks<std::string>(fileName, [&](const std::string& block) 
				{
					std::string data = overflow + block;
					overflow.clear();

					size_t start = 0;
					size_t end = 0;
					while (end < data.size()) 
					{
						// If no newline is found, is an incomplete line
						//therefore have to break out of loop and save data into the overflow
						end = data.find('\n', start);
						if (end == std::string::npos)
						{
							overflow = data.substr(start);
							break;
						}

						//otherwise we can extract the line and act upon it
						std::string_view line(data.data() + start, end - start);

						perLineFunction(line);
					}
					start = end +1;

				});
			}


			template<typename...ColumnTypes>
			void ForLineInTabularFile(const std::string& fileName, std::vector<std::tuple<ColumnTypes...>>& rows, char delimiter = ' ')
			{
				forLineIn(fileName,[&](std::string_view line){
					// Parse the line into a tuple
					std::tuple<ColumnTypes...> row;
					size_t columnIndex = 0;
					size_t tokenStart = 0;

					// Lambda to parse each token
					auto parseToken = [&](auto& columnValue) {
						size_t tokenEnd = line.find(delimiter, tokenStart);
						if (tokenStart >= line.size()) {
							throw std::runtime_error("Insufficient columns in line: " + std::string(line));
						}

						std::string_view token = line.substr(tokenStart, tokenEnd - tokenStart);
						tokenStart = (tokenEnd == std::string_view::npos) ? std::string_view::npos : tokenEnd + 1;

						columnValue = convert<decltype(columnValue)>(token);

						columnIndex++;
					};

					// Apply the parsing function to each element of the tuple
					std::apply([&](auto&... columnValues) { (parseToken(columnValues), ...); }, row);

					// Ensure there are no extra columns
					if (tokenStart < line.size()) {
						throw std::runtime_error("Too many columns in line: " + std::string(line));
					}

					rows.push_back(std::move(row));

				});
			}
			Archive(const Archive&) = delete;
			Archive& operator=(const Archive&) = delete;
	};
}