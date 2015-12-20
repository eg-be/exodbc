/*!
* \file PreparedStatement.h
* \author Elias Gerber <eg@elisium.ch>
* \date 20.12.2015
* \brief Header file for the PreparedStatement classe.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "SqlHandle.h"

// Other headers
// System headers
#include <string>

// Forward declarations
// --------------------

namespace exodbc
{
	// Consts
	// ------

	// Structs
	// -------

	// Classes
	// -------
	/*!
	* \class PreparedStatement
	*
	* \brief Takes an SQL Statement that can contain parameter markers ('?').
	*		 SqlCBuffer classes matching those markers can be bound to
	*		 execute the statement once or multiple times.
	*/
	class PreparedStatement
	{
	public:
		PreparedStatement() = delete;

		/*!
		* \brief	Constructs a Prepared statement using the passed pHDbc to
		*			allocate a new statement to be used.
		* \see		Allocate()
		*/
		PreparedStatement(ConstSqlDbcHandlePtr pHDbc, const std::wstring& sqlstmt);


		/*!
		* \brief	Constructs a Prepared statement using the passed pHStmt.
		* \see		Allocate()
		*/
		PreparedStatement(SqlStmtHandlePtr pHStmt, const std::wstring& stqlstmt);
		
	private:
		SqlStmtHandlePtr m_pHStmt;	///< The statement we operate on
	};

} // namespace exodbc

