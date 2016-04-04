/*!
 * \file TestParams.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright GNU Lesser General Public License Version 3
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
	struct TestParams
	{
		TestParams()
		{ }

		TestParams(const std::wstring& dsn, const std::wstring& username, const std::wstring& password, test::Case namesCase = test::Case::LOWER)
			: m_dsn(dsn)
			, m_username(username)
			, m_password(password)
			, m_namesCase(namesCase)
		{};

		TestParams(const std::wstring& connectionString, test::Case namesCase)
			: m_connectionString(connectionString)
			, m_namesCase(namesCase)
		{};

		void Load(const boost::filesystem::wpath& settingsFile);

		bool HasConnectionString() const noexcept { return m_connectionString.length() > 0; }

		bool HasDsn() const noexcept { return m_dsn.length() > 0; }

		bool IsUsable() const noexcept { return HasConnectionString() || HasDsn(); };

		std::wstring m_dsn;
		std::wstring m_username;
		std::wstring m_password;
		std::wstring m_connectionString;
		test::Case m_namesCase;
		boost::log::trivial::severity_level m_logSeverity;
		bool m_createDb;
	};

	::std::ostream& operator<<(::std::ostream& os, const TestParams& oi);
	::std::wostream& operator<<(::std::wostream& os, const TestParams& oi);

} // namespace exodbc

#endif // TESTPARAMS_H
