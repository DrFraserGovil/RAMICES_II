#include "Archiver.h"

namespace Archiver
{
	Archive::Archive(){HasClosed = false;Mode = Uninitialised;};
	Archive::Archive(std::string archivePath, ArchiveMode mode)
	{	
		Mode = Uninitialised;
		HasClosed = false;
		Open(archivePath,mode);
	}


	void Archive::Open(std::string archivePath, ArchiveMode mode)
	{
		if (Mode!=Uninitialised)
		{
			LOG(WARN) << "Called Open() on an already open Archive. Forcing a Close() call";
			Close();
		}
		Mode = Uninitialised;
		Name = archivePath;
	

		switch (mode)
		{
			case(Read):
				OpenForReading();
				break;
			case(Write):
				OpenForWriting();
				break;
			default:
				LOG(ERROR) << "Must open a file in either Read or Write mode";
				throw std::runtime_error("Opened archive in invalid state");
		}
	}

	void Archive::OpenForReading()
	{
		LOG(DEBUG) << "Opening a file stream in READ mode";
		Stream.open(Name, std::ios::in | std::ios::binary);
		if (!Stream.is_open()) 
		{
			LOG(ERROR) << "The archive " << Name << " could not be opened in READ mode";
			throw std::runtime_error("Failed to open archive: " + Name);
		}
		BuildIndex();
		Mode = Read;//manually assign -- therefore checks to CheckRead validate that this function has completed
	}

	void Archive::OpenForWriting()
	{
		LOG(DEBUG) << "Opening a file stream in WRITE mode";
		if (std::filesystem::exists(Name))
		{
			LOG(WARN) << "A file with the name " << Name << " already exists at the specified location. It is being overwritten.";
		}
		Stream.open(Name, std::ios::out | std::ios::binary);
		if (!Stream.is_open()) 
		{
			LOG(ERROR) << "The archive " << Name << " could not be opened in the WRITE mode";
			throw std::runtime_error("Failed to open archive: " + Name);
		}
		HasWritten = false;
		HasClosed = false;
		FileOpen = false;
		Mode = Write; //manually assign -- therefore checks to CheckWrite validate that this function has completed
	}

	Archive::~Archive()
	{
		if (!HasClosed)
		{
			Close();
		}
	}
	void Archive::Close()
	{
		if (Mode == Write)
		{
			if (FileOpen)
			{
				DeactivateStream();
			}
			if (!HasWritten && !ExpectingEmpty)
			{
				LOG(WARN) << "The archive " << Name << " was created, but no data was provided. The archive is valid, but empty";
			}

			//append the termination sequence
			WriteCleanup();
		}


		if (Stream.is_open())
		{
			Stream.close();
		}
		HasClosed = true;
	}
	void Archive::WriteCleanup(unsigned int tailBlockRepetition)
	{
		/// TAR archives expect a tail of null bytes at the end - min of 512 * 2, but implementations often add more
		Stream << std::string(tailBlockRepetition * BLOCK_SIZE, '\0');
	}

	void Archive::CheckValidState(ArchiveMode targetState)
	{
		if (Mode != targetState)
		{
			if (Mode == Write)
			{
				LOG(ERROR) << "Cannot call read functions whilst in write mode";
			}
			if (Mode == Read)
			{
				LOG(ERROR) << "Cannot call write functions whilst in read mode";
			}
			if (Mode == Uninitialised)
			{
				LOG(ERROR) << "Cannot call read functions on an Uninitialised archive: must complete a call to Open()!";
			}
			throw std::runtime_error("Accessed Read-functions whilst in invalid state");
		}
	}
	

	bool Archive::ReadBlock(char* buffer)
	{
		
		Stream.read(buffer, BLOCK_SIZE);
        return Stream.gcount() == BLOCK_SIZE;
	}
	
	void Archive::BuildIndex()
	{
		//Assume we don't need a check here -- it's a private function that can only be called via Open.
		//This completeing without throwing is part of the validation efforts!
		char header[BLOCK_SIZE];
		int zeroBlockCount = 0;
        while (ReadBlock(header)) 
		{
            // Check for all-zero block (end of archive is two such blocks)
            if (std::all_of(header, header + BLOCK_SIZE, [](char c) { return c == '\0'; })) 
			{
				++zeroBlockCount;
				if (zeroBlockCount >= 2)
				{
					break;
				}
            }	
			else
			{
				zeroBlockCount = 0; //reset the count -- termination sequence is two contiguous null blocks
			

				// Parse metadata
				ReadMetaData metadata;
				metadata.filename = std::string(header, 100);
				metadata.filename.erase(metadata.filename.find('\0')); // Remove null padding

				char size_str[12];
				std::memcpy(size_str, header + 124, 12); // File size starts at offset 124
				size_str[11] = '\0'; // Ensure null-termination
				metadata.file_size = std::strtol(size_str, nullptr, 8); // Size is octal-encoded

				// Calculate data offset
				metadata.data_offset = Stream.tellg(); // Current position after reading the header

				// Store metadata in the index
				FileIndex[metadata.filename] = metadata;

				// Skip file data and padding
				size_t data_blocks = (metadata.file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
				Stream.seekg(data_blocks * BLOCK_SIZE, std::ios::cur);
			}
        }

		if (zeroBlockCount >= 2)
		{
			LOG(DEBUG) << "Located end of Archive.\nSuccessfully constructed an index of the archive.";
		}
		else
		{
			LOG(ERROR) << "Archive Scan did not locate null termination sequence. The Archive " << Name << " is corrupted or incomplete";
			throw std::runtime_error("Corrupted archive");
		}
    }
	std::vector<std::string> Archive::ListFiles()
	{
		//Check we are in Read mode & and therefore properly initialised etc.
		CheckValidState(Read);
		std::vector<std::string> filenames(FileIndex.size());
		int i = 0;
		for (const auto& [filename, _] : FileIndex) 
		{
			filenames[i] = filename;
			++i;
		}
		return filenames;
	}
	void Archive::WriteFile(WriteMetaData &&file)
	{
		CheckValidState(Write);
		/// Read a "file" in memory, and write it as a TAR archive to the stream

		//this is all hardcoded and 'magic numbers' -- they're compile-time constants determined by the TAR specification. 
		//It's gross, but it works.
		struct {                                                                      // offset
			char name[100]{};                                                           //   0    filename
			char mode[8]{};                                                             // 100    file mode: 0000644 etc
			char uid[8]{};                                                              // 108    user id, ascii representation of octal value: "0001750" (for UID 1000)
			char gid[8]{};                                                              // 116    group id, ascii representation of octal value: "0001750" (for GID 1000)
			char size[12]{};                                                            // 124    file size, ascii representation of octal value
			char mtime[12]{"00000000000"};                                              // 136    modification time, seconds since epoch
			char chksum[8]{' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};                     // 148    checksum: six octal bytes followed by null and ' '.  Checksum is the octal sum of all bytes in the header, with chksum field set to 8 spaces.
			char typeflag{'0'};                                                         // 156    '0'
			char linkname[100]{};                                                       // 157    null bytes when not a link
			char magic[6]{'u', 's', 't', 'a', 'r', ' '};                                // 257    format: Unix Standard TAR: "ustar ", not null-terminated
			char version[2]{" "};                                                       // 263    " "
			char uname[32]{};                                                           // 265    user name
			char gname[32]{};                                                           // 297    group name
			char devmajor[8]{};                                                         // 329    null bytes
			char devminor[8]{};                                                         // 337    null bytes
			char prefix[155]{};                                                         // 345    null bytes
			char padding[12]{};                                                         // 500    padding to reach 512 block size
		} header;                                                                     // 512

		file.filemode.insert(file.filemode.begin(), 7 - file.filemode.length(), '0'); // zero-pad the file mode

		std::strncpy(header.name,  file.filename.c_str(), sizeof(header.name ) - 1);  // leave one char for the final null
		std::strncpy(header.mode,  file.filemode.c_str(), sizeof(header.mode ) - 1);
		std::strncpy(header.uname, file.uname.c_str(),    sizeof(header.uname) - 1);
		std::strncpy(header.gname, file.gname.c_str(),    sizeof(header.gname) - 1);

		snprintf(header.size, 12, "%011lo",  file.data.size());
		snprintf(header.mtime,12, "%011llo", file.mtime);
		snprintf(header.uid, 8,  "%07o",    file.uid);
		snprintf(header.gid,8,   "%07o",    file.gid);

		{
			unsigned int checksum_value = 0;
			for(size_t i{0}; i != sizeof(header); ++i) {
			checksum_value += reinterpret_cast<uint8_t*>(&header)[i];
			}
			snprintf(header.chksum,8, "%06o", checksum_value);
		}
		size_t const padding{(512u - file.data.size() % 512) & 511u};
		Stream<< std::string_view(reinterpret_cast<const char*>(&header), sizeof(header))
				<< std::string_view{reinterpret_cast<char const*>(file.data.data()), file.data.size()}
				<< std::string(padding, '\0');
		HasWritten = true;
	}

	//a default writer which constructs a metadata file with basic input
	void Archive::WriteFile(const std::string & file,const std::string & data)
	{
		// WriteMetaData md;
		WriteFile({
			.filename{file},
			.data{std::as_bytes(std::span<const char>{data})},
		});
	}

	std::stringstream & Archive::ActivateStream(const std::string & filename)
	{
		CheckValidState(Write);
		if (FileOpen)
		{
			LOG(WARN) << "Closing file " << OpenFileName << " automatically. Can only have one open filestream at a time"; 
			DeactivateStream();
		}

		FileBuffer.str("");//clear the stringstream
		OpenFileName = filename;
		FileOpen = true;
		return FileBuffer;
	}
	void Archive::DeactivateStream()
	{
		FileOpen = false;
		WriteFile(OpenFileName,FileBuffer.str());
	}

	std::string Archive::GetText(std::string fileName)
	{

		std::string buffer = "";
		StreamBlocks(fileName,[&](std::string block)
		{
			buffer += block;
		});
		return buffer;
	}

	
	void Archive::StreamBlocks(std::string fileName, std::function<void(std::string)> dataCallback)
	{
		CheckValidState(Read);
		auto it = FileIndex.find(fileName);
		if (it == FileIndex.end())
		{
			throw std::runtime_error("File not found in archive. File: " + fileName + " Archive: " + Name + "\n");
		}
		ReadMetaData meta = it->second;

		Stream.seekg(meta.data_offset);
		size_t remainingSize = meta.file_size;
		const int blocksInBuffer = 10;
		char buffer[blocksInBuffer*BLOCK_SIZE];

		while (remainingSize > 0)
		{
			size_t chunk_size = std::min(remainingSize, sizeof(buffer));
			Stream.read(buffer, chunk_size);
			dataCallback(std::string(buffer,buffer+chunk_size));
			remainingSize -= chunk_size;
		}
	};

	void Archive::ForLineIn(const std::string & fileName,std::function<void(std::string_view)> perLineFunction)
			{
				//have to be clever because StreamBlocks (and the tar protocol in general) does not guarantee blocks are complete lines
				std::string overflow; //entity for holding overflow from the previous block
				StreamBlocks(fileName, [&](const std::string& block) 
				{
					std::string data = overflow + block;
					overflow.clear();
					size_t current_pos = 0; // Use a more descriptive name than 'start'
					size_t newline_pos = 0;
			
					while ((newline_pos = data.find('\n', current_pos)) != std::string::npos)
					{
						std::string_view line(data.data() + current_pos, newline_pos - current_pos);
						perLineFunction(line);
						current_pos = newline_pos + 1; // Move past the newline
					}
			
					// If anything remains after the last newline (or if no newline was found)
					if (current_pos < data.size())
					{
						overflow = data.substr(current_pos);
					}

				});

				//any remaining overflow at the end is presumed to be  non-newline-terminated final entry
				if (!overflow.empty())
				{
					perLineFunction(overflow);
				}
			}
}



