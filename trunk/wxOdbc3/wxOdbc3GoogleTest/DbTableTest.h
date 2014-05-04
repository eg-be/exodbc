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
		virtual void SetUp();
		virtual void TearDown();

		//private:
		//void TestCharTypes(CharTypesTable* pTable);
		//void OpenTest(const std::wstring dsn, const std::wstring user, const std::wstring pass, bool forwardOnlyCursors);
	};

	INSTANTIATE_TEST_CASE_P(
		ReadTypes_MySql_3_51,
		DbTableTest,
		::testing::Values(	SOdbcInfo(MYSQL_3_51_DSN, MYSQL_USER, MYSQL_PASS, SOdbcInfo::forwardOnlyCursors)));

	INSTANTIATE_TEST_CASE_P(
		ReadTypes_MySql_5_2,
		DbTableTest,
		::testing::Values(	SOdbcInfo(MYSQL_5_2_DSN, MYSQL_USER, MYSQL_PASS, SOdbcInfo::forwardOnlyCursors)));

	INSTANTIATE_TEST_CASE_P(
		ReadTypes_IBM_DB_2,
		DbTableTest,
		::testing::Values(	SOdbcInfo(DB2_DSN, DB2_USER, DB2_PASS, SOdbcInfo::forwardOnlyCursors)));

}
