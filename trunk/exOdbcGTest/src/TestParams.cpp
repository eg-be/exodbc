/*!
 * \file TestParams.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * \copyright wxWindows Library Licence, Version 3.1
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "TestParams.h"

// Same component headers
// Other headers

// Debug
#include "DebugNew.h"

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
			<< L"; Names: " << (oi.m_namesCase == TestTables::NameCase::LOWER ? L"lowercase" : L"uppercase");
		std::string s;

		// \todo Resolve with ticket #44 #53 - ugly conversion (but okay here, we know its only ascii)
		std::wstring ws = wos.str();
		std::stringstream ss;
		for(size_t i = 0; i < ws.length(); i++)
		{
			char c = (char) ws[i];
			ss << c;
		}
		s = ss.str();

		return os << s.c_str();
	}


	::std::wostream& operator<<(::std::wostream& wos, const SOdbcInfo& oi) {
		wos << L"DSN: " << oi.m_dsn.c_str()
			<< L"; Username: " << oi.m_username.c_str()
			<< L"; Password: " << oi.m_password.c_str()
			<< L"; Names: " << (oi.m_namesCase == TestTables::NameCase::LOWER ? L"lowercase" : L"uppercase");


		return wos;
	}


	// Interfaces
	// ----------
	
	 
} // namespace exodbc