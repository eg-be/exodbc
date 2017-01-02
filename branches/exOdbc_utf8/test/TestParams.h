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
#include "exodbc/exOdbc.h"
#include "boost/filesystem.hpp"

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

		TestParams(const std::string& dsn, const std::string& username, const std::string& password, Case namesCase = Case::LOWER)
			: m_dsn(dsn)
			, m_username(username)
			, m_password(password)
			, m_namesCase(namesCase)
		{};

		TestParams(const std::string& connectionString, Case namesCase)
			: m_connectionString(connectionString)
			, m_namesCase(namesCase)
		{};

		void Load(const boost::filesystem::path& settingsFile, std::vector<std::string>& skipNames);

		bool HasConnectionString() const noexcept { return m_connectionString.length() > 0; }

		bool HasDsn() const noexcept { return m_dsn.length() > 0; }

		bool IsUsable() const noexcept { return HasConnectionString() || HasDsn(); };

		std::string m_dsn;
		std::string m_username;
		std::string m_password;
		std::string m_connectionString;
		Case m_namesCase;
		bool m_createDb;
	};

	::std::ostream& operator<<(::std::ostream& os, const TestParams& oi);
	::std::wostream& operator<<(::std::wostream& os, const TestParams& oi);

} // namespace exodbctest

