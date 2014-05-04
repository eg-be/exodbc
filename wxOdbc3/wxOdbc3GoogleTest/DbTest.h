#include "gtest/gtest.h"
#include "TestParams.h"
#include "MySqlParams.h"
#include "Db2Params.h"

#include "Utils.h"

class wxDbConnectInf;

namespace wxOdbc3Test
{
	class DbTest : public ::testing::TestWithParam<SOdbcInfo>
	{
	protected:
		virtual void SetUp();
		virtual void TearDown();

		wxDbConnectInf* m_pConnectInf;
		bool m_forwardOnlyCursors;
	};


	INSTANTIATE_TEST_CASE_P(
		MySql_3_51,
		DbTest,
		::testing::Values(	SOdbcInfo(MYSQL_3_51_DSN, MYSQL_USER, MYSQL_PASS, SOdbcInfo::forwardOnlyCursors),
							SOdbcInfo(MYSQL_3_51_DSN, MYSQL_USER, MYSQL_PASS, SOdbcInfo::notForwardOnlyCursors)));

	INSTANTIATE_TEST_CASE_P(
		MySql_5_2,
		DbTest,
		::testing::Values(	SOdbcInfo(MYSQL_5_2_DSN, MYSQL_USER, MYSQL_PASS, SOdbcInfo::forwardOnlyCursors),
		SOdbcInfo(MYSQL_5_2_DSN, MYSQL_USER, MYSQL_PASS, SOdbcInfo::notForwardOnlyCursors)));

	INSTANTIATE_TEST_CASE_P(
		IBM_DB2,
		DbTest,
		::testing::Values(	SOdbcInfo(DB2_DSN, DB2_USER, DB2_PASS, SOdbcInfo::forwardOnlyCursors),
		SOdbcInfo(DB2_DSN, DB2_USER, DB2_PASS, SOdbcInfo::notForwardOnlyCursors)));

}