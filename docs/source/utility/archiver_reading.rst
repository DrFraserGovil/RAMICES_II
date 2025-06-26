.. archiver_reading

Reading From Archives
=====================

Archive Reading can occur if the archive was:

1. :cpp:func:`Constructed <void Archiver::Archive::Archive(std::string archivePath, ArchiveMode mode)>` in Read mode 
2. Opened into Read mode (:cpp:func:`Open<Archiver::Archive::Open>`) 
3. Switched into Read mode (:cpp:func:`ChangeMode <Archiver::Archive::ChangeMode>`)

When an Archive initialises into Read mode, it scans the object and attempts to :cpp:any:`Build an Index<Archiver::Archive::BuildIndex>` of files within the archive.

Querying files
------------------

The  :cpp:any:`ListFiles<Archiver::Archive::ListFiles>` function returns a vector-of-strings, indicating the filenames of files within the archive. Each filename is guaranteed to be unique. 

Accessing a file which is not part of the ListFiles return will cause an Error to be thrown.

The ordering of files within the ListFiles object is not guaranteed to obey any particular rule. Generally it will follow a First-In-Last-Out principle, but this is only a heuristic for the user. Code should not rely on the internal file order of the archive.

Reading Files
-----------------

By default, the Archive assumes that all files to be read in are text files -- binary files would need a custom handler written for them, to interface with the :cpp:any:`StreamBlocks<Archiver::Archive::StreamBlocks>` function.

Text Extraction
///////////////////

The simplest reading method acts to simply copy the file into a std::string object

.. code-block:: cpp

    #example.cpp
    #include "Archive.h"

    using namespace Archiver
    
    int main(int argc, char** argv)
    {
        Archive A("test.archive",Read);
        std::cout << "The archive is open! What is inside?\n";
        for (auto file : A.ListFiles())
        { 
            std::string content = A.GetText(file);
            std::cout << file << "\n" << content << "\n";
        }
        return 0;
    }

.. code-block:: console

    $ echo -e "hi!\n\tThis is a greeting!" > "hi.txt"; 
    $ echo -e "bye!\n\tThis is a farewell!" > "bye.txt"; 
    $ tar -cf test.archive hi.txt bye.txt
    $ ./test.out
    The archive is open! What is inside?
    Bye.txt
    bye!
        This is a farewell!
    Hi.txt
    hi!
        This is a greeting!


For Line In
/////////////////

Many times it is desirable not only to access the text, but to process it. The :cpp:any:`ForLineIn<Archiver::Archive::ForLineIn>` function iterates line by line and performs a callback function.

.. code-block:: cpp

    #example.cpp
    #include "Archive.h"

    using namespace Archiver
    
    int main(int argc, char** argv)
    {
        Archive A("test.archive",Read);
        
        int numberOfLetterE = 0;
        for (auto file : A.ListFiles())
        { 
            A.ForLineIn(file,[&numberOfLetterE](auto line){

                for (auto &ch : line)
                {
                    numberOfLetterE += (int)(ch=='e');
                }
            });
        }
        std::cout << "The letter 'e' occurs " << numberOfLetterE << " times in the archive";
        return 0;
    }

.. code-block:: console

    $ echo -e "hi!\n\tThis is a greeting!" > "hi.txt"; 
    $ echo -e "bye!\n\tThis is a farewell!" > "bye.txt"; 
    $ tar -cf test.archive hi.txt bye.txt
    $ ./test.out
    The letter 'e' occurs 3 times in the archive

GetTabular
///////////////

An equivalent of the GetText function, but where the data is known to obey a tabulated data format. Each line is assumed to be a delimited line of equal length, with each element being a compile-time determined data type.

GetTabular makes use of the :cpp:func:`convert\<T\> <convert>` interface to convert the input strings (or string_views) into the expected type. See the convert documentation for the supported data types. 

Each line is stored as a `tuple <https://en.cppreference.com/w/cpp/utility/tuple.html>`_. These are rather more structured than a vector (and annoying to access) but allow for mixed-types.

The code will throw an error if the line has the incorrect number of elements (i.e. if a <int,int,int> line has 2 or 4 elements), as detected by the delimiter. The GetTabular interface does not allow modification of the vector-convert internal delimiter, using the default "," -- it is therefore recommended to not use commas to delimit files.

.. code-block:: cpp

    #example.cpp
    #include "Archive.h"

    using namespace Archiver
    
    int main(int argc, char** argv)
    {
        Archive A("test.archive",Read);
        
        auto people = A.GetTabular<std::string,int,std::vector<int>>("people.txt"," ");

        std::cout << "Name\tAge\t#. of Accolades\n";
        for (auto line : people)
        {
            std::cout << std::get<0>(line) <<"\t";
            std::cout << std::get<1>(line) << "\t";
            std::cout << std::get<2>(line).size() << "\n";
        }
    }

.. code-block:: console

    $ echo -e "cat 0 1\ndog 2 8.4\nsquirrel 8 -8.2" > animals.txt
    $ echo -e "jess 0 []\njack 17 [3,9,8]\njyoti 2 [0,0,0,0,2]" > people.txt
    $ tar -cf test.archive animals.txt people.txt
    $ ./test.out
    Name    Age     #. of Accolades
    jess    0       0
    jack    17      3
    jyoti   2       5


ForTabularLineIn
//////////////////////

As with GetTabular, but instead of returning a vector of the tuples, it performs a callback function on them.

.. code-block:: cpp

    #example.cpp
    #include "Archive.h"

    using namespace Archiver
    
    int main(int argc, char** argv)
    {
        Archive A("test.archive",Read);
        
        int accumulator = 0;
        A.ForLineIn<std::string,int,double>("animals.txt"," ",[&](auto line)//auto because tuples are long
        {
            if (std::get<1>(line) > 5)
            {
               accumulator += std::get<2>(line);
            }
        }
        std::cout << "The accumulator value is " << accumulator;
    }

.. code-block:: console

    $ echo -e "cat 0 1\ndog 2 8.4\nsquirrel 8 -8.2" > animals.txt
    $ echo -e "jess 0 []\njack 17 [3,9,8]\njyoti 2 [0,0,0,0,2]" > people.txt
    $ tar -cf test.archive animals.txt people.txt
    $ ./test.out
    The accumulator value is 0.2