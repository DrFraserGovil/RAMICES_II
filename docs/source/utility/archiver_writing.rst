.. archiver_writing

Writing To Archives
=====================

Archive Writing can occur if the archive was:

1. :cpp:func:`Constructed <void Archiver::Archive::Archive(std::string archivePath, ArchiveMode mode)>` into Write or Append mode 
2. Opened into Write or Append mode (:cpp:func:`Open<Archiver::Archive::Open>`) 
3. Switched into Write or Append mode (:cpp:func:`ChangeMode <Archiver::Archive::ChangeMode>`)

When an Archive is opened, an entity with that name is immediately written to disk, overwriting any other file there (or, if file permissions do not allow, it throws an error). 

Types of Writing 
/////////////////////

Complete File Writing
---------------------------

The :cpp:func:`WriteFile(std::string fileName,std::string data)<void Archiver::Archive::WriteFile(const std::string &fileName, const std::string &data)>` function takes a single string -the :cpp:any:`data` - and writes it to the file :cpp:any:`fileName` within the target archive. 

The data object forms the complete contents of the file - no data can be added or appended at a later date.

.. code-block:: cpp

    #example.cpp
    #include "Archive.h"

    using namespace Archiver
    std::string GenerateText(int n)
    {
        std::stringstream stream;
        stream << "This file has " << n << " lines\n";
        for (int i = 2; i <= n; ++i)
        {
            stream << "Line "<< i << "\n";
        }
        return stream.str();
    }

    int main(int argc, char** argv)
    {
        Archive A("test.archive",Write);

        A.WriteFile("TenLines.txt",GenerateText(10));
        A.WriteFile("HundredLines.txt",GenerateText(100));
        A.WriteFile("MillionLines.txt",GenerateText(1e6));
        return 0;
    }

The `test.archive` will then contain three files, with 10, 100 and 1,000,000 lines of text:

.. code-block:: console

    $ ls
    test.archive  test.cpp  test.out
    $ tar -xf test.archive #manually extract the tar archive
    $ ls
    test.archive  test.cpp  test.out TenLines.txt  HundredLines.txt  MillionLines.txt
    $ head -n 5 HundredLines.txt
    This file has 100 lines
    Line 2
    Line 3
    Line 4
    Line 5
    $ tail -n 5 MillionLines.txt
    Line 999996
    Line 999997
    Line 999998
    Line 999999
    Line 1000000


Stream File Writing
------------------------

In the above example we constructed a temporary stream, which was then converted into a string. Archive provides this functionality automatically, via the :cpp:any:`ActivateStream()<Archiver::Archive::ActivateStream>` function. 

.. code-block:: cpp

    #example.cpp
    #include "Archive.h"

    using namespace Archiver
    std::string GenerateText(int n)
    {
        std::stringstream stream;
        stream << "This file has " << n << " lines\n";
        for (int i = 2; i <= n; ++i)
        {
            stream << "Line "<< i << "\n";
        }
        return stream.str();
    }

    int main(int argc, char** argv)
    {
        Archive A("test.archive",Write);

        A.WriteFile("TenLines.txt",GenerateText(10)); //exactly as before

        //alternative approach:
        auto stream = A.ActivateStream("StreamedLines.txt");
        stream << "This data is streamed in\n";
        stream << "Bit by bit\n";
        A.DeactivateStream(); //close the stream

        A.ActivateStream("OtherLines.txt");
        A << "You can also stream\n";
        A << "\t To the archive itself!";
        A << "\n Without needing to copy the 'stream' object.";
        
        A << "You can even get away with not deactivating the stream";
        A << ": But you probably should.";
        return 0;
    }

Running this code and examining the output:

.. code-block:: console

    $ ls
    test.archive  test.cpp  test.out
    $ tar -xf test.archive #manually extract the tar archive
    $ ls
    test.archive  test.cpp  test.out TenLines.txt StreamedLines.txt  OtherLines.txt
    $ cat OtherLines.txt
    You can also stream
        To the archive itself!
    Without needing to copy the 'stream object'
    You can even get away with not deactivating the stream: But you probably should.

As can be seen, any formatting escape codes are implemented as expected from normal C++ string-streams.

Manual File Writing
------------------------

The :cpp:func:`WriteFile(metaData)<void Archiver::Archive::WriteFile(WriteMetaData &&input)>` is used internally to implement the above two functions. It should not be used unless users are comfortable generating their own byte-converted string-streams into the :cpp:struct:`Archiver::WriteMetaData`.




Overwriting files
///////////////////////

Appending to a file in a tar archive is not possible. When writing to a file in a tar archive, if that file already exists, the newest file automatically takes precedence. 

Strictly speaking, the tar format allows two files to exist with the same name in an archive - however only the first (the most recent) is accessible. The other data remains as a 'ghost' within the archive - taking up space, but unable to be accessed. 

We prevent this through the :cpp:member:`RequiresDuplicateCleanup<Archiver::Archive::RequiresDuplicateCleanup>` flag, which detects when ghost data is present. When an archive with ghost data is closed, a temporary :cpp:any:`Append<Archiver::ArchiveMode::Append>`-mode Archive is created. This copies all accessible data into a duplicate archive, which overwrites the old -- thereby removing the duplicate files. 

.. warning::
    The creation of Append-mode archives is computationally costly (but prevents spiralling archive sizes). We therefore recommend avoiding file-name duplication wherever possible.
