/*!
 * \file TestParams.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright GNU Lesser General Public License Version 3
 * 
 * [Brief Header-file description]
 */ 

#pragma once

// Same component headers

// Other headers
#include "exOdbc.h"

// System headers

// Forward declarations
// --------------------

// Structs
// -------

// Classes
// -------
namespace exodbctest
{
	enum class Case
	{
		UPPER,	///< Tables will be created using all UPPERCASE letters for table- and column-names
		LOWER	///< Tables will be created using all lowercase letters for table- and column-names
	};

	struct TestParams
	{
		TestParams()
		{ }

		TestParams(const std::wstring& dsn, const std::wstring& username, const std::wstring& password, Case namesCase = Case::LOWER)
			: m_dsn(dsn)
			, m_username(username)
			, m_password(password)
			, m_namesCase(namesCase)
		{};

		TestParams(const std::wstring& connectionString, Case namesCase)
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
		Case m_namesCase;
		bool m_createDb;
	};

	::std::ostream& operator<<(::std::ostream& os, const TestParams& oi);
	::std::wostream& operator<<(::std::wostream& os, const TestParams& oi);

} // namespace exodbctest

