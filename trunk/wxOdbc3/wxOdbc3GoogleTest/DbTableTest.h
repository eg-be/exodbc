#include "gtest/gtest.h"

#include "TestParams.h"
#include "wxOdbc3GoogleTest.h"

class wxDb;
class wxDbConnectInf;


namespace wxOdbc3Test
{
	class DbTableTest : public ::testing::TestWithParam<SOdbcInfo>
	{
	public:
		static void SetUpTestCase() {};
		static void TearDownTestCase() {};

	protected:
		wxDb*	m_pDb;
		wxDbConnectInf* m_pConnectInf;
		SOdbcInfo m_odbcInfo;
		virtual void SetUp();
		virtual void TearDown();
	};

	INSTANTIATE_TEST_CASE_P(
		ParametrizedOdbc,
		DbTableTest,
		::testing::ValuesIn(g_odbcInfos));
}
