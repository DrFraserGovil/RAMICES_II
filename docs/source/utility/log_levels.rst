.. log_levels

Log Levels
+++++++++++++

The Log system is based around the concept of 'Levels'. An entry in the log is always assigned one of four levels.

At run time, a :doc:`ConfigObject<log_config>` is created and an "Output Level" is assigned. If the Output Level is *lower* than a Log Entry, then the Entry is not printed.

Log levels therefore allow the user to determine (at runtime) what level of detail is output (either to terminal, or to file).

Enumerations
//////////////

.. doxygenenum:: LogLevel
    

Converters
//////////////

.. doxygenfunction:: LogLevelConvert