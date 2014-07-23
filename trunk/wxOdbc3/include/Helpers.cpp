/*!
 * \file Helpers.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 23.07.2014
 * 
 * [Brief CPP-file description]
 */ 

//#include "stdafx.h"

// Own header
#include "Helpers.h"

// Same component headers
// Other headers
#include <iostream>

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------
void exOnAssert(const char* file, int line, const char* function, const char* condition, const char* msg)
{
	std::wcerr << L"ASSERTION failure!" << std::endl;
	std::wcerr << L" File:      " << file << std::endl;
	std::wcerr << L" Line:      " << line << std::endl;
	std::wcerr << L" Function:  " << function << std::endl;
	std::wcerr << L" Condition: " << condition << std::endl;
	if(msg)
	{
		std::wcerr << L" Msg:       " << msg << std::endl;
	}
}


// Interfaces
// ----------

