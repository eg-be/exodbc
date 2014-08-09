/*!
 * \file wxOdbc3.h
 * \author Elias Gerber <egerber@gmx.net>
 * \date 09.02.2014
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef WXODBC3_H
#define WXODBC3_H

// Defines to dll-import/export
// ----------------------------

#ifdef EXODBC_EXPORTS
	#define EXODBCAPI __declspec(dllexport)
#else
	#define EXODBCAPI __declspec(dllimport)
#endif

/* There are too many false positives for this one, particularly when using templates like wxVector<T> */
/* class 'foo' needs to have dll-interface to be used by clients of class 'bar'" */
#pragma warning(disable:4251)

#include <windows.h>

#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>
#include <odbcinst.h>

namespace exodbc
{


	// Global Consts
	// -------------
	extern EXODBCAPI const wchar_t* emptyString;
	extern EXODBCAPI const wchar_t* SQL_LOG_FILENAME;
	extern EXODBCAPI const wchar_t* SQL_CATALOG_FILENAME;

	const int wxDB_PATH_MAX                 = 254;

	// Database Globals
	const int DB_TYPE_NAME_LEN				= 40;
	const int DB_MAX_STATEMENT_LEN			= 4096;
	const int DB_MAX_WHERE_CLAUSE_LEN		= 2048;
	const int DB_MAX_ERROR_MSG_LEN			= 512;
	const int DB_MAX_ERROR_HISTORY			= 5;
	const int DB_MAX_TABLE_NAME_LEN			= 128;	// This value is sometimes also available from dbInf: dbInf.tableNameLen != 0
	const int DB_MAX_SCHEMA_NAME_LEN		= 128;	// This value is sometimes also available from dbInf: dbInf.schemaNameLen != 0
	const int DB_MAX_CATALOG_NAME_LEN		= 128;	// This value is sometimes also available from dbInf: dbInf.catalogNameLen != 0
	const int DB_MAX_COLUMN_NAME_LEN		= 128;
	const int DB_MAX_TABLE_TYPE_LEN			= 128;
	const int DB_MAX_TABLE_REMARKS_LEN		= 128;

	const int DB_DATA_TYPE_VARCHAR        = 1;
	const int DB_DATA_TYPE_INTEGER        = 2;
	const int DB_DATA_TYPE_FLOAT          = 3;
	const int DB_DATA_TYPE_DATE           = 4;
	const int DB_DATA_TYPE_BLOB           = 5;
	const int DB_DATA_TYPE_MEMO           = 6;

	const int DB_SELECT_KEYFIELDS         = 1;
	const int DB_SELECT_WHERE             = 2;
	const int DB_SELECT_MATCHING          = 3;
	const int DB_SELECT_STATEMENT         = 4;

	const int DB_UPD_KEYFIELDS            = 1;
	const int DB_UPD_WHERE                = 2;

	const int DB_DEL_KEYFIELDS            = 1;
	const int DB_DEL_WHERE                = 2;
	const int DB_DEL_MATCHING             = 3;

	const int DB_WHERE_KEYFIELDS          = 1;
	const int DB_WHERE_MATCHING           = 2;

	const int DB_GRANT_SELECT             = 1;
	const int DB_GRANT_INSERT             = 2;
	const int DB_GRANT_UPDATE             = 4;
	const int DB_GRANT_DELETE             = 8;
	const int DB_GRANT_ALL                = DB_GRANT_SELECT | DB_GRANT_INSERT | DB_GRANT_UPDATE | DB_GRANT_DELETE;

	// Structs
	// -------

	/*!
	* \brief Store error-information from odbc
	* 
	*/
	struct EXODBCAPI SErrorInfo
	{
		SErrorInfo()
		{
			SqlState[0] = 0;
			NativeError = 0;
		}

		SQLWCHAR		SqlState[5 + 1];
		SQLINTEGER		NativeError;
		std::wstring	Msg;
		friend std::wostream& operator<< (std::wostream &out, const SErrorInfo& ei);
		friend std::ostream& operator<<(std::ostream& os, const SErrorInfo& ei);
	};

	
	/*!
	 * \brief Very ugly conversion of small to wide - DO NOT USE, see Ticket #44
	 * 
	 * Transforms from wide to small by simple taking the char-values. Remove as soon as #44 is done.
	 * \param const std::wstring& w
	 * \return std::string
	*/
	std::string w2s(const std::wstring& w);

	std::vector<SErrorInfo> GetAllErrors(SQLHANDLE hEnv = NULL, SQLHANDLE hDbc = NULL, SQLHANDLE hStmt = NULL);

	// Enums
	// -----
	enum OdbcVersion
	{
		OV_UNKNOWN = 0,
		OV_2 = 2UL,
		OV_3 = 3UL,
		OV_3_8 = 380UL
	};

	// ODBC Error codes (derived from ODBC SqlState codes)
	enum wxODBC_ERRORS
	{
		DB_FAILURE                        = 0,
		DB_SUCCESS                        = 1,
		DB_ERR_NOT_IN_USE,
		DB_ERR_GENERAL_WARNING,                            // SqlState = '01000'
		DB_ERR_DISCONNECT_ERROR,                           // SqlState = '01002'
		DB_ERR_DATA_TRUNCATED,                             // SqlState = '01004'
		DB_ERR_PRIV_NOT_REVOKED,                           // SqlState = '01006'
		DB_ERR_INVALID_CONN_STR_ATTR,                      // SqlState = '01S00'
		DB_ERR_ERROR_IN_ROW,                               // SqlState = '01S01'
		DB_ERR_OPTION_VALUE_CHANGED,                       // SqlState = '01S02'
		DB_ERR_NO_ROWS_UPD_OR_DEL,                         // SqlState = '01S03'
		DB_ERR_MULTI_ROWS_UPD_OR_DEL,                      // SqlState = '01S04'
		DB_ERR_WRONG_NO_OF_PARAMS,                         // SqlState = '07001'
		DB_ERR_DATA_TYPE_ATTR_VIOL,                        // SqlState = '07006'
		DB_ERR_UNABLE_TO_CONNECT,                          // SqlState = '08001'
		DB_ERR_CONNECTION_IN_USE,                          // SqlState = '08002'
		DB_ERR_CONNECTION_NOT_OPEN,                        // SqlState = '08003'
		DB_ERR_REJECTED_CONNECTION,                        // SqlState = '08004'
		DB_ERR_CONN_FAIL_IN_TRANS,                         // SqlState = '08007'
		DB_ERR_COMM_LINK_FAILURE,                          // SqlState = '08S01'
		DB_ERR_INSERT_VALUE_LIST_MISMATCH,                 // SqlState = '21S01'
		DB_ERR_DERIVED_TABLE_MISMATCH,                     // SqlState = '21S02'
		DB_ERR_STRING_RIGHT_TRUNC,                         // SqlState = '22001'
		DB_ERR_NUMERIC_VALUE_OUT_OF_RNG,                   // SqlState = '22003'
		DB_ERR_ERROR_IN_ASSIGNMENT,                        // SqlState = '22005'
		DB_ERR_DATETIME_FLD_OVERFLOW,                      // SqlState = '22008'
		DB_ERR_DIVIDE_BY_ZERO,                             // SqlState = '22012'
		DB_ERR_STR_DATA_LENGTH_MISMATCH,                   // SqlState = '22026'
		DB_ERR_INTEGRITY_CONSTRAINT_VIOL,                  // SqlState = '23000'
		DB_ERR_INVALID_CURSOR_STATE,                       // SqlState = '24000'
		DB_ERR_INVALID_TRANS_STATE,                        // SqlState = '25000'
		DB_ERR_INVALID_AUTH_SPEC,                          // SqlState = '28000'
		DB_ERR_INVALID_CURSOR_NAME,                        // SqlState = '34000'
		DB_ERR_SYNTAX_ERROR_OR_ACCESS_VIOL,                // SqlState = '37000'
		DB_ERR_DUPLICATE_CURSOR_NAME,                      // SqlState = '3C000'
		DB_ERR_SERIALIZATION_FAILURE,                      // SqlState = '40001'
		DB_ERR_SYNTAX_ERROR_OR_ACCESS_VIOL2,               // SqlState = '42000'
		DB_ERR_OPERATION_ABORTED,                          // SqlState = '70100'
		DB_ERR_UNSUPPORTED_FUNCTION,                       // SqlState = 'IM001'
		DB_ERR_NO_DATA_SOURCE,                             // SqlState = 'IM002'
		DB_ERR_DRIVER_LOAD_ERROR,                          // SqlState = 'IM003'
		DB_ERR_SQLALLOCENV_FAILED,                         // SqlState = 'IM004'
		DB_ERR_SQLALLOCCONNECT_FAILED,                     // SqlState = 'IM005'
		DB_ERR_SQLSETCONNECTOPTION_FAILED,                 // SqlState = 'IM006'
		DB_ERR_NO_DATA_SOURCE_DLG_PROHIB,                  // SqlState = 'IM007'
		DB_ERR_DIALOG_FAILED,                              // SqlState = 'IM008'
		DB_ERR_UNABLE_TO_LOAD_TRANSLATION_DLL,             // SqlState = 'IM009'
		DB_ERR_DATA_SOURCE_NAME_TOO_LONG,                  // SqlState = 'IM010'
		DB_ERR_DRIVER_NAME_TOO_LONG,                       // SqlState = 'IM011'
		DB_ERR_DRIVER_KEYWORD_SYNTAX_ERROR,                // SqlState = 'IM012'
		DB_ERR_TRACE_FILE_ERROR,                           // SqlState = 'IM013'
		DB_ERR_TABLE_OR_VIEW_ALREADY_EXISTS,               // SqlState = 'S0001'
		DB_ERR_TABLE_NOT_FOUND,                            // SqlState = 'S0002'
		DB_ERR_INDEX_ALREADY_EXISTS,                       // SqlState = 'S0011'
		DB_ERR_INDEX_NOT_FOUND,                            // SqlState = 'S0012'
		DB_ERR_COLUMN_ALREADY_EXISTS,                      // SqlState = 'S0021'
		DB_ERR_COLUMN_NOT_FOUND,                           // SqlState = 'S0022'
		DB_ERR_NO_DEFAULT_FOR_COLUMN,                      // SqlState = 'S0023'
		DB_ERR_GENERAL_ERROR,                              // SqlState = 'S1000'
		DB_ERR_MEMORY_ALLOCATION_FAILURE,                  // SqlState = 'S1001'
		DB_ERR_INVALID_COLUMN_NUMBER,                      // SqlState = 'S1002'
		DB_ERR_PROGRAM_TYPE_OUT_OF_RANGE,                  // SqlState = 'S1003'
		DB_ERR_SQL_DATA_TYPE_OUT_OF_RANGE,                 // SqlState = 'S1004'
		DB_ERR_OPERATION_CANCELLED,                        // SqlState = 'S1008'
		DB_ERR_INVALID_ARGUMENT_VALUE,                     // SqlState = 'S1009'
		DB_ERR_FUNCTION_SEQUENCE_ERROR,                    // SqlState = 'S1010'
		DB_ERR_OPERATION_INVALID_AT_THIS_TIME,             // SqlState = 'S1011'
		DB_ERR_INVALID_TRANS_OPERATION_CODE,               // SqlState = 'S1012'
		DB_ERR_NO_CURSOR_NAME_AVAIL,                       // SqlState = 'S1015'
		DB_ERR_INVALID_STR_OR_BUF_LEN,                     // SqlState = 'S1090'
		DB_ERR_DESCRIPTOR_TYPE_OUT_OF_RANGE,               // SqlState = 'S1091'
		DB_ERR_OPTION_TYPE_OUT_OF_RANGE,                   // SqlState = 'S1092'
		DB_ERR_INVALID_PARAM_NO,                           // SqlState = 'S1093'
		DB_ERR_INVALID_SCALE_VALUE,                        // SqlState = 'S1094'
		DB_ERR_FUNCTION_TYPE_OUT_OF_RANGE,                 // SqlState = 'S1095'
		DB_ERR_INF_TYPE_OUT_OF_RANGE,                      // SqlState = 'S1096'
		DB_ERR_COLUMN_TYPE_OUT_OF_RANGE,                   // SqlState = 'S1097'
		DB_ERR_SCOPE_TYPE_OUT_OF_RANGE,                    // SqlState = 'S1098'
		DB_ERR_NULLABLE_TYPE_OUT_OF_RANGE,                 // SqlState = 'S1099'
		DB_ERR_UNIQUENESS_OPTION_TYPE_OUT_OF_RANGE,        // SqlState = 'S1100'
		DB_ERR_ACCURACY_OPTION_TYPE_OUT_OF_RANGE,          // SqlState = 'S1101'
		DB_ERR_DIRECTION_OPTION_OUT_OF_RANGE,              // SqlState = 'S1103'
		DB_ERR_INVALID_PRECISION_VALUE,                    // SqlState = 'S1104'
		DB_ERR_INVALID_PARAM_TYPE,                         // SqlState = 'S1105'
		DB_ERR_FETCH_TYPE_OUT_OF_RANGE,                    // SqlState = 'S1106'
		DB_ERR_ROW_VALUE_OUT_OF_RANGE,                     // SqlState = 'S1107'
		DB_ERR_CONCURRENCY_OPTION_OUT_OF_RANGE,            // SqlState = 'S1108'
		DB_ERR_INVALID_CURSOR_POSITION,                    // SqlState = 'S1109'
		DB_ERR_INVALID_DRIVER_COMPLETION,                  // SqlState = 'S1110'
		DB_ERR_INVALID_BOOKMARK_VALUE,                     // SqlState = 'S1111'
		DB_ERR_DRIVER_NOT_CAPABLE,                         // SqlState = 'S1C00'
		DB_ERR_TIMEOUT_EXPIRED                             // SqlState = 'S1T00'
	};

	// SQL-Log-State
	enum wxDbSqlLogState
	{
		sqlLogOFF,
		sqlLogON
	};

	// Known Databases
	// ---------------
	// These are the databases currently tested and working with these classes
	// See the comments in wxDb::Dbms() for exceptions/issues with
	// each of these database engines
	enum wxDBMS
	{
		dbmsUNIDENTIFIED,
		dbmsORACLE,
		dbmsSYBASE_ASA,        // Adaptive Server Anywhere
		dbmsSYBASE_ASE,        // Adaptive Server Enterprise
		dbmsMS_SQL_SERVER,
		dbmsMY_SQL,
		dbmsPOSTGRES,
		dbmsACCESS,
		dbmsDBASE,
		dbmsINFORMIX,
		dbmsVIRTUOSO,
		dbmsDB2,
		dbmsINTERBASE,
		dbmsPERVASIVE_SQL,
		dbmsXBASE_SEQUITER,
		dbmsFIREBIRD,
		dbmsMAXDB,
		dbmsFuture1,
		dbmsFuture2,
		dbmsFuture3,
		dbmsFuture4,
		dbmsFuture5,
		dbmsFuture6,
		dbmsFuture7,
		dbmsFuture8,
		dbmsFuture9,
		dbmsFuture10
	};


}

#endif // WXODBC3_H
