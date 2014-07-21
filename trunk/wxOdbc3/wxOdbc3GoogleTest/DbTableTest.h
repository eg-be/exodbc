#include "gtest/gtest.h"

#include "TestParams.h"
#include "MySqlParams.h"
#include "Db2Params.h"

class wxDb;
class wxDbConnectInf;


namespace wxOdbc3Test
{
	class DbTableTest : public ::testing::TestWithParam<SOdbcInfo>
	{
	public:
		static void SetUpTestCase();
//		static void TearDownTestCase();

	protected:
		wxDb*	m_pDb;
		wxDbConnectInf* m_pConnectInf;
		SOdbcInfo m_odbcInfo;
		virtual void SetUp();
		virtual void TearDown();

		//private:
		//void TestCharTypes(CharTypesTable* pTable);
		//void OpenTest(const std::wstring dsn, const std::wstring user, const std::wstring pass, bool forwardOnlyCursors);
	};

	INSTANTIATE_TEST_CASE_P(
		MySql_3_51,
		DbTableTest,
		::testing::Values(	SOdbcInfo(DSN_MYSQL_3_51, USER_MYSQL, PASS_MYSQL, SOdbcInfo::forwardOnlyCursors)));

	INSTANTIATE_TEST_CASE_P(
		MySql_5_2,
		DbTableTest,
		::testing::Values(	SOdbcInfo(DSN_MYSQL_5_2, USER_MYSQL, PASS_MYSQL, SOdbcInfo::forwardOnlyCursors)));

	INSTANTIATE_TEST_CASE_P(
		IBM_DB2,
		DbTableTest,
		::testing::Values(	SOdbcInfo(DSN_DB2, USER_DB2, PASS_DB2, SOdbcInfo::forwardOnlyCursors)));

}
