#pragma once

#include <iostream>

// How to terminate each log message
#ifndef _ZARKS_LOG_NEWLINE
    #define _ZARKS_LOG_NEWLINE std::endl;
#endif

// Catch-all defines for directing all logging output to stdout or stderr
#ifdef ZARKS_LOG_STDOUT_ALL
    #define _ZARKS_LOG_DEBUG_OUT std::cout
    #define _ZARKS_LOG_INFO_OUT std::cout
    #define _ZARKS_LOG_WARN_OUT std::cout
    #define _ZARKS_LOG_ERROR_OUT std::cout
#elif defined ZARKS_LOG_STDERR_ALL
    #define _ZARKS_LOG_DEBUG_OUT std::cerr
    #define _ZARKS_LOG_INFO_OUT std::cerr
    #define _ZARKS_LOG_WARN_OUT std::cerr
    #define _ZARKS_LOG_ERROR_OUT std::cerr
#endif // ifdef ZARKS_LOG_STDOUT_ALL

// If not defined above, these are the default output streams for each logging level
#ifndef _ZARKS_LOG_DEBUG_OUT
    #define _ZARKS_LOG_DEBUG_OUT std::cout
#endif
#ifndef _ZARKS_LOG_INFO_OUT
    #define _ZARKS_LOG_INFO_OUT std::cout
#endif
#ifndef _ZARKS_LOG_WARN_OUT
    #define _ZARKS_LOG_WARN_OUT std::cerr
#endif
#ifndef _ZARKS_LOG_ERROR_OUT
    #define _ZARKS_LOG_ERROR_OUT std::cerr
#endif

//              //
// VERBOSE LOGS //
//              //

#ifndef LOG_DEBUG
#ifdef ZARKS_DEBUG
    #define LOG_DEBUG( msg ) _ZARKS_LOG_DEBUG_OUT << "\033[96m[DEBUG]\033[0m " << msg << _ZARKS_LOG_NEWLINE
#else
    #define LOG_DEBUG( msg )
#endif // ifdef ZARKS_DEBUG
#endif // ifndef LOG_DEBUG

//               //
// STANDARD LOGS //
//               //

#ifndef LOG_INFO
    #define LOG_INFO( msg ) _ZARKS_LOG_INFO_OUT<< "[ INFO] " << msg << _ZARKS_LOG_NEWLINE
#endif

#ifndef LOG_WARN
    #define LOG_WARN( msg ) _ZARKS_LOG_WARN_OUT << "\033[1;33m[ WARN]\033[0m " << msg << _ZARKS_LOG_NEWLINE
#endif

#ifndef LOG_ERROR
    #define LOG_ERROR( msg ) _ZARKS_LOG_ERROR_OUT << "\033[1;31m[ERROR]\033[0m " << msg << _ZARKS_LOG_NEWLINE
#endif
