/*!
* \file GetDataWrapper.h
* \author Elias Gerber <eg@elisium.ch>
* \date 04.05.2017
* \brief Header file for GetDataWrapper.
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
	/*!
	* \class GetDataWrapper
	* \brief Provides convenience wrappers for SQLGetData
	*/
	class EXODBCAPI GetDataWrapper
	{
	public:
		/*!
		 * \brief	Gets one field of a record of the passed stmt-handle.
		 *
		 * \param	pHStmt				  	The statement-handle.
		 * \param	colOrParamNr		  	The col or parameter nr. (1-indexed)
		 * \param	targetType			  	Type of the target.
		 * \param	pTargetValue			Pointer to the target buffer.
		 * \param	bufferLen			  	Length of the buffer in Bytes (not chars!).
		 * \param [in,out]	strLenOrIndPtr	Pointer to return the number of bytes read.
		 * \param [in,out]	pIsNull		  	If pIsNull is not NULL, set to TRUE if the field is NULL. Ignored if pIsNull is NULL.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		static void		GetData(ConstSqlStmtHandlePtr pHStmt, SQLUSMALLINT colOrParamNr, SQLSMALLINT targetType, SQLPOINTER pTargetValue, SQLLEN bufferLen, SQLLEN* strLenOrIndPtr, bool* pIsNull);


		/*!
		 * \brief	Gets string data. Allocates a wchar_t buffer with maxNrOfChars wchars + one char for the null-terminate:
		 * 			SQLWCHAR* buff = new buff[maxNrOfChars + 1];
		 * 			Then calls GetData with that buffer and takes into account that GetData needs a buffer-size, not a char-size.
		 * 			If the data is null or reading fails, value is set to an empty string.
		 *
		 * \param	pHStmt		 	The statement.
		 * \param	colNr		 	The col nr. (1-indexed)
		 * \param	maxNrOfChars 	The maximum nr of characters.
		 * \param [in,out]	value	The value.
		 * \param	pIsNull			If it is a non-null pointer, its value will be set to True if the string to retrieve is NULL.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		static void		GetData(ConstSqlStmtHandlePtr pHStmt, SQLUSMALLINT colNr, size_t maxNrOfChars, std::string& value, bool* pIsNull = NULL);

	private:
	};
}

