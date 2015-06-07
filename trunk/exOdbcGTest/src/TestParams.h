/*!
 * \file TestParams.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright wxWindows Library Licence, Version 3.1
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

		SOdbcInfo(const std::wstring& dsn, const std::wstring& username, const std::wstring& password, test::Case namesCase = test::Case::LOWER)
			: m_dsn(dsn)
			, m_username(username)
			, m_password(password)
			, m_namesCase(namesCase)
		{};

		SOdbcInfo(const std::wstring& connectionString, test::Case namesCase)
			: m_connectionString(connectionString)
		{};

		bool HasConnectionString() const { return m_connectionString.length() > 0; }

		std::wstring m_dsn;
		std::wstring m_username;
		std::wstring m_password;
		std::wstring m_connectionString;
		test::Case m_namesCase;
	};

	::std::ostream& operator<<(::std::ostream& os, const SOdbcInfo& oi);
	::std::wostream& operator<<(::std::wostream& os, const SOdbcInfo& oi);

} // namespace exodbc

#endif // TESTPARAMS_H
