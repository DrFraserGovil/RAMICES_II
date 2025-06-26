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
	/*
		A container for recording the location of files in an archive stream
	*/
	struct ReadMetaData
	{
		std::string filename; //!< The name of the file - for comparing to user requests
		size_t file_size; //!< The size of the file (in bytes)
		std::streampos data_offset; //!< Byte offset to the file's data in the archive
	};

	/*! 
		@brief Container for Properties of the file to enter into the stream

		@details This is a touch hacky, and contains a number of hardcoded offsets of a certain size, in order to emulate the tar specification, without actually fully implementing it
	*/
	struct WriteMetaData {
		std::string const &filename;                                                  //!< name of the file to write
		std::span<std::byte const> data;                                              //!< the location of the file's contents in memory
		uint64_t mtime{0u};                                                           //!< file modification time, in seconds since epoch
		std::string filemode{"644"};                                                  //!< file mode
		unsigned int uid{0u};                                                         //!< file owner user ID
		unsigned int gid{0u};                                                         //!< file owner group ID
		std::string const &uname{"root"};                                             //!< file owner username
		std::string const &gname{"root"};                                             //!< file owner group name
	};

	//!Custom counterparts to std::ios_base::openmode
	enum ArchiveMode
	{
		//! Puts the Archive into read mode
		Read,

		//! Puts the Archive in write mode. If a file exists with the archive name, it is replaced by a blank archive.
		Write,

		//! Puts the Archive in write mode. If a file exists with the archive name, it is read as an archive, and the contents copied across.
		Append,

		//! The state invoked by the default constructor of the Archive. Archives in this Modes throw an error if used for Reading or Writing. 
		Uninitialised
	};


	/*!
		@brief A class interface for direct reading to and from tar-like entities on disk

		@details In Write mode, an Archive can stream data to individual 'files' within a self-contained archive format, grouping together files for ease of transport and preventing clutter. The Write format mimics the tar standard. The Read mode allows data to be extracted from such archives - including from tar archives created using externally tools, **so long as those archives are uncompressed**.
	*/
	class Archive
	{
		private:
			//status indicators

			//! A magic number associated with the tar block size. DO NOT CHANGE!
			static constexpr size_t BLOCK_SIZE = 512;

			//! When true, the Archive has called WriteFile, and an archive has been populated. Used to throw warnings on creating empty archives
			bool HasWritten;
			
			//! When true, the Archive has gracefully closed down (via the Close() function). Calls to Write or Read will fail. 
			bool HasClosed;

			//! When true, a Write-stream is currently active (via ActivateStream()), writing to a single file within the archive. Used to throw warnings, and detect when to shut down the Stream object.
			bool FileOpen;

			
			//! If FileOpen is true, holds the name of the file the Write-stream is currently opened to
			std::string OpenFileName;
			
			//! If FileOpen is true, this acts as the Write-stream. Objects streamed into the Archive are streamed here until DeactivateStream() is called.
			std::stringstream FileBuffer;
			
			//! Detects if a file has been written with the same name as another one. The older file remains within the archive, but cannot be accessed, since the newer one takes priority. If true, triggers the duplication-cleanup process in the destructor.
			bool RequiresDuplicateCleanup;

			

			//! The stream associated with the archive on disk. Performs the complex read and write operations. Not to be confused with \ref FileBuffer.
			std::fstream Stream;

			//! A list of files within the current archive. In read mode, the ReadMetaData is populated, allowing for \ref Stream to jump to the correct location. In Write mode, this is used to set \ref RequiresDuplicateCleanup, and the ReadMetaData is spoofed.
			std::unordered_map<std::string, ReadMetaData> FileIndex;

			//! The filename and path associated with the archive - set during the constructor, or by Open()
			std::string Name;

			//! The current ::ArchiveMode. Set by construction, Open() or ChangeMode(). Changing the mode is (essentially) equivalent to re-constructing the Archive-in place.
			ArchiveMode Mode;
			

			/*! @brief Configures the Archive for Reading and opens the associated streams.

				@details Called when Open() or ChangeMode() set the mode to reading, or at the beginning of an OpenForAppending(). Calls BuildIndex() in order to establish archive integrity

				@throws runtime_error If the file given by \ref Name does not exist on disk (or is otherwise inaccessible)

				@throws runtime_error If the archive cannot be read, is corrupted, or is missing the tar-defined nullbyte termination sequence 
			*/ 
			void OpenForReading();

			/*! @brief Configures the Archive for Writing and opens the associated streams

				@details Called when Open() or ChangeMode() set the mode to writing, or at the end of an OpenForAppending(). Overwrites any existing files at the location of \ref Name.

				@throws warning If a file with the name \ref Name already exists, but does not prevent it from being overwritten

				@throws runtime_error If the output file stream cannot be opened
			*/
			void OpenForWriting();

			/*! @brief Copies an existing archive in-place, and then opens it. Allows for new files to be added to an existing archive. 

				@warning Does **not** allow for appending to files in an existing archive. This operation attempts to read the archive entirely into memory, before re-writing it. This is computationally expensive. Append mode should be used with caution.
			
				@throws runtime_error If either OpenForReading() or OpenForWriting() would throw an error.
			*/
			void OpenForAppending();


			//! Writes the terminating null-byte strings to an archive during 
			void WriteCleanup(unsigned int tailBlockRepetition= 2u);
			
			/*! @brief Checks if fileName exists within \ref FileIndex

				@details If fileName is found, sets \ref RequiresDuplicateCleanup to true, which flags the old version as being redundant. 

				@throws warning If fileName is found, and not in Append mode.
			*/
			void CheckWriteRegistry(std::string fileName);

			/*! @brief Scans through the archive block-by-block and detects the breakpoints between files, building an index of the internal structure of the archive

				@throws runtime_error If the archive is not correctly null-terminated
			*/
			void BuildIndex();

			/*!
				Reads in a block of size \ref BLOCK_SIZE into the memory buffer.
				@param buffer a c-style buffer, part of the file-streaming primitives
			*/
			bool ReadBlock(char*buffer);

			/*!
				@brief Iteratively calls ReadBlock(), and performs a callback function on it
				@details This forms the core of all Read functions. Note, however, that each Block is 512 bytes long, and does not necessarily correspond to anything particularly meaningful. File readins based on i.e. linebreaks have to get clever.
				@param fileName The file to be queried
				@param dataCallback The function to be called on each block. Usually an accumulator of some form
			*/
			void StreamBlocks(std::string fileName, std::function<void(std::string)> dataCallback);

			/*! @brief A check performed at various stages to enforce the write/read divide. 
				@param targetState The expected write/read/append state of the Archive at the calling location
				@throws logic_error when \ref Mode does not match targetState
			*/
			void CheckValidState(ArchiveMode targetState);
		public:
			//! When true, suppresses the warnings associated with HasWritten. Indicates that an archive is allowed to be empty.
			bool ExpectingEmpty = false;


			/** @name Constructors & Initialisers */
			///@{

				//! Default initialiser, which places the object into an ::ArchiveMode::Uninitialised state. Throws an error if used before Open() is called.
				Archive();

				//! Initialises the object and immediately calls Open() @param archivePath The name of the archive on disk @param mode The ::ArchiveMode assigned to the object
				Archive(std::string archivePath, ArchiveMode mode = Read);

				/*! @brief Initialises the Archive into a valid state and sets the file location of the archive on disk.
					@brief Depending on the mode, calls either OpenForReading(), OpenForWriting() or OpenForAppending(). See those documentation for more details.

					@param archivePath The name of the archive on disk 
					@param mode The ::ArchiveMode assigned to the object

					@throws Warning when called on an already-open archive, before Close() has been called. 
					@throws runtime_error when mode is ::ArchiveMode::Uninitialised
				*/
				void Open(std::string archivePath, ArchiveMode mode);

				/*! As with Open(), but only changes the \ref Mode, not the \ref Name. 
				
					@details Functionally acts to create a new Archive object in-place with the new mode.

					@param mode The new mode to assign to \ref Mode
				*/
				void ChangeMode(ArchiveMode mode);

			///@}
			
			/** @name Destructors and Cleaners */
			///@{
				//! @brief Shuts down any still-open streams, calling Close() 
				~Archive();

				/*!
					@brief Deactivates the current Archive stream. And begins archive flushing.
					
					@details In read mode, simply shuts down \ref Stream. In Write mode, signals the end of the archive, calling WriteCleanup() and (if necessary), cleaning up duplicate files. 
				*/
				void Close();
			///@}

			/** @name Writing Functions */
			///@{
				/*! @brief The base write-to-tar function. Adds a file with this data into the target archive. 
					@details This code is near-verbatim the code from <a href="https://github.com/Armchair-Software/tar_to_stream/">tar_to_stream</a>. As a result, it has a *lot* of dark sorcery, magic number hackery inside.
					@param input A formatted WriteMetaData object, corresponding to the bytestream of the data to be written,
					@throws warning If input.filename already exists as a file within the archive -- this file is then overwritten.
					@throws logic_error If called whilst in Read mode
				*/
				void WriteFile(WriteMetaData &&input);

				/*! @brief Writes the passed string as data into a file in the archive. 
					@param fileName The name of the file within the archive (can include subdirectories)
					@param data The string which will comprise the full contents of the file. Control characters (such as \\n) are executed.
					@throws warning If fileName already exists as a file within the archive -- this file is then overwritten.
					@throws logic_error If called whilst in Read mode
				*/
				void WriteFile(const std::string & fileName, const std::string & data);

				/*!
					@brief Activate \ref FileBuffer as a valid stream, pointing towards a particular file within the archive.
					@details sets \ref OpenFileName and \ref FileOpen as persistent entities
					@throws warning If fileName already exists as a file within the archive -- this file is then overwritten.
					@returns A reference to \ref FileBuffer
				*/
				std::stringstream & ActivateStream(const std::string & fileName);

			
				/*!
					@brief Closes \ref FileBuffer, and flushes the output to WriteFile, adding it into the archive at the specified location. 
					@details Unsets \ref OpenFileName and \ref FileOpen.
				*/
				void DeactivateStream();

				/*! @brief Streams the input data into \ref FileBuffer

					@returns A reference to FileBuffer, allowing chaining of streams

					@throws logic_error If called before ActivateStream() is called.
				*/
				template<class T>
				std::stringstream &operator<<(const T &msg)
				{
					if (FileOpen)
					{
						CheckValidState(Write);
						FileBuffer << msg;
					}
					else
					{
						LOG(ERROR) << "Trying to stream data into an archive without an active stream.";
						throw std::logic_error("Attempted to access inactive stream");
					}
					return FileBuffer;
				}
			///@}


			/** @name Reading Functions */
			///@{
				//! Get a vector list of files contained in the archive. @throws when called whilst in write mode @returns A vector with each element being the name of a file in the archive
				std::vector<std::string> ListFiles();

				/*! Get the entire text of a file as a single string
					@param file The file to be queried 
					@returns A string representing the entire entire file, interpreted as plaintext.
					@throws logic_error if called whilst in write mode 
					@throws runtime_error if file not in the archive
				*/
				std::string GetText(std::string file);

				/*!
					@brief Iterates line-by-line through a plaintext file, calling perLineFunction on each line
					@param fileName The file to be iterated through
					@param perLineFunction A function (expressed as a lambda or an std::function) which is called on each line. The function cannot have a return value, but may capture-by-reference values outside the lambda scope.
					@throws logic_error if called whilst in write mode 
					@throws runtime_error if fileName not in the archive
				*/
				void ForLineIn(const std::string & fileName,std::function<void(std::string_view)> perLineFunction);

				/*!
					@brief Parse the datafile and convert each line into a tuple of datatypes based on an assumed regular tabular. Then performs a callback function on each line.

					@details Each line is converted into a tuple of types specified by the ColumnTypes parameter (i.e. ForTabularLineIn<int,int,int> attempts to read the file as a set of three integers). Tuples are then accessed via std::get<i> to get the ith element of the tuple.
					@param ColumnTypes An (arbitrary) number of typenames, representing the converted types of each element. typenames must have an associated convert() function.
					@param TupleFunctor (optional -- usually compiler-inferred) template parameter for the type of the per-tuple callback function
					@param fileName The file to be queried
					@param delimiter The string delimiter between ColumnTypes in the file
					@param perTupleFunction The function to be called on 

					@throws runtime_error If the columns of the file cannot be interpreted as a regular grid of len(ColumnTypes) with consistent data types with the specified delimiter
					@throws runtime_error If a datatype in a column cannot be converted into the specified typr
					@throws logic_error when called whilst in write mode 
					@throws logic_error when fileName not in the archive
					
				*/
				template<typename...ColumnTypes, typename TupleFunctor>
				void ForTabularLineIn(const std::string& fileName, std::string delimiter,TupleFunctor perTupleFunction)
				{
					ForLineIn(fileName,[&](std::string_view line){
						
						auto row = convertTuple<ColumnTypes...>(split(line,delimiter));

						perTupleFunction(row);

					});
				}

				/*!
					Parse the datafile and convert each line into a tuple of datatypes.
					@details Each line is converted into a tuple of types specified by the ColumnTypes parameter (i.e. ForTabularLineIn<int,int,int> attempts to read the file as a set of three integers). Tuples are then accessed via std::get<i> to get the ith element of the tuple.
					@param ColumnTypes An (arbitrary) number of typenames, representing the converted types of each element. typenames must have an associated convert() function.
					@param fileName The file to be queried
					@param delimiter The string delimiter between ColumnTypes in the file
					@returns A vector of tuples, with each element representing a line in the datafile.
					@throws runtime_error If the columns of the file cannot be interpreted as a regular grid of len(ColumnTypes) with consistent data types with the specified delimiter
					@throws runtime_error If a datatype in a column cannot be converted into the specified typr
					@throws logic_error if called whilst in write mode 
					@throws logic_error if fileName not in the archive
					
				*/
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

			///@}

		//!Deleted copy constructor to prevent pointer nonsense
		Archive(const Archive&) = delete;
		//!Deleted move constructor to prevent pointer nonsense
		Archive& operator=(const Archive&) = delete;
	};
}