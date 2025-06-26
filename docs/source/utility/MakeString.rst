.. makestring

MakeString
++++++++++++++++++

Generally speaking, converting things to strings is quite easy. The annoying thing, however, is that it's not universal:

* Numeric types can call std::to_string
* Chars can be used as arguments in a string constructor
* string_views must be *explicitly* constructed into strings (due to copy semantics)
* Booleans can cast to ints

In addition, converting vectors into strings is not natively supported. 

MakeString is a template function interface to provide a standard, unified way to convert entities into strings. 

.. doxygenfunction:: MakeString(T obj)



Internal Functions
=====================

.. doxygenstruct:: MakeStringStruct



Specialisations
//////////////////

.. doxygenstruct:: MakeStringStruct< bool, void >

.. doxygenstruct:: MakeStringStruct< char, void >
	
.. doxygenstruct:: MakeStringStruct< std::string, void >

.. doxygenstruct:: MakeStringStruct< std::string_view, void >


Vector Strings
====================

.. doxygenstruct:: MakeStringStruct< std::vector< T_Inner >, void >