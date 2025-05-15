#include "Log.h"

void structlog::SetLevel(int i)
{
	switch(i){
		case 0: 
			level=ERROR; break;
		case 1:
			level=WARN;break;
		case 2:
			level=INFO;break;
		case 3:
			level=DEBUG;break;
		default:
			throw std::runtime_error(std::to_string(i) + "is not a valid logging level");
	}
}

structlog LOGCFG = {};

