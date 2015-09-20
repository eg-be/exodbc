/*!
* \file Visitors.h
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2014
* \brief Header file for the Visitors of the BufferPtrVariant.
* \copyright wxWindows Library Licence, Version 3.1
*
*/

#pragma once
#ifndef VISITORS_H
#define VISITORS_H

// Same component headers
#include "exOdbc.h"
#include "Exception.h"

// Other headers
#include "boost/variant.hpp"
#include "boost/lexical_cast.hpp"

// System headers

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \class BigintVisitor
	*
	* \brief Visitor to cast current value to a SQLBIGINT.
	*
	* This Visitor can cast the following sources to a SQLBIGINT:
	*
	* - SQLSMALLINT*
	* - SQLINTEGER*
	* - SQLBIGINT*
	*
	* \todo SQL_NUMERIC_STRUCT* if it fits into the bigint
	*/
	class EXODBCAPI BigintVisitor
		: public boost::static_visitor < SQLBIGINT >
	{
	public:
		SQLBIGINT operator()(SQLSMALLINT* smallInt) const { return *smallInt; };
		SQLBIGINT operator()(SQLINTEGER* i) const { return *i; };
		SQLBIGINT operator()(SQLBIGINT* bigInt) const { return *bigInt; };
		SQLBIGINT operator()(SQLCHAR* pChar) const { throw CastException(SQL_C_CHAR, SQL_C_SBIGINT); };
		SQLBIGINT operator()(SQLWCHAR* pWChar) const { throw CastException(SQL_C_WCHAR, SQL_C_SBIGINT); };
		SQLBIGINT operator()(SQLDOUBLE* pDouble) const { throw CastException(SQL_C_DOUBLE, SQL_C_SBIGINT); };
		SQLBIGINT operator()(SQLREAL* pReal) const { throw CastException(SQL_C_FLOAT, SQL_C_SBIGINT); };
		SQLBIGINT operator()(SQL_DATE_STRUCT* pTime) const { throw CastException(SQL_C_TYPE_DATE, SQL_C_SBIGINT); };
		SQLBIGINT operator()(SQL_TIME_STRUCT* pDate) const { throw CastException(SQL_C_TYPE_TIME, SQL_C_SBIGINT); };
		SQLBIGINT operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { throw CastException(SQL_C_TYPE_TIMESTAMP, SQL_C_SBIGINT); };
		SQLBIGINT operator()(SQL_NUMERIC_STRUCT* pNumeric) const { throw CastException(SQL_C_NUMERIC, SQL_C_SBIGINT); };
#if HAVE_MSODBCSQL_H
		SQLBIGINT operator()(SQL_SS_TIME2_STRUCT* pTime) const { throw CastException(SQL_C_SS_TIME2, SQL_C_SBIGINT); };
#endif
	};	// class BigintVisitor


	/*!
	* \class WStringVisitor
	*
	* \brief Visitor to cast current value to a std::wstring .
	*
	* This Visitor can cast the following sources to a std::wstring :
	*
	* - SQLSMALLINT* (Note: This will just format using %d)
	* - SQLINTEGER* (Note: This will just format using %ld)
	* - SQLBIGINT* (Note: This will just format using %lld)
	* - SQLCHAR*
	* - SQLDOUBLE* (Note: This will just format using boost::lexical_cast<std::wstring>)
	* - SQLDATE* Returns a date in the format YYYY-MM-DD
	* - SQLTIME_STRUCT* Returns a time in the format hh:mm:ss
	* - SQL_TIMESTAMP_STRUCT* Returns a timestamp in the format YYYY-MM-DDThh:mm:ss.fffffffff
	* \todo NUMERIC_STRUCT*
	*
	*/
	class EXODBCAPI WStringVisitor
		: public boost::static_visitor < std::wstring >
	{
	public:
		std::wstring operator()(SQLSMALLINT* smallInt) const { return (boost::wformat(L"%d") % *smallInt).str(); };
		std::wstring operator()(SQLINTEGER* i) const { return (boost::wformat(L"%ld") % *i).str(); };
		std::wstring operator()(SQLBIGINT* bigInt) const { return (boost::wformat(L"%lld") % *bigInt).str(); };
		std::wstring operator()(SQLCHAR* pChar) const{ throw CastException(SQL_C_CHAR, SQL_C_WCHAR); };
		std::wstring operator()(SQLWCHAR* pWChar) const { return pWChar; };
		std::wstring operator()(SQLDOUBLE* pDouble) const { return boost::lexical_cast<std::wstring>(*pDouble); };
		std::wstring operator()(SQLREAL* pReal) const { return boost::lexical_cast<std::wstring>(*pReal); };
		std::wstring operator()(SQL_DATE_STRUCT* pDate) const { return boost::str(boost::wformat(L"%04d-%02d-%02d") % pDate->year % pDate->month %pDate->day); };
		std::wstring operator()(SQL_TIME_STRUCT* pTime) const { return boost::str(boost::wformat(L"%02d:%02d:%02d") % pTime->hour % pTime->minute %pTime->second); };
		std::wstring operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { return boost::str(boost::wformat(L"%04d-%02d-%02dT%02d:%02d:%012.9f") % pTimestamp->year %pTimestamp->month %pTimestamp->day %pTimestamp->hour %pTimestamp->minute % ((SQLDOUBLE)pTimestamp->second + ((SQLDOUBLE)pTimestamp->fraction / (SQLDOUBLE)1000000000.0))); };
		std::wstring operator()(SQL_NUMERIC_STRUCT* pNumeric) const { throw CastException(SQL_C_NUMERIC, SQL_C_WCHAR); };
#if HAVE_MSODBCSQL_H
		std::wstring operator()(SQL_SS_TIME2_STRUCT* pTime2) const { return boost::str(boost::wformat(L"%02d:%02d:%012.9f") % pTime2->hour % pTime2->minute % ((SQLDOUBLE)pTime2->second + ((SQLDOUBLE)pTime2->fraction / (SQLDOUBLE)1000000000.0))); };
#endif
	};	// class WStringVisitor


	/*!
	* \class StringVisitor
	*
	* \brief Visitor to cast current value to a std::string .
	*
	* This Visitor can cast the following sources to a std::string :
	*
	* - SQLSMALLINT* (Note: This will just format using %d)
	* - SQLINTEGER* (Note: This will just format using %ld)
	* - SQLBIGINT* (Note: This will just format using %lld)
	* - SQLCHAR*
	* - SQLDOUBLE* (Note: This will just format using boost::lexical_cast<std::wstring>)
	* - SQLDATE* Returns a date in the format YYYY-MM-DD
	* - SQLTIME_STRUCT* Returns a time in the format hh:mm:ss
	* - SQL_TIMESTAMP_STRUCT* Returns a timestamp in the format YYYY-MM-DDThh:mm:ss.fffffffff
	* - SQL_SS_TIME2_STRUCT* Returns a time in the format hh:mm:ss.fffffffff Only available if HAVE_MSODBCSQL_His defined.
	* \todo NUMERIC_STRUCT*
	*/
	class EXODBCAPI StringVisitor
		: public boost::static_visitor < std::string >
	{
	public:
		std::string operator()(SQLSMALLINT* smallInt) const { return (boost::format("%d") % *smallInt).str(); };
		std::string operator()(SQLINTEGER* i) const { return (boost::format("%ld") % *i).str(); };
		std::string operator()(SQLBIGINT* bigInt) const { return (boost::format("%lld") % *bigInt).str(); };
		std::string operator()(SQLCHAR* pChar) const { return (char*)pChar; };
		std::string operator()(SQLWCHAR* pWChar) const { throw CastException(SQL_C_WCHAR, SQL_C_CHAR); };
		std::string operator()(SQLDOUBLE* pDouble) const { return boost::lexical_cast<std::string>(*pDouble); };
		std::string operator()(SQLREAL* pReal) const { return boost::lexical_cast<std::string>(*pReal); };
		std::string operator()(SQL_DATE_STRUCT* pDate) const { return boost::str(boost::format("%04d-%02d-%02d") % pDate->year % pDate->month %pDate->day); };
		std::string operator()(SQL_TIME_STRUCT* pTime) const { return boost::str(boost::format("%02d:%02d:%02d") % pTime->hour % pTime->minute %pTime->second); };
		std::string operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { return boost::str(boost::format("%04d-%02d-%02dT%02d:%02d:%012.9f") % pTimestamp->year %pTimestamp->month %pTimestamp->day %pTimestamp->hour %pTimestamp->minute % ((SQLDOUBLE)pTimestamp->second + ((SQLDOUBLE)pTimestamp->fraction / (SQLDOUBLE)1000000000.0))); };
		std::string operator()(SQL_NUMERIC_STRUCT* pNumeric) const { throw CastException(SQL_C_NUMERIC, SQL_C_CHAR); };
#if HAVE_MSODBCSQL_H
		std::string operator()(SQL_SS_TIME2_STRUCT* pTime2) const { return boost::str(boost::format("%02d:%02d:%012.9f") % pTime2->hour % pTime2->minute % ((SQLDOUBLE)pTime2->second + ((SQLDOUBLE)pTime2->fraction / (SQLDOUBLE)1000000000.0))); };
#endif
	};	// class StringVisitor


	/*!
	* \class DoubleVisitor
	*
	* \brief Visitor to cast current value to a SQLDOUBLE.
	*
	* This Visitor can cast the following sources to a SQLDOUBLE :
	*
	* - SQLSMALLINT*
	* - SQLINTEGER*
	* - SQLREAL*
	* - SQLDOUBLE*
	* \todo: SQL_NUMERIC_STRUCT*
	*/
	class EXODBCAPI DoubleVisitor
		: public boost::static_visitor < SQLDOUBLE >
	{
	public:
		SQLDOUBLE operator()(SQLSMALLINT* smallInt) const { return *smallInt; };
		SQLDOUBLE operator()(SQLINTEGER* i) const { return *i; };
		SQLDOUBLE operator()(SQLBIGINT* bigInt) const { throw CastException(SQL_C_SBIGINT, SQL_C_DOUBLE); };
		SQLDOUBLE operator()(SQLCHAR* pChar) const { throw CastException(SQL_C_CHAR, SQL_C_DOUBLE); };
		SQLDOUBLE operator()(SQLWCHAR* pWChar) const { throw CastException(SQL_C_WCHAR, SQL_C_DOUBLE); };
		SQLDOUBLE operator()(SQLDOUBLE* pDouble) const { return *pDouble; };
		SQLDOUBLE operator()(SQLREAL* pReal) const { return (SQLDOUBLE)(*pReal); };
		SQLDOUBLE operator()(SQL_DATE_STRUCT* pTime) const { throw CastException(SQL_C_TYPE_DATE, SQL_C_DOUBLE); };
		SQLDOUBLE operator()(SQL_TIME_STRUCT* pDate) const { throw CastException(SQL_C_TYPE_TIME, SQL_C_DOUBLE); };
		SQLDOUBLE operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { throw CastException(SQL_C_TYPE_TIMESTAMP, SQL_C_DOUBLE); };
		SQLDOUBLE operator()(SQL_NUMERIC_STRUCT* pNumeric) const { throw CastException(SQL_C_NUMERIC, SQL_C_DOUBLE); };
#if HAVE_MSODBCSQL_H
		SQLDOUBLE operator()(SQL_SS_TIME2_STRUCT* pTime) const { throw CastException(SQL_C_SS_TIME2, SQL_C_DOUBLE); };
#endif
	};	// class DoubleVisitor


	/*!
	* \class RealVisitor
	*
	* \brief Visitor to cast current value to a SQLREAL.
	*
	* This Visitor can cast the following sources to a SQLREAL :
	*
	* - SQLSMALLINT*
	* - SQLREAL*
	* \todo: SQL_NUMERIC_STRUCT*
	*/
	class EXODBCAPI RealVisitor
		: public boost::static_visitor < SQLDOUBLE >
	{
	public:
		SQLREAL operator()(SQLSMALLINT* smallInt) const { return *smallInt; };
		SQLREAL operator()(SQLINTEGER* i) const { throw CastException(SQL_C_SLONG, SQL_C_FLOAT); };
		SQLREAL operator()(SQLBIGINT* bigInt) const { throw CastException(SQL_C_SBIGINT, SQL_C_FLOAT); };
		SQLREAL operator()(SQLCHAR* pChar) const { throw CastException(SQL_C_CHAR, SQL_C_FLOAT); };
		SQLREAL operator()(SQLWCHAR* pWChar) const { throw CastException(SQL_C_WCHAR, SQL_C_FLOAT); };
		SQLREAL operator()(SQLDOUBLE* pDouble) const { throw CastException(SQL_C_DOUBLE, SQL_C_FLOAT); };
		SQLREAL operator()(SQLREAL* pReal) const { return *pReal; };
		SQLREAL operator()(SQL_DATE_STRUCT* pTime) const { throw CastException(SQL_C_TYPE_DATE, SQL_C_FLOAT); };
		SQLREAL operator()(SQL_TIME_STRUCT* pDate) const { throw CastException(SQL_C_TYPE_TIME, SQL_C_FLOAT); };
		SQLREAL operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { throw CastException(SQL_C_TYPE_TIMESTAMP, SQL_C_FLOAT); };
		SQLREAL operator()(SQL_NUMERIC_STRUCT* pNumeric) const { throw CastException(SQL_C_NUMERIC, SQL_C_FLOAT); };
#if HAVE_MSODBCSQL_H
		SQLREAL operator()(SQL_SS_TIME2_STRUCT* pTime) const { throw CastException(SQL_C_SS_TIME2, SQL_C_FLOAT); };
#endif
	};	// class RealVisitor


	/*!
	* \class TimestampVisitor
	*
	* \brief Visitor to cast current value to a SQL_TIMESTAMP_STRUCT.
	*
	* This Visitor can cast the following sources to a SQL_TIMESTAMP_STRUCT :
	*
	* - SQLDATE* (time is set to 00:00:00)
	* - SQLTIME* (date is set to 00.00.0000)
	* - SQLTIMESTAMP*
	* - SQL_SS_TIME2_STRUCT* (date is set to 00.00.0000). Only available if HAVE_MSODBCSQL_H is defined.
	*
	*/
	class EXODBCAPI TimestampVisitor
		: public boost::static_visitor < SQL_TIMESTAMP_STRUCT >
	{
	public:

		SQL_TIMESTAMP_STRUCT operator()(SQLSMALLINT* smallInt) const { throw CastException(SQL_C_SSHORT, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLINTEGER* i) const { throw CastException(SQL_C_SSHORT, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLBIGINT* bigInt) const { throw CastException(SQL_C_SBIGINT, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLCHAR* pChar) const { throw CastException(SQL_C_CHAR, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLWCHAR* pWChar) const { throw CastException(SQL_C_WCHAR, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLDOUBLE* pDouble) const { throw CastException(SQL_C_DOUBLE, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLREAL* pReal) const { throw CastException(SQL_C_FLOAT, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQL_DATE_STRUCT* pDate) const;
		SQL_TIMESTAMP_STRUCT operator()(SQL_TIME_STRUCT* pTime) const;
		SQL_TIMESTAMP_STRUCT operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const;
		SQL_TIMESTAMP_STRUCT operator()(SQL_NUMERIC_STRUCT* pNumeric) const { throw CastException(SQL_C_NUMERIC, SQL_C_TYPE_TIMESTAMP); };
#if HAVE_MSODBCSQL_H
		SQL_TIMESTAMP_STRUCT operator()(SQL_SS_TIME2_STRUCT* pTime) const;
#endif

	private:
	};	// class TimestampVisitor


	/*!
	* \class NumericVisitor
	*
	* \brief Visitor to cast current value to a SQL_NUMERIC_STRUCT.
	*
	* This Visitor can cast the following sources to a SQL_NUMERIC_STRUCT :
	*
	* - SQL_NUMERIC_STRUCT*
	* \todo SMALLINT, INTEGER, BIGINT
	*/
	class EXODBCAPI NumericVisitor
		: public boost::static_visitor < SQL_NUMERIC_STRUCT >
	{
	public:

		SQL_NUMERIC_STRUCT operator()(SQLSMALLINT* smallInt) const { throw CastException(SQL_C_SSHORT, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQLINTEGER* i) const { throw CastException(SQL_C_SSHORT, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQLBIGINT* bigInt) const { throw CastException(SQL_C_SBIGINT, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQLCHAR* pChar) const { throw CastException(SQL_C_CHAR, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQLWCHAR* pWChar) const { throw CastException(SQL_C_WCHAR, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQLDOUBLE* pDouble) const { throw CastException(SQL_C_DOUBLE, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQLREAL* pReal) const { throw CastException(SQL_C_FLOAT, SQL_C_NUMERIC); }
		SQL_NUMERIC_STRUCT operator()(SQL_DATE_STRUCT* pDate) const { throw CastException(SQL_TYPE_DATE, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQL_TIME_STRUCT* pTime) const { throw CastException(SQL_TYPE_TIME, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { throw CastException(SQL_TYPE_TIMESTAMP, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQL_NUMERIC_STRUCT* pNumeric) const { return *pNumeric; };
#if HAVE_MSODBCSQL_H
		SQL_NUMERIC_STRUCT operator()(SQL_SS_TIME2_STRUCT* pTime) const { throw CastException(SQL_C_SS_TIME2, SQL_C_NUMERIC); };
#endif

	private:
	};	// class NumericVisitor


	/*!
	* \class CharPtrVisitor
	*
	* \brief Visitor to access the buffer.
	*
	* This Visitor extracts the buffer-pointer and returns it as const SQLCHAR*,
	* which works for every datatype.
	*
	*/
	class EXODBCAPI CharPtrVisitor
		: public boost::static_visitor < const SQLCHAR* >
	{
	public:

		const SQLCHAR* operator()(SQLSMALLINT* smallInt) const { return (const SQLCHAR*)(smallInt); };
		const SQLCHAR* operator()(SQLINTEGER* i) const { return (const SQLCHAR*)(i); };
		const SQLCHAR* operator()(SQLBIGINT* bigInt) const { return (const SQLCHAR*)(bigInt); };
		const SQLCHAR* operator()(SQLCHAR* pChar) const { return (const SQLCHAR*)(pChar); };
		const SQLCHAR* operator()(SQLWCHAR* pWChar) const { return (const SQLCHAR*)(pWChar); };
		const SQLCHAR* operator()(SQLDOUBLE* pDouble) const { return (const SQLCHAR*)(pDouble); };
		const SQLCHAR* operator()(SQLREAL* pReal) const { return (const SQLCHAR*)(pReal); };
		const SQLCHAR* operator()(SQL_DATE_STRUCT* pDate) const { return (const SQLCHAR*)(pDate); };
		const SQLCHAR* operator()(SQL_TIME_STRUCT* pTime) const { return (const SQLCHAR*)(pTime); };
		const SQLCHAR* operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { return (const SQLCHAR*)(pTimestamp); };
		const SQLCHAR* operator()(SQL_NUMERIC_STRUCT* pNumeric) const { return (const SQLCHAR*)(pNumeric); };
#if HAVE_MSODBCSQL_H
		const SQLCHAR* operator()(SQL_SS_TIME2_STRUCT* pNumeric) const { return (const SQLCHAR*)(pNumeric); };
#endif
	};
}


#endif // VISITORS
