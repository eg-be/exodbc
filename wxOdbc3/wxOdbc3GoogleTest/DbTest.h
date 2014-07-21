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

	// TODO: We can keep this as an example for the params, but we must test the forward-only otherwise.
	// TODO: Forward only only affects moving through row (?)
	INSTANTIATE_TEST_CASE_P(
		MySql_3_51,
		DbTest,
		::testing::Values(	SOdbcInfo(DSN_MYSQL_3_51, USER_MYSQL, PASS_MYSQL, SOdbcInfo::forwardOnlyCursors),
							SOdbcInfo(DSN_MYSQL_3_51, USER_MYSQL, PASS_MYSQL, SOdbcInfo::notForwardOnlyCursors)));

	INSTANTIATE_TEST_CASE_P(
		MySql_5_2,
		DbTest,
		::testing::Values(	SOdbcInfo(DSN_MYSQL_5_2, USER_MYSQL, PASS_MYSQL, SOdbcInfo::forwardOnlyCursors),
		SOdbcInfo(DSN_MYSQL_5_2, USER_MYSQL, PASS_MYSQL, SOdbcInfo::notForwardOnlyCursors)));

	INSTANTIATE_TEST_CASE_P(
		IBM_DB2,
		DbTest,
		::testing::Values(	SOdbcInfo(DSN_DB2, USER_DB2, PASS_DB2, SOdbcInfo::forwardOnlyCursors),
		SOdbcInfo(DSN_DB2, USER_DB2, PASS_DB2, SOdbcInfo::notForwardOnlyCursors)));

}