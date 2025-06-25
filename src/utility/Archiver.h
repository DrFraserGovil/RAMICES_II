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
#include <sstream>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <functional>
#include <tuple>
#include <cstring>
#include <stdexcept>
#include "Log.h"
#include "convert.h"


namespace Archiver
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

	//!Custom counterparts to std::ios_base::openmode
	enum ArchiveMode
	{
		Read,

		//
		Write,

		Append,

		//! The state invoked by the default initialiser of the Archive. Archives in this Modes throw an error if used for Reading or Writing. 
		Uninitialised
	};

	class Archive
	{
		private:
			//status indicators
			bool HasWritten;
			bool HasClosed;
			bool FileOpen;
			bool ExpectingEmpty = false;
			bool RequiresDuplicateCleanup;
			std::string OpenFileName;
			std::stringstream FileBuffer;
			//magic numbers
			static constexpr size_t BLOCK_SIZE = 512;

			std::fstream Stream;
			std::unordered_map<std::string, ReadMetaData> FileIndex;
			std::string Name;
			ArchiveMode Mode;
			
			
			//writes the terminating strings to archive
			void OpenForWriting();
			void WriteCleanup(unsigned int tailBlockRepetition= 2u);
			void CheckWriteRegistry(std::string fileName);

			//reading functions
			void OpenForReading();
			void OpenForAppending();
			void BuildIndex();
			bool ReadBlock(char*buffer);
			void StreamBlocks(std::string fileName, std::function<void(std::string)> dataCallback);

			void CheckValidState(ArchiveMode targetState);
		public:
			//Constructors & Initialisers

			Archive();
			Archive(std::string archivePath, ArchiveMode mode = Read);
			void Open(std::string archivePath, ArchiveMode mode);

			//Destructors & cleaners
			~Archive();
			void Close();

			void ExpectEmpty(bool value){ExpectingEmpty = value;};
			void WriteFile(WriteMetaData &&input);
			void WriteFile(const std::string & fileName, const std::string & data);
			std::stringstream & ActivateStream(const std::string & fileName);
			void DeactivateStream();
			template<class T>
			std::stringstream &operator<<(const T &msg)
			{
				CheckValidState(Write);
				FileBuffer << msg;
				return FileBuffer;
			}


			std::vector<std::string> ListFiles();
			std::string GetText(std::string file);

			//iterates through the blocks of a file, and calls the callback function *on every complete line*
			void ForLineIn(const std::string & fileName,std::function<void(std::string_view)> perLineFunction);


			

			template<typename...ColumnTypes, typename TupleFunctor>
			void ForTabularLineIn(const std::string& fileName, std::string delimiter,TupleFunctor perTupleFunction)
			{
				ForLineIn(fileName,[&](std::string_view line){
					
					auto row = convertTuple<ColumnTypes...>(split(line,delimiter));

					perTupleFunction(row);

				});
			}


			template<typename...ColumnTypes>
			std::vector<std::tuple<ColumnTypes...>> GetTabular(const std::string& fileName, std::string delimiter = " ")
			{
				std::vector<std::tuple<ColumnTypes...>> rows;
				ForTabularLineIn<ColumnTypes...>(fileName,delimiter,[&](auto row)
				{
					rows.push_back(row);
				});
				return rows;
			}

			Archive(const Archive&) = delete;
			Archive& operator=(const Archive&) = delete;
	};
}