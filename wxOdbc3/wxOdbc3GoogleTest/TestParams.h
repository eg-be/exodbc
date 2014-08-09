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
// Other headers
#include "exOdbc.h"
#include "Utils.h"

// System headers

// Forward declarations
// --------------------

// Structs
// -------

// Classes
// -------
namespace exOdbcTest
{
	struct SOdbcInfo
	{
		enum CursorType
		{
			forwardOnlyCursors,
			notForwardOnlyCursors
		};

		SOdbcInfo()
		{ }

		SOdbcInfo(const std::wstring& dsn, const std::wstring& username, const std::wstring& password, CursorType cursorType, exodbc::OdbcVersion odbcVersion) 
			: m_dsn(dsn)
			, m_username(username)
			, m_password(password)
			, m_cursorType(cursorType)
			, m_odbcVersion(odbcVersion)
		{};
		std::wstring m_dsn;
		std::wstring m_username;
		std::wstring m_password;
		exodbc::OdbcVersion m_odbcVersion;
		CursorType m_cursorType;
	};

	::std::ostream& operator<<(::std::ostream& os, const SOdbcInfo& oi);

}

#endif // TESTPARAMS_H
