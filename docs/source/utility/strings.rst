.. strings

String Manipulators
======================

The following are some common functions used to manipulate and interact with strings (or, more often, string_views, due to the performance gain).

String Split
-----------------

.. doxygenfunction:: split

String Trim
----------------

.. doxygenfunction:: trim(std::string_view)

.. doxygenfunction:: trim(std::string_view, const std::string &)


Insensitive Equals
---------------------

.. doxygenfunction:: insensitiveEquals