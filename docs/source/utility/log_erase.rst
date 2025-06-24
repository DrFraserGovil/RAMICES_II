.. log_erase

Erasing Logs
=================


It can sometimes be helpful to delete a number of lines from the terminal -- to prevent clutter or provide an 'animated' interface.

.. warning::
    
    Log Erasing functions using ANSI escape codes, and therefore only functions where ANSI commands are interpreted: primarily POSIX terminals.

    This also means Log Erasing can only occur if the output is a Terminal (i.e. :doc:`Config.TerminalOutput<log_globals>` is true). When piping to file, Log Erasing is automatically disabled, and nothing will happen.


Commands & Logic
--------------------

We provide two commands, the :doc:`Erase and ErasePrevious<log_core>`, both members of the LoggerCore -- and therefore also accessed through the usual :doc:`LOG macro<log>`.



Erase
//////////

.. warning::
    
    Unlike ErasePrevious, Erase is **not thread safe** (due to deadlock clashes with ErasePrevious). It is recommended to use a custom mutex if calling Erase in a multithreaded environment.



Erase 'deletes' `n` lines from the terminal output. It does this by writing several ANSI codes -- cursor-up, clear-line and cursor-left -- to the terminal `n` times. This has the effect of erasing the lines. 

We leave Erase exposed to the user, but note that it should only be used were ErasePrevious is not suitable.


ErasePrevious
////////////////

ErasePrevious continually deletes log entries of a lower priority (/higher :doc:`LogLevel<log_levels>`), until one of the following conditions has been met:

1. A higher priority log entry is encountered -- this is not deleted
2. A single log entry of a level equal to the :doc:`LoggerCore.Level<log_core>` (set by `LOG(level)`) has been deleted


ErasePrevious (and therefore the internal Erase call) is thread safe. 

.. warning::
    The Log does not keep a buffered record of all previous logs. ErasePrevious functions on simple line counts - and erases a set number of lines. Any non-Log output to the terminal will interfere with this calculation, and may produce unexpected results.

    The Logger only 'remembers' the last entry of each type. 

Usage 
-----------




.. code-block:: c++
    
    //test.cpp

    #include "Log.h"
    void testPrint(LogLevel level)
    {
        LogConfig.SetLevel(level)
        LOG(INFO) << "This is an initial info, which cannot be deleted!";
        LOG(INFO) << "This resets the `info' memory, so the previous line is forgotten";
        LOG(ERROR) << "This is an error";
        LOG(WARN) << "This is a warning\nIt is split across lines -- these will all be deleted together";
        LOG(DEBUG) << "This is noisy debugging";
    }

    int main(int argc, char**argv)
    {
        testPrint(DEBUG); //prints everything
        
        LOG(WARN).ErasePrevious(); //erases both the DEBUG + WARN
        
        LOG(DEBUG).ErasePrevious(); //no DEBUG to delete -- nothing happens
        
        LOG(INFO).ErasePrevious(); //The ERROR takes priority -- nothing is deleted
        
        LOG(ERROR).ErasePrevious(); //Deletes the error
        
        LOG(INFO).ErasePrevious(); //Deletes the *second* info block
        
        LOG(INFO).ErasePrevious(); //Does NOT delete the first INFO block 
        // This is because the internal memory cannot remember it. 
        
        LOG(INFO).Erase(1); //manually delete the line -- not recommended!
    }