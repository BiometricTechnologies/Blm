
#ifndef LOGGING_H
#define LOGGING_H

#include <windows.h>

// Other useful files
#include <strsafe.h>

#ifdef IDENTAZONEAP_EXPORTS
#define IDENTAZONEAP_API __declspec(dllexport)
#else
#define IDENTAZONEAP_API __declspec(dllimport)
#endif

extern CRITICAL_SECTION DllCriticalSection;
/*
 ---------------------------------------------------------------------------------------------
 DLL Definitions
 ---------------------------------------------------------------------------------------------
 */

#define		IDENTAZONEAP_VERSION		1L


/*
 ---------------------------------------------------------------------------------------------
 Utility Functions
 ---------------------------------------------------------------------------------------------
 */


/* For logging */
BOOLEAN open_log(void);
BOOLEAN close_log(void);
int iz_log(const char *format, ...);


#endif