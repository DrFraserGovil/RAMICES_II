.. Log

Logging System
++++++++++++++++++

RAMICES III uses a custom-built logging system for printing output to the terminal


.. doxygendefine:: LOG

Log Levels
---------------

.. doxygenenum:: LogLevel


Configuration
----------------

Usage
---------


.. code-block:: c++

    //test.cpp


    void testPrint(LogLevel level)
    {
        LogConfig.SetLevel(level)
        LOG(DEBUG) << "This is detailed debugging"
        LOG(INFO) << "This is progress information";
        LOG(WARN) << "This is a warning that something went wrong\nBut was recovered.";
        LOG(ERROR) << "Something has gone very badly wrong";
    }

    int main(int argc, char**argv)
    {
        LogConfig.SetLevel(INFO)
        testPrint(INFO);
        testPrint(ERROR);
    }

Internal Documentation
=========================

The following 


.. doxygenclass:: LoggerCore

