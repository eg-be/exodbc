/*!
* \file PreparedStatement.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 20.12.2015
* \brief Source file for SqlCBuffer.
* \copyright GNU Lesser General Public License Version 3
*
*/

#include "stdafx.h"

// Own header
#include "PreparedStatement.h"

// Same component headers
// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	PreparedStatement::PreparedStatement(ConstSqlDbcHandlePtr pHDbc, const std::wstring& sqlstmt)
	{
		m_pHStmt = std::make_shared<SqlStmtHandle>(pHDbc);
	}


	PreparedStatement::PreparedStatement(SqlStmtHandlePtr pHstmt, const std::wstring& sqlstmt)
		: m_pHStmt(pHstmt)
	{

	}
}
