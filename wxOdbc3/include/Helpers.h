/*!
 * \file Helpers.h
 * \author Elias Gerber <egerber@gmx.net>
 * \date 23.07.2014
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef HELPERS_H
#define HELPERS_H

// Same component headers
// Other headers
// System headers
#include <string>

// Forward declarations
// --------------------

extern void exOnAssert(const wchar_t* file, int line, const char* function, const char* condition, const char* msg);

// Macros available for everybody
// ----------------

/*  size of statically declared array */
#define EXSIZEOF(array)   (sizeof(array)/sizeof(array[0]))

// A much simpler form of the exASSERT-Macro
#ifdef _DEBUG
#define exASSERT_MSG(cond, msg)										\
do {																\
	if ( !(cond) )  {												\
		exOnAssert(__TFILE__, __LINE__, __FUNCTION__,#cond,msg);	\
		__debugbreak();												\
	}                                                               \
} while ( 0 )
#else
#define exASSERT_MSG(cond, msg)
#endif

// a version without any additional message, don't use unless condition
// itself is fully self-explanatory
#define exASSERT(cond) exASSERT_MSG(cond, (const char*)NULL)

// exFAIL must become something that will always trigger something, not depending on any flags
#define exFAIL_MSG(msg)												\
do {																\
	std::wcerr << msg << std::endl;									\
} while ( 0 )

// Structs
// -------

// Classes
// -------



#endif // HELPERS_H
