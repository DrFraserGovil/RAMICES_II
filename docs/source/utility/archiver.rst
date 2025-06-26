.. Archiver

Archiver
++++++++++++++++++


Instead of relying on a huge set of output files, RAMICES III bundles a number of core inputs and outputs into **archives**. These archives are `tarballs <https://en.wikipedia.org/wiki/Tar_(computing)>`_, however instead of relying on external system calls, the Archiver module reads and writes files directly into the archive; effectively emulating the tar standard internally. 

The Archive class grew out of a code snippet from the `tar_to_stream <https://github.com/Armchair-Software/tar_to_stream/>`_ project (provided under an MIT license). Some of this code appears verbatim in Archive::WriteFile, however the OOP approach and the reading functions were written for RAMICESIII.

Details
----------


.. toctree::
    archiver_writing
    archiver_reading
    archiver_modes
    archiver_core
    archiver_meta
    :maxdepth: 1


Basic Usage
-------------------




.. code-block:: c++

    //test.cpp
    #include "Archive.h"
    
    void ArchiveWriting(std::string archivePath)
    {
        Archive archive(archivePath,ArchiveMode::Write);

        std::string fullFileData = "This is a complete file\nIt is already packaged up\nReady to be written\n";
        archive.WriteFile("completeFile.txt",fullFileData);

        archive.ActivateStream("streamedFile.txt");
        archive << "This data is streamed in\n";
        archive << "Bit by bit\n";
        archive << "Like an fstream\n";
        archive.DeactivateStream();

        archive.ActivateStream("numericFile.txt");
        for (int i = 0; i < 10; ++i)
        {
            archive << i << " " << 0.1*i << "\n";
        }
   
        
    } //archive passes out of scope, and closes itself, appending the necessary null-terminations


    void ArchiveReading(std::string archivePath)
    {
        Archive archive; //default initialise 
        archive.Open(archivePath,ArchiveMode::Read); //delayed initialisation (for demonstration purposes!)

        std::vector<std::string> fileList = archive.ListFiles(); 
        // fileList = some permutation of {"completeFile.txt","streamedFile.txt","numericFile.txt"}

        //see file list
        std::cout << "The files in the archive are:\n";
        for (auto file : fileList)
        {
            std::cout << "\t" << file <<"\n";
        }

        //basic string capture
        std::cout << "\nThe contents of 'streamedFile.txt' are:\n";

        //copy the file contents into a raw string stream
        std::string contents = archive.GetText("streamedFile.txt"); 
        std::cout << contents << "\n";


        //per-file read in -- only output lines which have the letter 'o' in them
        std::cout << "The following lines in `completeFile.txt' do not have 'o' in them:\n";
        archive.ForLineIn("completeFile.txt",[&](auto line)
        {
            if (std::find(line.begin(),line.end(),'o') == line.end())
            {
                std::cout << "\t" <<line << "\n";
            }
        });

        //numerical parsing
        double value = 0;
        archive.ForTabularLineIn<int,double>("numericFile.txt"," ",[&value](auto lineTuple)
        {
            int index = std::get<0>(lineTuple);
            if (index % 2 == 0)
            {
                value += std::get<1>(lineTuple);
            } 
        });
        std::cout << "\nThe sum of even-indices in numericFile.txt is: " << value << "\n";
    }

    int main(int argc, char**argv)
    {
        Settings.Initialise(argc,argv);
       ArchiveWriting("test.archive");
       ArchiveReading("test.archive");
    }

Gives the output:

.. code-block:: 

    #terminal output
    The files in the archive are:
        numericFile.txt
        streamedFile.txt
        completeFile.txt

    The contents of streamedFile.txt are:
    This data is streamed in
    Bit by bit
    Like an fstream

    The following lines in completeFile.txt do not have o in them:
            It is already packaged up

    The sum of even-indices in numericFile.txt is: 2

Note that, as expected, the files in the archive are not in any guaranteed order (FILO order is common, but not mandated)