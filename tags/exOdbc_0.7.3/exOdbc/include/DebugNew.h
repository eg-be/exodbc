/*!
* \file DebugNew.h
* \author Elias Gerber <eg@elisium.ch>
* \date 21.09.2014
* \copyright wxWindows Library Licence, Version 3.1
*
* Defines new to something else, so we can track memory leak.
* See: http://msdn.microsoft.com/en-us/library/974tc9t1.aspx
*/

#pragma once
#ifndef DEBUGNEW_H
#define DEBUGNEW_H

#ifdef _DEBUG
	#include <crtdbg.h>
	#define DEBUG_NEW   new( _NORMAL_BLOCK, __FILE__, __LINE__)
	// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
	//allocations to be of _CLIENT_BLOCK type
	#define new DEBUG_NEW
#else
	#define DEBUG_NEW
#endif // _DEBUG

// Same component headers
// Other headers
// System headers

// Forward declarations
// --------------------

// Globals
// -------

// Structs
// -------

// Classes
// -------

#endif // DEBUGNEW_H
