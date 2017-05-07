/*!
* \file Sql2StringHelper.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.07.2014
* \brief Source file for the stuff from exOdbc.h
* \copyright GNU Lesser General Public License Version 3
*/ 

// Own header
#include "Sql2StringHelper.h"

// Same component headers
// Other headers
// Debug
#include "DebugNew.h"

namespace exodbc {

	// Static consts
	// -------------

	// Implementation
	// --------------
	std::string Sql2StringHelper::SqlTrueFalse2s(SQLSMALLINT b) noexcept
	{
		switch (b)
		{
		case SQL_TRUE:
			return u8"TRUE";
		case SQL_FALSE:
			return u8"FALSE";
		default:
			return u8"?????";
		}
	}


	std::string Sql2StringHelper::SqlReturn2s(SQLRETURN ret) noexcept
	{
		switch (ret)
		{
		case SQL_SUCCESS:
			return u8"SUCCESS";
		case SQL_SUCCESS_WITH_INFO:
			return u8"SUCCESS_WITH_INFO";
		case SQL_NO_DATA:
			return u8"SQL_NO_DATA";
		case SQL_ERROR:
			return u8"SQL_ERROR";
		case SQL_NEED_DATA:
			return u8"SQL_NEED_DATA";
		case SQL_INVALID_HANDLE:
			return u8"SQL_INVALID_HANDLE";
		default:
			return u8"???";
		}
	}


	std::string Sql2StringHelper::SqlType2s(SQLSMALLINT sqlType) noexcept
	{
		switch (sqlType)
		{
		case SQL_CHAR:
			return u8"CHAR";
		case SQL_VARCHAR:
			return u8"VARCHAR";
		case SQL_LONGVARCHAR:
			return u8"LONGVARCHAR";
		case SQL_WCHAR:
			return u8"WCHAR";
		case SQL_WVARCHAR:
			return u8"WVARCHAR";
		case SQL_WLONGVARCHAR:
			return u8"WLONGVARCHAR";
		case SQL_DECIMAL:
			return u8"DECIMAL";
		case SQL_NUMERIC:
			return u8"NUMERIC";
		case SQL_SMALLINT:
			return u8"SMALLINT";
		case SQL_INTEGER:
			return u8"INTEGER";
		case SQL_REAL:
			return u8"REAu8";
		case SQL_FLOAT:
			return u8"FLOAT";
		case SQL_DOUBLE:
			return u8"DOUBLE";
		case SQL_BIT:
			return u8"BIT";
		case SQL_TINYINT:
			return u8"TINYINT";
		case SQL_BIGINT:
			return u8"BIGINT";
		case SQL_BINARY:
			return u8"BINARY";
		case SQL_VARBINARY:
			return u8"VARBINARY";
		case SQL_LONGVARBINARY:
			return u8"LONGVARBINARY";
		case SQL_TYPE_DATE:	//[6]
			return u8"TYPE_DATE";
		case SQL_TYPE_TIME: //[6]
			return u8"TYPE_TIME";
		case SQL_TYPE_TIMESTAMP: //[6]
			return u8"TYPE_TIMESTAMP";
		case SQL_DATE:
			return u8"DATE";
		case SQL_TIME:
			return u8"TIME";
		case SQL_TIMESTAMP:
			return u8"TIMESTAMP";
			//		case SQL_TYPE_UTCDATETIME:
			//			return u8"TYPE_UTCDATETIME";
			//		case SQL_TYPE_UTCTIME:
			//			return u8"TYPE_UTCTIME";
		case SQL_INTERVAL_MONTH: //[7]
			return u8"INTERVAL_MONTH";
		case SQL_INTERVAL_YEAR: //[7]
			return u8"INTERVAL_YEAR";
		case SQL_INTERVAL_YEAR_TO_MONTH: //[7]
			return u8"INTERVAL_YEAR_TO_MONTH";
		case SQL_INTERVAL_DAY: //[7]
			return u8"INTERVAL_DAY";
		case SQL_INTERVAL_HOUR: ///[7]
			return u8"INTERVAL_HOUR";
		case SQL_INTERVAL_MINUTE: //[7]
			return u8"INTERVAL_MINUTE";
		case SQL_INTERVAL_SECOND: //[7]
			return u8"INTERVAL_SECOND";
		case SQL_INTERVAL_DAY_TO_HOUR: //[7]
			return u8"INTERVAL_DAY_TO_HOUR";
		case SQL_INTERVAL_DAY_TO_MINUTE: //[7]
			return u8"INTERVAL_DAY_TO_MINUTE";
		case SQL_INTERVAL_DAY_TO_SECOND: //[7]
			return u8"INTERVAL_DAY_TO_SECOND";
		case SQL_INTERVAL_HOUR_TO_MINUTE: //[7]
			return u8"INTERVAL_HOUR_TO_MINUTE";
		case SQL_INTERVAL_HOUR_TO_SECOND: //[7]
			return u8"INTERVAL_HOUR_TO_SECOND";
		case SQL_INTERVAL_MINUTE_TO_SECOND: //[7]
			return u8"INTERVAL_MINUTE_TO_SECOND";
		case SQL_GUID:
			return u8"GUID";
		default:
			return u8"???";
		}
	}


	std::string Sql2StringHelper::SqLCType2s(SQLSMALLINT sqlCType) noexcept
	{
		switch (sqlCType)
		{
		case SQL_C_STINYINT:
			return u8"SQL_C_STINYINT";
		case SQL_C_UTINYINT:
			return u8"SQL_C_UTINYINT";
		case SQL_C_SSHORT:
			return u8"SQL_C_SSHORT";
		case SQL_C_USHORT:
			return u8"SQL_C_USHORT";
		case SQL_C_SLONG:
			return u8"SQL_C_SLONG";
		case SQL_C_ULONG:
			// Note: This is also the type for SQL_C_BOOKMARK
			return u8"SQL_C_ULONG";
		case SQL_C_SBIGINT:
			return u8"SQL_C_SBIGINT";
		case SQL_C_UBIGINT:
			return u8"SQL_C_UBIGINT";
		case SQL_C_CHAR:
			return u8"SQL_C_CHAR";
		case SQL_C_WCHAR:
			return u8"SQL_C_WCHAR";
		case SQL_C_FLOAT:
			return u8"SQL_C_FLOAT";
		case SQL_C_DOUBLE:
			return u8"SQL_C_DOUBLE";
		case SQL_C_BIT:
			return u8"SQL_C_BIT";
		case SQL_C_BINARY:
			// Note: This is also the type for SQL_C_VARBOOKMARK:
			return u8"SQL_C_BINARY";
			//case SQL_C_BOOKMARK:
			//	return u8"SQL_C_BOOKMARK";
			//case SQL_C_VARBOOKMARK:
			//	return u8"SQL_C_VARBOOKMARK";
		case SQL_C_TYPE_DATE:
			return u8"SQL_C_TYPE_DATE";
		case SQL_C_TYPE_TIME:
			return u8"SQL_C_TYPE_TIME";
		case SQL_C_TYPE_TIMESTAMP:
			return u8"SQL_C_TYPE_TIMESTAMP";
		case SQL_C_NUMERIC:
			return u8"SQL_C_NUMERIC";
		case SQL_C_GUID:
			return u8"SQL_C_GUID";

			// Old odbc 2.x values
		case SQL_C_DATE:
			return u8"SQL_C_DATE";
		case SQL_C_TIME:
			return u8"SQL_C_TIME";
		case SQL_C_TIMESTAMP:
			return u8"SQL_C_TIMESTAMP";

		default:
			return u8"???";
		}
	}


	std::string Sql2StringHelper::SqlNullable2s(SQLSMALLINT nullable) noexcept
	{
		switch (nullable)
		{
		case SQL_NO_NULLS:
			return u8"No Nulls";
		case SQL_NULLABLE:
			return u8"Nullable";
		case SQL_NULLABLE_UNKNOWN:
			return u8"Unknown";
			
		default:
			return u8"???";
		}
	}

	std::string Sql2StringHelper::SqlCType2Odbcs(SQLSMALLINT sqlCType) noexcept
	{
		switch (sqlCType)
		{
		case SQL_C_STINYINT:
			return u8"SQLSCHAR";
		case SQL_C_UTINYINT:
			return u8"SQLCHAR";
		case SQL_C_SSHORT:
			return u8"SQLSMALLINT";
		case SQL_C_USHORT:
			return u8"SQLUSMALLINT";
		case SQL_C_SLONG:
			return u8"SQLINTEGER";
		case SQL_C_ULONG:
			// Note: This is also the type for SQL_C_BOOKMARK
			return u8"SQLUINTEGER";
		case SQL_C_SBIGINT:
			return u8"SQLBIGNT";
		case SQL_C_UBIGINT:
			return u8"SQLUBIGINT";
		case SQL_C_CHAR:
			return u8"SQLCHAR*";
		case SQL_C_WCHAR:
			return u8"SQLWCHAR*";
		case SQL_C_FLOAT:
			return u8"SQLREAu8";
		case SQL_C_DOUBLE:
			return u8"SQLDOUBLE";
		case SQL_C_BIT:
			return u8"SQLCHAR";
		case SQL_C_BINARY:
			// Note: This is also the type for SQL_C_VARBOOKMARK:
			return u8"SQLCHAR*";
			//case SQL_C_BOOKMARK:
			//	return u8"SQL_C_BOOKMARK";
			//case SQL_C_VARBOOKMARK:
			//	return u8"SQL_C_VARBOOKMARK";
		case SQL_C_TYPE_DATE:
			return u8"SQL_DATE_STRUCT";
		case SQL_C_TYPE_TIME:
			return u8"SQL_TIME_STRUCT";
		case SQL_C_TYPE_TIMESTAMP:
			return u8"SQL_TIMESTAMP_STRUCT";
		case SQL_C_NUMERIC:
			return u8"SQL_NUMERIC_STRUCT";
		case SQL_C_GUID:
			return u8"SQL_GUID";

			// Old odbc 2.x values
		case SQL_C_DATE:
			return u8"SQL_DATE_STRUCT";
		case SQL_C_TIME:
			return u8"SQL_TIME_STRUCT";
		case SQL_C_TIMESTAMP:
			return u8"SQL_TIMESTAMP_STRUCT";

		default:
			return u8"???";
		}
	}


	std::string Sql2StringHelper::DatabaseProcudt2s(DatabaseProduct dbms) noexcept
	{
		switch (dbms)
		{
		case DatabaseProduct::DB2:
			return u8"DB2";
		case DatabaseProduct::MS_SQL_SERVER:
			return u8"SqlServer";
		case DatabaseProduct::MY_SQL:
			return u8"MySql";
		case DatabaseProduct::EXCEL:
			return u8"Excel";
		case DatabaseProduct::ACCESS:
			return u8"Access";
		default:
			return u8"UnknownDbms";
		}
	}


	std::string Sql2StringHelper::HandleType2s(SQLSMALLINT type) noexcept
	{
		switch (type)
		{
		case SQL_HANDLE_ENV:
			return u8"ENV";
		case SQL_HANDLE_DBC:
			return u8"DBC";
		case SQL_HANDLE_STMT:
			return u8"STMT";
		case SQL_HANDLE_DESC:
			return u8"DESC";
		default:
			return u8"???";
		}
	}
}
