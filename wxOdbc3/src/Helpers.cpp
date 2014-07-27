/*!
 * \file Helpers.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 23.07.2014
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "Helpers.h"

// Same component headers
// Other headers
#include <iostream>
#include <sstream>

// Static consts
// -------------
namespace exodbc
{

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------
void exOnAssert(const char* file, int line, const char* function, const char* condition, const char* msg)
{
	std::wstringstream ws;
	ws 	<< L"ASSERTION failure!" << std::endl;
	ws	<< L" File:      " << file << std::endl;
	ws	<< L" Line:      " << line << std::endl;
	ws	<< L" Function:  " << function << std::endl;
	ws	<< L" Condition: " << condition << std::endl;
	if(msg)
	{
		ws << L" Msg:       " << msg << std::endl;
	}
	BOOST_LOG_TRIVIAL(error) << ws.str();
}

}

// Interfaces
// ----------

