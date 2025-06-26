.. archiver_meta

Archive Metadata
--------------------

Because tar files require very specific formatting and stream positioning, we use the following structs to hold the metadata associated with each file.

These are largely internal objects, and shouldn't need to be used by anybody.

Read Metadata
==================

.. doxygenstruct:: Archiver::ReadMetaData

Write Metadata
==================

This was inherited from the original code snippet that the Archive grew out of; `tar_to_stream <https://github.com/Armchair-Software/tar_to_stream/>`_, and is largely unchanged. 

.. doxygenstruct:: Archiver::WriteMetaData