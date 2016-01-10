/*!
* \file Helpers.h
* \author Elias Gerber <eg@elisium.ch>
* \date 23.07.2014
* \brief Header file for generic Helpers, mostly about error-handling and assertions.
* \copyright GNU Lesser General Public License Version 3
*/ 

#pragma once

// Same component headers
#include "exOdbc.h"
#include "SqlHandle.h"

// Other headers
// System headers
#include <string>
#include <iostream>

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \brief	A wrapper to SQLGetInfo to read a String info.
	* 
	* \param	pHDbc					The Database connection handle.
	* \param	fInfoType				Type of the information.
	* \param [in,out]	sValue			String to receive the value read.
	* \details This will first call SQLGetInfo to determine the size of the buffer, then allocate a
	*			corresponding buffer and call GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER pInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue)
	* \see		SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER pInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue)
	* \throw	Exception
	*/
	extern EXODBCAPI void		GetInfo(ConstSqlDbcHandlePtr pHDbc, SQLUSMALLINT fInfoType, std::wstring& sValue);


	/*!
	 * \brief	A wrapper to SQLGetInfo.
	 *
	 * \param	pHDbc					The Database connection handle.
	 * \param	fInfoType				Type of the information.
	 * \param	pInfoValue			Output buffer pointer.
	 * \param	cbInfoValueMax			Length of buffer.
	 * \param [in,out]	pcbInfoValue	Out-pointer for total length in bytes (excluding null-terminate char for string-values).
	 * \see		http://msdn.microsoft.com/en-us/library/ms711681%28v=vs.85%29.aspx
	 * \throw	Exception
	 */
	extern EXODBCAPI void		GetInfo(ConstSqlDbcHandlePtr pHDbc, SQLUSMALLINT fInfoType, SQLPOINTER pInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue);


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
	extern EXODBCAPI void		GetData(ConstSqlStmtHandlePtr pHStmt, SQLUSMALLINT colOrParamNr, SQLSMALLINT targetType, SQLPOINTER pTargetValue, SQLLEN bufferLen, SQLLEN* strLenOrIndPtr, bool* pIsNull);


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
	extern EXODBCAPI void		GetData(ConstSqlStmtHandlePtr pHStmt, SQLUSMALLINT colNr, size_t maxNrOfChars, std::wstring& value, bool* pIsNull = NULL);


	/*!
	* \brief	A wrapper to SQLSetDescField
	*
	* \param	hDesc		 	The description handle to set an attribute for.
	* \param	recordNumber	Column number to set the attribute for.
	* \param	descriptionField Attribute to set.
	* \param	value			Value to set.
	* \throw	Exception
	*/
	extern EXODBCAPI void		SetDescriptionField(SQLHDESC hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value);


	/*!
	* \see SetDescriptionField(SQLHDESC hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value)
	*/
	extern EXODBCAPI void		SetDescriptionField(ConstSqlDescHandlePtr pHDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value);


	/*!
	* \see SetDescriptionField(SQLHDESC hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value)
	*/
	extern EXODBCAPI void		SetDescriptionField(const SqlDescHandle& hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value);


	/*!
	* \brief	A wrapper to SQLGetStmtAttr
	*
	* \param	pHStmt		 	The statement.
	* \param	type	Type of Description Handle to fetch from hStmt
	* \return	Row Descriptor Handle
	* \throw	Exception
	*/
	extern EXODBCAPI SqlDescHandlePtr	GetRowDescriptorHandle(ConstSqlStmtHandlePtr pHStmt, RowDescriptorType type);


	/*!
	* \brief	Return a SQL_TIME_STRUCT with the passed values set.
	*/
	extern EXODBCAPI SQL_TIME_STRUCT InitTime(SQLUSMALLINT hour, SQLUSMALLINT minute, SQLUSMALLINT second) noexcept;


	/*!
	* \brief	Return a SQL_DATE_STRUCT with the passed values set.
	*/
	extern EXODBCAPI SQL_DATE_STRUCT InitDate(SQLUSMALLINT day, SQLUSMALLINT month, SQLSMALLINT year) noexcept;


	/*!
	* \brief	Return a SQL_TIMESTAMP_STRUCT with the passed values set.
	*/
	extern EXODBCAPI SQL_TIMESTAMP_STRUCT InitTimestamp(SQLUSMALLINT hour, SQLUSMALLINT minute, SQLUSMALLINT second, SQLUINTEGER fraction, SQLUSMALLINT day, SQLUSMALLINT month, SQLSMALLINT year) noexcept;


	/*!
	* \brief	Return a SQL_NUMERIC_STRUCT with the passed values set.
	*/
	extern EXODBCAPI SQL_NUMERIC_STRUCT InitNumeric(SQLCHAR precision, SQLSCHAR scale, SQLCHAR sign, SQLCHAR val[SQL_MAX_NUMERIC_LEN]) noexcept;


	/*!
	* \brief	Return a SQL_NUMERIC_STRUCT where all fields are initialized to 0.
	*/
	extern EXODBCAPI SQL_NUMERIC_STRUCT InitNullNumeric() noexcept;


	/*!
	* \brief	Return true if all fields of the compared SQL_TIME_STRUCTs have the same value.
	*/
	extern EXODBCAPI bool IsTimeEqual(const SQL_TIME_STRUCT& t1, const SQL_TIME_STRUCT& t2) noexcept;


	/*!
	* \brief	Return true if all fields of the compared SQL_DATE_STRUCTs have the same value.
	*/
	extern EXODBCAPI bool IsDateEqual(const SQL_DATE_STRUCT& d1, const SQL_DATE_STRUCT& d2) noexcept;


	/*!
	* \brief	Return true if all fields of the compared SQL_TIMESTAMP_STRUCTs have the same value.
	*/
	extern EXODBCAPI bool IsTimestampEqual(const SQL_TIMESTAMP_STRUCT& ts1, const SQL_TIMESTAMP_STRUCT& ts2) noexcept;


	/*!
	* \brief Convert the value of a SQL_NUMERIC_STRUCT to the long value
	* \details Just copied from the ms sample.
	* \see https://support.microsoft.com/kb/222831/en-us
	*/
	extern EXODBCAPI long Str2Hex2Long(unsigned char hexValue[16]);


	extern EXODBCAPI void Long2StrHex(long value, char* hexValue);

	// Classes
	// -------
}

