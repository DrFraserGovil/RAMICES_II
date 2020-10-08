#include <stdio.h>
#include <iostream>

#include "Logger.h"
#include "Options.h"


int main(int argc, char** argv)
{
	Options options;
	
	initialiseLogger(&options);
	
	
	shutDownLogger();
	return 0;
}
