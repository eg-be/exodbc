/*!
* \file ColumnDescription.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 04.05.2017
* \brief Source file for ColumnDescription.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "ColumnDescription.h"

// Same component headers
#include "AssertionException.h"

// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	// Class ColumnDescription
	// ==========================
	ColumnDescription::ColumnDescription(ConstSqlStmtHandlePtr pStmt, SQLUSMALLINT columnNr, const SqlInfoProperties& props)
		: m_sqlType(SQL_UNKNOWN_TYPE)
		, m_charSize(0)
		, m_decimalDigits(0)
		, m_nullable(SQL_NULLABLE_UNKNOWN)
	{
		exASSERT(pStmt);
		exASSERT(pStmt->IsAllocated());
		exASSERT(columnNr >= 1);

		SQLUSMALLINT maxColName = props.GetMaxColumnNameLen();
		std::unique_ptr<SQLAPICHARTYPE[]> nameBuffer(new SQLAPICHARTYPE[maxColName + 1]);
		SQLRETURN ret = SQLDescribeCol(pStmt->GetHandle(), columnNr, nameBuffer.get(), maxColName + 1, NULL, &m_sqlType, &m_charSize, &m_decimalDigits, &m_nullable);
		THROW_IFN_SUCCEEDED(SQLDescribeCol, ret, SQL_HANDLE_STMT, pStmt->GetHandle());

		m_name = SQLAPICHARPTR_TO_EXODBCSTR(nameBuffer.get());
	}
}
