.. convert

Converters
==============


.. doxygenfunction:: convert(std::string_view sv)

Accepted Conversion Types
------------------------------

The following types are accepted template parameters:

* Any integral type (int, short, size_t, long, etc)
* Booleans
* Double and float
* std::string
* char
* std:::vector of any accepted type

Details
----------

.. toctree::
	converter_core
	converter_vector
	:maxdepth: 1


Basic Usage
----------------------



.. code-block:: cpp

	int a = convert<int>("2"); //a = 2
	double b = convert<double>("-2"); //b=-2.0
	auto c = convert<std::vector<int>>("[5 8 7 87]"," "); // c = {5,8,7,87};
	std::string d = convert<std::string>("hi"); // d = "hi"; 