/*!
* \file Helpers.h
* \author Elias Gerber <eg@elisium.ch>
* \date 23.07.2014
* \brief Header file for generic Helpers.
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
	* \brief	Return a SQL_TIMESTAMP_STRUCT, set to now in UTC. Fraction is set to 0.
	*/
	extern EXODBCAPI SQL_TIMESTAMP_STRUCT InitUtcTimestamp();


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


	/*
	* \brief	Format the value of a SQL_TIMESTAMP_STRUCT as 'YYYY-MM-DD hh:mm:ss' and return as string.
	*			If includeFraction is true, the fractional part of ts is added, as (ts.fraction / 1'000'000'000).
	*/
	extern EXODBCAPI std::string TimestampToSqlString(const SQL_TIMESTAMP_STRUCT& ts, bool includeFraction = false) noexcept;

	// Classes
	// -------
}

