.. Log

Logging System
++++++++++++++++++

RAMICES III uses a custom-built logging system for printing output to the terminal


.. doxygendefine:: LOG
    :path:

Modifying Logs
---------------

.. toctree::
    log_levels
    log_config
    log_globals
    log_core
    :maxdepth: 1


Usage
---------


.. code-block:: c++

    //test.cpp

    #include "Log.h"
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
        testPrint(DEBUG);
        testPrint(WARN);
    }




.. raw:: html

    <div class="highlight log-output-box"> <pre><code>
    <span class="log-debug">[DEBUG] This is detailed debugging</span><br>
    <span class="log-info">[INFO]   &nbspThis is progress information</span><br>
    <span class="log-warn">[WARN]   &nbspLine 7 of src/main.cpp in function testPrint</span><br>
    <span class="log-warn">&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbspThis is a warning that something went wrong</span><br>
    <span class="log-warn">&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbspBut was recovered.</span><br>
    <span class="log-error">[ERROR] Line 8 of src/main.cpp in function testPrint</span><br>
    <span class="log-error">&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbspomething has gone very badly wrong</span><br>
    <span class="log-warn">[WARN]   &nbspLine 7 of src/main.cpp in function testPrint</span><br>
    <span class="log-warn">&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbspBut was recovered.</span><br>
    <span class="log-error">[ERROR] Line 8 of src/main.cpp in function testPrint</span><br>
    <span class="log-error">&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbspSomething has gone very badly wrong</span>
    </code></pre></div> 




