.. archiver_modes

Archive Modes
----------------


Just as with the Standard Library `std::fstream`, Archives can be opened in a number of 'modes'. 

The Mode of an Archive object is a private member, and can only be altered by calling Archive::Open().

.. doxygenenum:: Archiver::ArchiveMode

Using Modes
==================


Unlike std::ios_base::openmode, we do not allow 'combining' of modes. Archives are always strictly in a 'read' or 'write' mode - and archives cannot be read 'mid-write' -- they must first pass out of scope or have Close() called manually.

Trying to read from an Archive in Write mode (and vice versa) will cause errors. Similarly, constructing a second archive in Read mode to 'follow' a writing archive will cause errors,  unless the archive is closed first.

The Append mode **does not** allow appending to files within an archive - instead it allows new files to be added to an existing archive. Appending to a file requires first extracting the file text whilst in Read mode, appending the new information, then calling ChangeMode(Append) and overwriting the file with this new information.


Why?
////////

This is a consequence of the 'fire-and-forget' nature of an Archive; which are designed to hold potentially very large data objects; more than could be fit into memory at once.

This design principle means that the archive does not load itself into memory on a Read, or retain a buffered memory of previous Write operations. 
