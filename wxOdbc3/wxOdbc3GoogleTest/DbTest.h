#include "gtest/gtest.h"

#include "MySqlParams.h"
#include "Db2Params.h"

#include <iostream>
#include <windows.h>

class wxDbConnectInf;

namespace wxOdbc3Test
{

	std::string w2mb(const std::wstring& wstr, unsigned int codepage = CP_UTF8)
	{
		int size_needed = WideCharToMultiByte(codepage, 0, &wstr[0], (int) wstr.size(), NULL, 0, NULL, NULL);
		if(size_needed == 0)
			return std::string("");
		std::string strTo(size_needed, 0);
		if (WideCharToMultiByte(codepage, 0, wstr.c_str(), (int) wstr.size(), &strTo[0], size_needed, NULL, NULL) < 1)
		{
			return "Failed To Convert";
		}
		return strTo;
		return "";
	}

	struct SOdbcInfo
	{
		enum CursorType
		{
			forwardOnlyCursors,
			notForwardOnlyCursors
		};

		SOdbcInfo(const std::wstring& dsn, const std::wstring& username, const std::wstring& password, CursorType cursorType) 
			: m_dsn(dsn)
			, m_username(username)
			, m_password(password)
			, m_cursorType(cursorType)
		{};
		std::wstring m_dsn;
		std::wstring m_username;
		std::wstring m_password;
		CursorType m_cursorType;
	};

	::std::ostream& operator<<(::std::ostream& os, const SOdbcInfo& oi) {
		std::wstringstream wos;
		wos << L"DSN: " << oi.m_dsn.c_str() 
			<< L"; Username: " << oi.m_username.c_str()
			<< L"; Password: " << oi.m_password.c_str() 
			<< L"; Forward-Only Cursor: " << (oi.m_cursorType == SOdbcInfo::forwardOnlyCursors);
		return os << w2mb(wos.str());
	}

	class DbTest : public ::testing::TestWithParam<SOdbcInfo>
	{
	protected:
		virtual void SetUp();
		virtual void TearDown();

		wxDbConnectInf* m_pConnectInf;
		bool m_forwardOnlyCursors;
	};


	INSTANTIATE_TEST_CASE_P(
		Open_MySql_3_51,
		DbTest,
		::testing::Values(	SOdbcInfo(MYSQL_3_51_DSN, MYSQL_USER, MYSQL_PASS, SOdbcInfo::forwardOnlyCursors),
							SOdbcInfo(MYSQL_3_51_DSN, MYSQL_USER, MYSQL_PASS, SOdbcInfo::notForwardOnlyCursors)));

	INSTANTIATE_TEST_CASE_P(
		Open_MySql_5_2,
		DbTest,
		::testing::Values(	SOdbcInfo(MYSQL_5_2_DSN, MYSQL_USER, MYSQL_PASS, SOdbcInfo::forwardOnlyCursors),
		SOdbcInfo(MYSQL_5_2_DSN, MYSQL_USER, MYSQL_PASS, SOdbcInfo::notForwardOnlyCursors)));

	INSTANTIATE_TEST_CASE_P(
		Open_DB2,
		DbTest,
		::testing::Values(	SOdbcInfo(DB2_DSN, DB2_USER, DB2_PASS, SOdbcInfo::forwardOnlyCursors),
		SOdbcInfo(DB2_DSN, DB2_USER, DB2_PASS, SOdbcInfo::notForwardOnlyCursors)));

}