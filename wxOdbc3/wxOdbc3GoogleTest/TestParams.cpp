/*!
 * \file TestParams.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "TestParams.h"

// Same component headers
// Other headers

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------
namespace exodbc
{
	::std::ostream& operator<<(::std::ostream& os, const SOdbcInfo& oi) {
		std::wstringstream wos;
		wos << L"DSN: " << oi.m_dsn.c_str() 
			<< L"; Username: " << oi.m_username.c_str()
			<< L"; Password: " << oi.m_password.c_str() 
			<< L"; Forward-Only Cursor: " << (oi.m_cursorType == SOdbcInfo::forwardOnlyCursors);
		std::string s;
		eli::w2mbNoThrow(wos.str(), s);
		return os << s.c_str();
	}


	// Interfaces
	// ----------
	
	 
} // namespace exodbc