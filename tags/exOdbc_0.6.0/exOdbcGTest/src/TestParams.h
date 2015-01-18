/*!
 * \file TestParams.h
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef TESTPARAMS_H
#define TESTPARAMS_H

// Same component headers
#include "TestTables.h"

// Other headers
#include "exOdbc.h"

// System headers

// Forward declarations
// --------------------

// Structs
// -------

// Classes
// -------
namespace exodbc
{
	struct SOdbcInfo
	{
		SOdbcInfo()
		{ }

		SOdbcInfo(const std::wstring& dsn, const std::wstring& username, const std::wstring& password, TestTables::NameCase namesCase = TestTables::NC_LOWER) 
			: m_dsn(dsn)
			, m_username(username)
			, m_password(password)
			, m_namesCase(namesCase)
		{};
		std::wstring m_dsn;
		std::wstring m_username;
		std::wstring m_password;
		TestTables::NameCase m_namesCase;
	};

	::std::ostream& operator<<(::std::ostream& os, const SOdbcInfo& oi);
	::std::wostream& operator<<(::std::wostream& os, const SOdbcInfo& oi);

} // namespace exodbc

#endif // TESTPARAMS_H
