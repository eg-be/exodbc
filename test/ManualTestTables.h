/*!
* \file ManualTestTables.h
* \author Elias Gerber <eg@elisium.ch>
* \date 26.12.2014
* \copyright GNU Lesser General Public License Version 3
*
* Declares classes derived from exodbc::Table that define their columns manually.
*/

#pragma once

// Same component headers

// Other headers
#include "exodbc/Database.h"
#include "exodbc/Table.h"

// System headers

// Forward declarations
// --------------------


// Structs
// -------

// Classes
// -------
namespace exodbctest
{
	// IntTypesTable
	// ----------
	class MIntTypesTable : public exodbc::Table
	{
	public:

		MIntTypesTable(exodbc::ConstDatabasePtr pDb, const std::string& name = u8"IntegerTypes");
		virtual ~MIntTypesTable() {};

		// Size of Type							Bytes		Min						Max
		SQLINTEGER		m_idIntegerTypes;	//	4

		SQLSMALLINT		m_smallInt;			//	2			-32768 					32767
		SQLINTEGER		m_int;				//	4			-2147483648 			2147483647		
		SQLBIGINT		m_bigInt;			//	8 			-9223372036854775808 	9223372036854775807
	};


	// FloatTypesTable
	// --------------
	class MFloatTypesTable : public exodbc::Table
	{
	public:
		MFloatTypesTable(exodbc::ConstDatabasePtr pDb, const std::string& name = u8"FloatTypes");
		virtual ~MFloatTypesTable() {};

		// Note: SQLFLOAT is typedefed to SQLDOUBLE (!!). Use SQLREAL for explicit FLOAT!

		SQLINTEGER				m_idFloatTypes;
		SQLDOUBLE				m_double;
		SQLREAL					m_doubleAsFloat;
		SQLREAL					m_float;
	};


	// CharTypesTable
	// --------------
	class MCharTypesTable : public exodbc::Table
	{
	public:

		MCharTypesTable(exodbc::ConstDatabasePtr pDb, const std::string& name = u8"CharTypes");
		virtual ~MCharTypesTable() {};

		SQLINTEGER	m_idCharTypes;
		SQLCHAR	m_varchar[128 + 1];
		SQLCHAR	m_char[128 + 1];
		SQLCHAR m_varchar_10[10 + 1];
		SQLCHAR m_char_10[10 + 1];
	};



	// WCharTypesTable
	// --------------
	class MWCharTypesTable : public exodbc::Table
	{
	public:

		MWCharTypesTable(exodbc::ConstDatabasePtr pDb, const std::string& name = u8"CharTypes");
		virtual ~MWCharTypesTable() {};

		SQLINTEGER	m_idCharTypes;
		SQLWCHAR	m_varchar[128 + 1];
		SQLWCHAR	m_char[128 + 1];
		SQLWCHAR	m_varchar_10[10 + 1];
		SQLWCHAR	m_char_10[10 + 1];
	};


	// DateTypesTable
	// --------------
	class MDateTypesTable : public exodbc::Table
	{
	public:
		MDateTypesTable(exodbc::ConstDatabasePtr pDb, const std::string& name = u8"DateTypes");
		virtual ~MDateTypesTable() {};

		SQLINTEGER				m_idDateTypes;
		SQL_DATE_STRUCT			m_date;
		SQL_TIME_STRUCT			m_time;
		SQL_TIMESTAMP_STRUCT	m_timestamp;
	};

	
	// BlobTypesTable
	// --------------
	class MBlobTypesTable : public exodbc::Table
	{
	public:
		MBlobTypesTable(exodbc::ConstDatabasePtr pDb, const std::string& name = u8"BlobTypes");
		virtual ~MBlobTypesTable() {};

		SQLINTEGER		m_idBlobTypes;
		SQLCHAR			m_blob[16];
		SQLCHAR			m_varblob_20[20];
	};


	// MNumbericTypesTable
	// ------------------
	class MNumericTypesTable : public exodbc::Table
	{
	public:
		MNumericTypesTable(exodbc::ConstDatabasePtr pDb, const std::string& name = u8"NumericTypes");
		virtual ~MNumericTypesTable() {};

		SQLINTEGER			m_idNumericTypes;

		SQL_NUMERIC_STRUCT	m_decimal_18_0;
		SQL_NUMERIC_STRUCT	m_decimal_18_10;
		SQL_NUMERIC_STRUCT	m_decimal_5_3;
	};


	// MNumbericTypesAsCharTable
	// ------------------
	class MNumericTypesAsCharTable : public exodbc::Table
	{
	public:
		MNumericTypesAsCharTable(exodbc::ConstDatabasePtr pDb, const std::string& name = u8"NumericTypes");
		virtual ~MNumericTypesAsCharTable() {};

		SQLINTEGER			m_idNumericTypes;

		SQLWCHAR			m_wcdecimal_18_0[20 + 1];
		SQLWCHAR			m_wcdecimal_18_10[20 + 1];
		SQLWCHAR			m_wdecimal_5_3[20 + 1];
	};


	// CharTable
	// ---------
	class MCharTable : public exodbc::Table
	{
	public:
		MCharTable(exodbc::ConstDatabasePtr pDb);
		virtual ~MCharTable() {};

		SQLINTEGER	m_idCharTable;
		SQLWCHAR	m_col2[128 + 1];
		SQLWCHAR	m_col3[128 + 1];
		SQLWCHAR	m_col4[128 + 2];
	};


	// IncompleCharTable
	// -----------------
	class MIncompleteCharTable : public exodbc::Table
	{
	public:
		MIncompleteCharTable(exodbc::ConstDatabasePtr pDb);
		virtual ~MIncompleteCharTable() {};

		SQLINTEGER	m_idCharTable;
		SQLWCHAR	m_col2[128 + 1];
		// we do not bind col3 and have a gap
		//SQLWCHAR	m_col3[128 + 1];
		SQLWCHAR	m_col4[128 + 2];
	};


	// NotExistingTable
	// ----------------
	class MNotExistingTable : public exodbc::Table
	{
	public:
		MNotExistingTable(exodbc::ConstDatabasePtr pDb);
		~MNotExistingTable() {};

		SQLINTEGER		m_idNotExisting;
	};
}