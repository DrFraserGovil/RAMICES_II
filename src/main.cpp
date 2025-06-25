#include <iostream>
#include "utility/Log.h"
#include "utility/parallel.h"
#include "utility/Timer.h"
#include "utility/Random.h"

#include "settings/SimulationSettings.h"
#include "population/lifetime.h"
#include <thread>
// int main(int argc, char ** argv)
// {
//     Settings.Initialise(argc,argv);
    
//     Stellar::Lifetime L;
    
//     return (0);
// }

//test.cpp
#include "utility/Archiver.h"
using namespace Archiver;
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

        std::vector<std::string> fileList = archive.ListFiles(); // some permutation of {"completeFile.txt","streamedFile.txt","numericFile.txt"}

        //see file list
        std::cout << "The files in the archive are:\n";
        for (auto file : fileList)
        {
            std::cout << "\t" << file <<"\n";
        }

        //basic string capture
        std::cout << "\nThe contents of 'streamedFile.txt' are:\n";
        std::string contents = archive.GetText("streamedFile.txt"); //copies the file contents into a raw string stream
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
      
        Archive A("t.tst",Write);
        for (int i =0 ; i < 10000; ++i)
        {
            std::string s = std::string(10000,'x');
            A.WriteFile("tmp.tst",s);
        }
        A.Close();
        Archive B("t.tst",Read);
        LOG(INFO) << MakeString(B.ListFiles());
        LOG(INFO) << B.GetText("tmp.tst");

    }