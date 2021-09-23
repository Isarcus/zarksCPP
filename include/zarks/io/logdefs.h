#pragma once

#include <iostream>

#ifdef ZARKS_DEBUG
#define LOG_DEBUG( msg ) std::cout << "\033[1m[DEBUG]\033[0m " << msg << '\n';
#else
#define LOG_DEBUG( msg )
#endif
