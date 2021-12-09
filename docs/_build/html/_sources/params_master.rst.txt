.. params_master

###################################
Global & Runtime Parameters
###################################

Global variables are horrible, but in a project with this many twiddly dials, also fairly unavoidable. 

To avoid having truly "global parameters", we package the vast majority of our values into a GlobalParameters object, which is then passed around by reference. This object makes extensive use of the `JSL::Argument <https://jackstandardlibrary.readthedocs.io/en/latest/argument.html>`_ object, which is included as a submodule.


.. toctree::
	params_global
	params_list_super
	params_list
	enums
	:maxdepth: 2
	:caption: Contents:

