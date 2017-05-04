/*!
* \file SetDescriptionFieldWrapperTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 04.05.2017
* \copyright GNU Lesser General Public License Version 3
* 
* [Brief CPP-file description]
*/ 

// Own header
#include "SetDescriptionFieldWrapperTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

// Other headers
#include "exodbc/SetDescriptionFieldWrapper.h"

// Debug
#include "DebugNew.h"

using namespace exodbc;

namespace exodbctest
{


	// Static consts
	// -------------

	// Construction
	// -------------

	// Destructor
	// -----------

	// Implementation
	// --------------
	TEST_F(SetDescriptionFieldWrapperTest, SetDescriptionField)
	{
		// Test by setting the description of a numeric column parameter
		
		// Allocate a valid statement handle from an open connection handle
		DatabasePtr pDb = OpenTestDb();
		SqlStmtHandlePtr pHStmt = std::make_shared<SqlStmtHandle>();
		ASSERT_NO_THROW(pHStmt->AllocateWithParent(pDb->GetSqlDbcHandle()));

		// Open statement by doing some operation on it
		std::string sqlstmt;
		if (pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			sqlstmt = boost::str(boost::format(u8"SELECT * FROM %s WHERE %s = 3") % GetTableName(TableId::CHARTYPES) % GetIdColumnName(TableId::CHARTYPES));
		}
		else
		{
			sqlstmt = boost::str(boost::format(u8"SELECT * FROM exodbc.%s WHERE %s = 3") % GetTableName(TableId::CHARTYPES) % GetIdColumnName(TableId::CHARTYPES));
		}
		if (g_odbcInfo.m_namesCase == Case::UPPER)
		{
			boost::algorithm::to_upper(sqlstmt);
		}
		SQLRETURN ret = SQLExecDirect(pHStmt->GetHandle(), (SQLAPICHARTYPE*)EXODBCSTR_TO_SQLAPICHARPTR(sqlstmt), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		// Get Descriptor for a param
		SqlDescHandle hDesc(pHStmt, SqlDescHandle::RowDescriptorType::PARAM);

		if (pDb->GetDbms() != DatabaseProduct::ACCESS)
		{
			SQL_NUMERIC_STRUCT num;
			EXPECT_NO_THROW(SetDescriptionFieldWrapper::SetDescriptionField(hDesc, 3, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC));
			EXPECT_NO_THROW(SetDescriptionFieldWrapper::SetDescriptionField(hDesc, 3, SQL_DESC_PRECISION, (SQLPOINTER)18));
			EXPECT_NO_THROW(SetDescriptionFieldWrapper::SetDescriptionField(hDesc, 3, SQL_DESC_SCALE, (SQLPOINTER)10));
			EXPECT_NO_THROW(SetDescriptionFieldWrapper::SetDescriptionField(hDesc, 3, SQL_DESC_DATA_PTR, (SQLPOINTER)&num));
		}
	}



} // namespace exodbctest
