/*!
 * \file GenericTestTables.h
 * \author Elias Gerber <eg@zame.ch>
 * \date 23.02.2014
 * 
 * [Brief Header-file description]
 */ 

#pragma once

// Same component headers
// Other headers
#include "Table.h"

// System headers

// Forward declarations
// --------------------
namespace exodbc
{
	class Database;
}

// Structs
// -------

// Classes
// -------
namespace exodbc
{
	namespace TestTables
	{
		enum NameCase
		{
			NC_UPPER,	///< Tables will be created using all UPPERCASE letters for table- and column-names
			NC_LOWER	///< Tables will be created using all lowercase letters for table- and column-names
		};

		std::wstring GetTableName(const std::wstring& tableName, NameCase nameCase);
		std::wstring GetColName(const std::wstring& columnName, NameCase nameCase);
	}

	// NotExistingTable
	// ----------------
	class NotExistingTable : public exodbc::Table
	{
	public:
		NotExistingTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		~NotExistingTable() {};

		int		m_idNotExisting;
	};

	// CharTypesTable
	// --------------
	class CharTypesTable : public exodbc::Table
	{
	public:

		CharTypesTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~CharTypesTable() {};

		SQLINTEGER	m_idCharTypes;
		SQLWCHAR	m_varchar[128 + 1];
		SQLWCHAR	m_char[128 + 1];
	};

	// CharTypesTmpTable
	// --------------
	class CharTypesTmpTable : public exodbc::Table
	{
	public:

		CharTypesTmpTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~CharTypesTmpTable() {};

		SQLINTEGER	m_idCharTypes;
		SQLWCHAR	m_varchar[128 + 1];
		SQLWCHAR	m_char[128 + 1];
	};

	// IntTypesTable
	// ----------
	class IntTypesTable : public exodbc::Table
	{
	public:

		IntTypesTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~IntTypesTable() {};

		// Size of Type							Bytes		Min						Max
		SQLINTEGER		m_idIntegerTypes;	//	4
		
		SQLSMALLINT		m_smallInt;			//	2			-32768 					32767
		SQLINTEGER		m_int;				//	4			-2147483648 			2147483647		
		SQLBIGINT		m_bigInt;			//	8 			-9223372036854775808 	9223372036854775807
	};

	// IntTypesTmpTable
	// ----------
	class IntTypesTmpTable : public exodbc::Table
	{
	public:

		IntTypesTmpTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~IntTypesTmpTable() {};

		// Size of Type							Bytes		Min						Max
		SQLINTEGER		m_idIntegerTypes;	//	4
		
		SQLSMALLINT		m_smallInt;			//	2			-32768 					32767
		SQLINTEGER		m_int;				//	4			-2147483648 			2147483647
		SQLBIGINT		m_bigInt;			//	8 			-9223372036854775808 	9223372036854775807
	};

	// DateTypesTable
	// --------------
	class DateTypesTable : public exodbc::Table
	{
	public:
		DateTypesTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~DateTypesTable() {};

		SQLINTEGER				m_idDateTypes;
		SQL_DATE_STRUCT			m_date;
		SQL_TIME_STRUCT			m_time;
		SQL_TIMESTAMP_STRUCT	m_timestamp;
	};

	// DateTypesTmpTable
	// --------------
	class DateTypesTmpTable : public exodbc::Table
	{
	public:
		DateTypesTmpTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~DateTypesTmpTable() {};

		SQLINTEGER				m_idDateTypes;
		SQL_DATE_STRUCT			m_date;
		SQL_TIME_STRUCT			m_time;
		SQL_TIMESTAMP_STRUCT	m_timestamp;
	};

	// FloatTypesTable
	// --------------
	class FloatTypesTable : public exodbc::Table
	{
	public:
		FloatTypesTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~FloatTypesTable() {};

		SQLINTEGER				m_idFloatTypes;
		SQLDOUBLE				m_double;
		SQLFLOAT				m_float;
	};

	// FloatTypesTmpTable
	// ------------------
	class FloatTypesTmpTable : public exodbc::Table
	{
	public:
		FloatTypesTmpTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~FloatTypesTmpTable() {};

		SQLINTEGER				m_idFloatTypes;
		SQLDOUBLE				m_double;
		SQLFLOAT				m_float;
	};

	// NumbericTypesTable
	// ------------------
	class NumericTypesTable : public exodbc::Table
	{
	public:
		enum ReadMode { ReasAsNumeric, ReadAsChar };
		NumericTypesTable(exodbc::Database* pDb, ReadMode readMode, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~NumericTypesTable() {};

		SQLINTEGER			m_idNumericTypes;
		SQL_NUMERIC_STRUCT	m_decimal_18_0;
		SQL_NUMERIC_STRUCT	m_decimal_18_10;

		SQLWCHAR			m_wcdecimal_18_0[20 + 1];
		SQLWCHAR			m_wcdecimal_18_10[20 + 1];

		const ReadMode m_readMode;

	private:
	};

	// NumbericTypesTable
	// ------------------
	class NumericTypesTmpTable : public exodbc::Table
	{
	public:
		// TODO: This name is not correct. ? The test write as "numer".. mmh
		enum ReadMode { ReasAsNumeric, ReadAsChar };
		NumericTypesTmpTable(exodbc::Database* pDb, ReadMode readMode, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~NumericTypesTmpTable() {};

		SQLINTEGER			m_idNumericTypes;
		SQL_NUMERIC_STRUCT	m_decimal_18_0;
		SQL_NUMERIC_STRUCT	m_decimal_18_10;

		SQLWCHAR			m_wcdecimal_18_0[20 + 1];
		SQLWCHAR			m_wcdecimal_18_10[20 + 1];

		const ReadMode m_readMode;

	private:
	};

	// BlobTypesTable
	// --------------
	class BlobTypesTable : public exodbc::Table
	{
	public:
		BlobTypesTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~BlobTypesTable() {};

		SQLINTEGER		m_idBlobTypes;
		SQLCHAR			m_blob[16];
	};

	// BlobTypesTmpTable
	// --------------
	class BlobTypesTmpTable : public exodbc::Table
	{
	public:
		BlobTypesTmpTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~BlobTypesTmpTable() {};

		SQLINTEGER		m_idBlobTypes;
		SQLCHAR			m_blob[16];
	};

	// CharTable
	// ---------
	class CharTable : public exodbc::Table
	{
	public:
		CharTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~CharTable() {};

		SQLINTEGER	m_idCharTable;
		SQLWCHAR	m_col2[128 + 1];
		SQLWCHAR	m_col3[128 + 1];
		SQLWCHAR	m_col4[128 + 2];
	};

	// IncompleCharTable
	class IncompleteCharTable : public exodbc::Table
	{
	public:
		IncompleteCharTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~IncompleteCharTable() {};

		SQLINTEGER	m_idCharTable;
		SQLWCHAR	m_col2[128 + 1];
		// we do not bind col3 and have a gap
		//SQLWCHAR	m_col3[128 + 1];
		SQLWCHAR	m_col4[128 + 2];
	};
} // namespace exodbc