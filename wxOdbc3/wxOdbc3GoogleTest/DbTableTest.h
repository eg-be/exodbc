#include "gtest/gtest.h"

namespace wxOdbc3Test
{
	class CharTypesTable;

	class DbTableTest : public ::testing::Test
	{
	protected:
		virtual void SetUp() {};
		virtual void TearDown() {};

		private:
		void TestCharTypes(CharTypesTable* pTable);
		//void OpenTest(const std::wstring dsn, const std::wstring user, const std::wstring pass, bool forwardOnlyCursors);
	};

}
