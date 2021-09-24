#pragma once

#include <iostream>

#ifndef _ZARKS_LOG_NEWLINE
    #define _ZARKS_LOG_NEWLINE std::endl;
#endif // ifndef ZARKS_LOG_END_MESSAGE

#ifdef ZARKS_DEBUG
    #define LOG_DEBUG( msg ) std::cout << "\033[1m[DEBUG]\033[0m " << msg << _ZARKS_LOG_NEWLINE;
#else
    #define LOG_DEBUG( msg )
#endif // ifndef ZARKS_DEBUG
