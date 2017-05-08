/*!
* \file SpecialColumnInfo.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 01.05.2017
* \brief Source file for SpecialColumnInfo.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "SpecialColumnInfo.h"
 
// Same component headers
#include "GetDataWrapper.h"

// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{

	SpecialColumnInfo::SpecialColumnInfo(ConstSqlStmtHandlePtr pStmt, const SqlInfoProperties& props, IdentifierType identType)
		: m_identType(identType)
	{
		exASSERT(pStmt);
		exASSERT(pStmt->IsAllocated());

		SQLLEN cb = 0;
		SQLSMALLINT scopeVal;
		SQLSMALLINT pseudoColVal;
		GetDataWrapper::GetData(pStmt, 1, SQL_C_SSHORT, &scopeVal, sizeof(scopeVal), &cb, &m_isScopeNull);
		GetDataWrapper::GetData(pStmt, 2, props.GetMaxColumnNameLen(), m_columnName);
		GetDataWrapper::GetData(pStmt, 3, SQL_C_SSHORT, &m_sqlType, sizeof(m_sqlType), &cb, nullptr);
		GetDataWrapper::GetData(pStmt, 4, DB_MAX_TYPE_NAME_LEN, m_sqlTypeName);
		GetDataWrapper::GetData(pStmt, 5, SQL_C_SLONG, &m_columnSize, sizeof(m_columnSize), &cb, nullptr);
		GetDataWrapper::GetData(pStmt, 6, SQL_C_SLONG, &m_bufferLength, sizeof(m_bufferLength), &cb, nullptr);
		GetDataWrapper::GetData(pStmt, 7, SQL_C_SSHORT, &m_decimalDigits, sizeof(m_decimalDigits), &cb, nullptr);
		GetDataWrapper::GetData(pStmt, 8, SQL_C_SSHORT, &pseudoColVal, sizeof(pseudoColVal), &cb, nullptr);

		if (!m_isScopeNull)
		{
			switch (scopeVal)
			{
			case (SQLSMALLINT)SpecialColumnInfo::RowIdScope::CURSOR:
				m_scope = SpecialColumnInfo::RowIdScope::CURSOR;
				break;
			case (SQLSMALLINT)SpecialColumnInfo::RowIdScope::SESSION:
				m_scope = SpecialColumnInfo::RowIdScope::SESSION;
				break;
			case (SQLSMALLINT)SpecialColumnInfo::RowIdScope::TRANSCATION:
				m_scope = SpecialColumnInfo::RowIdScope::TRANSCATION;
				break;
			default:
				exASSERT_MSG(false, boost::str(boost::format(u8"Unknown Row id scope value %d") % scopeVal));
			}
		}
	}

}
