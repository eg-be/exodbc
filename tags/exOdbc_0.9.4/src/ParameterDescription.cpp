/*!
* \file ParameterDescription.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 04.05.2017
* \brief Source file for ParameterDescription.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "ParameterDescription.h"

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
	// Class ParameterDescription
	// ==========================
	ParameterDescription::ParameterDescription(ConstSqlStmtHandlePtr pStmt, SQLUSMALLINT paramNr)
		: m_sqlType(SQL_UNKNOWN_TYPE)
		, m_charSize(0)
		, m_decimalDigits(0)
		, m_nullable(SQL_NULLABLE_UNKNOWN)
	{
		exASSERT(pStmt);
		exASSERT(pStmt->IsAllocated());
		exASSERT(paramNr >= 1);

		SQLRETURN ret = SQLDescribeParam(pStmt->GetHandle(), paramNr, &m_sqlType, &m_charSize, &m_decimalDigits, &m_nullable);
		THROW_IFN_SUCCESS(SQLDescribeParam, ret, SQL_HANDLE_STMT, pStmt->GetHandle());
	}
}
