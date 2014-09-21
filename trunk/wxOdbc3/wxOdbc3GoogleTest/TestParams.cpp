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
			<< L"; Password: " << oi.m_password.c_str();
		std::string s;

		// TODO: Resolve with ticket #44 #53
		//	eli::w2mbNoThrow(wos.str(), s);
		// hack around
		std::wstring ws = wos.str();
		std::stringstream ss;
		for(size_t i = 0; i < ws.length(); i++)
		{
			char c = (char) ws[i];
			ss << c;
		}
		s = ss.str();
		// end hack

		return os << s.c_str();
	}


	::std::wostream& operator<<(::std::wostream& wos, const SOdbcInfo& oi) {
		wos << L"DSN: " << oi.m_dsn.c_str() 
			<< L"; Username: " << oi.m_username.c_str()
			<< L"; Password: " << oi.m_password.c_str();

		return wos;
	}


	// Interfaces
	// ----------
	
	 
} // namespace exodbc