#pragma  once

#include "gtest/gtest.h"
#include "TestParams.h"

#include "Utils.h"
#include "wxOdbc3GoogleTest.h"

class wxDbConnectInf;
class wxDb;

namespace wxOdbc3Test
{
	class DbTest : public ::testing::TestWithParam<SOdbcInfo>
	{
	
	public:
		static void SetUpTestCase() {};
		static void TearDownTestCase() {};

	protected:
		virtual void SetUp();
		virtual void TearDown();

		wxDbConnectInf* m_pConnectInf;
		wxDb*	m_pDb;
		SOdbcInfo m_odbcInfo;
	};

	static std::vector<SOdbcInfo> test;

	INSTANTIATE_TEST_CASE_P(
		ParametrizedOdbc,
		DbTest,
		::testing::ValuesIn(g_odbcInfos));

}