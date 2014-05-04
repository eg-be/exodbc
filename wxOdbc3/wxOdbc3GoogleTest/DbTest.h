#include "gtest/gtest.h"

#include "MySqlParams.h"
#include "Db2Params.h"

class wxDbConnectInf;

namespace wxOdbc3Test
{

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

	class DbTest : public ::testing::TestWithParam<SOdbcInfo>
	{
	protected:
		virtual void SetUp();
		virtual void TearDown();

		//private:
		void OpenTest(const std::wstring dsn, const std::wstring user, const std::wstring pass, bool forwardOnlyCursors);
		void OpenTest(wxDbConnectInf* pConnectInf);

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