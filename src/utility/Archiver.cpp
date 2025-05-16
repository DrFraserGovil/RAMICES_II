#include "Archiver.h"

namespace JAR
{
	Archive::Archive(){OpenedStream = false; HasClosed = false;};
	Archive::Archive(std::string archivePath) : Archive(archivePath, std::ios::in | std::ios::out){HasClosed = false;}
	Archive::Archive(std::string archivePath, std::ios_base::openmode mode) : Mode(mode),Name(archivePath)
	{	
		OpenedStream = false;
		HasClosed = false;
		// Open(archivePath,mode);
	}
	//doesn't actually open it, since opening occurs on a delay when the first write happens....
	void Archive::Open(std::string archivePath, std::ios_base::openmode mode)
	{
		Name = archivePath;
		Mode =mode;
	}
	void Archive::OpenStream()
	{

		switch (Mode)
		{
			case(std::ios::in):
				LOG(DEBUG) << "Opening a file stream in READ mode";
				break;
			case(std::ios::out):
				LOG(DEBUG) << "Opening a file stream in WRITE mode";
				break;
		}
		OpenedStream = true;
		Stream.open(Name, Mode | std::ios::binary);
		IndexBuilt = false;
		HasWritten = false;
		if (!Stream.is_open()) 
		{
			throw std::runtime_error("Failed to open archive: " + Name);
		}
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
		HasClosed = true;
		if (HasWritten)
		{
			WriteCleanup();
		}
		if (Stream.is_open())
		{
			Stream.close();
		}
	}
	void Archive::WriteCleanup(unsigned int tail_length)
	{
		/// TAR archives expect a tail of null bytes at the end - min of 512 * 2, but implementations often add more
		Stream << std::string(tail_length, '\0');
	}


	std::vector<std::string> Archive::ListFiles()
	{
		if (!IndexBuilt || !OpenedStream)
		{
			BuildIndex();
		}
		std::vector<std::string> filenames;
		for (const auto& [filename, _] : FileIndex) 
		{
			filenames.push_back(filename);
		}
		return filenames;
	}
	bool Archive::ReadBlock(char* buffer)
	{
		Stream.read(buffer, BLOCK_SIZE);
        return Stream.gcount() == BLOCK_SIZE;
	}
	void Archive::BuildIndex()
	{
		if (!OpenedStream)
		{
			OpenStream();
		}
		char header[BLOCK_SIZE];
        while (ReadBlock(header)) 
		{
            // Check for all-zero block (end of archive)
            if (std::all_of(header, header + BLOCK_SIZE, [](char c) { return c == '\0'; })) {
                break;
            }

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
		IndexBuilt = true;
		LOG(DEBUG) << "Successfully constructed an index of the archive.";
    }
	void Archive::Write(WriteMetaData &&file)
	{
		// LOG(DEBUG) << "Writing data chunk directly into archive.";
		if (!OpenedStream)
		{
			OpenStream();			
		}
		/// Read a "file" in memory, and write it as a TAR archive to the stream
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
		Stream<< std::string_view{header.name, sizeof(header)}
				<< std::string_view{reinterpret_cast<char const*>(file.data.data()), file.data.size()}
				<< std::string(padding, '\0');
		HasWritten = true;
	}

	//a default writer which constructs a metadata file with basic input
	void Archive::Write(const std::string & file,const std::string & data)
	{
		// WriteMetaData md;
		Write({
			.filename{file},
			.data{std::as_bytes(std::span<const char>{data})},
		});
	}

	std::string Archive::GetText(std::string fileName)
	{
		std::string buffer = "";
		StreamFile<std::string>(fileName,[&](std::string block)
		{
			buffer += block;
		});
		return buffer;
	}

	
}