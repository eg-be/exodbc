#include "gtest/gtest.h"

class wxDbConnectInf;


namespace wxOdbc3Test
{
	typedef wxDbConnectInf* CreateConnectInfFunc();


	wxDbConnectInf* CreateMySqlConnectInf();

	struct SOdbcInfo
	{
		SOdbcInfo(int _a, int _b) {};
		int a;
		int b;
	};

	typedef SOdbcInfo* CreateOcbcInfFunc();
	SOdbcInfo* CreateMySqlOdbcInf();

//	class DbTest : public ::testing::TestWithParam<CreateConnectInfFunc*>
	class DbTest : public ::testing::TestWithParam<SOdbcInfo>
	{
	protected:
		virtual void SetUp();
		virtual void TearDown();

		//private:
		void OpenTest(const std::wstring dsn, const std::wstring user, const std::wstring pass, bool forwardOnlyCursors);
		void OpenTest(wxDbConnectInf* pConnectInf);

		wxDbConnectInf* m_pConnectInf;
	};

	INSTANTIATE_TEST_CASE_P(
		OnTheFlyAndPreCalculated,
		DbTest,
		::testing::Values(SOdbcInfo(1,2)));
// 		::testing::Values(&CreateMySqlConnectInf));
}