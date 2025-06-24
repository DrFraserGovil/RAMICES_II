.. log_globals

Log Global Variables
===========================

Because we wish the Logger to be accessible anywhere, with minimal 'passing' of settings and configurations, we place the :ref:`LoggerCore object<Logger Core>` inside a globally accessible namespace: `GlobalLog`.

Several other global variables also exist within this namespace.

Other Members
-------------------

.. doxygenvariable:: GlobalLog::Config

.. doxygenvariable:: GlobalLog::StreamMutex

.. doxygenvariable:: GlobalLog::PreviousLines