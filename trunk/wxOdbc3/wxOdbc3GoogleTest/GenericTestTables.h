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
#include "dbtable.h"

// System headers

// Forward declarations
// --------------------
namespace exodbc
{
	class wxDb;
}

// Structs
// -------

// Classes
// -------
namespace wxOdbc3Test
{

	// NotExistingTable
	// ----------------
	class NotExistingTable : public exodbc::wxDbTable
	{
	public:
		NotExistingTable(exodbc::wxDb* pDb);
		~NotExistingTable() {};

		int		m_idNotExisting;
	};

	// CharTypesTable
	// --------------
	class CharTypesTable : public exodbc::wxDbTable
	{
	public:

		CharTypesTable(exodbc::wxDb* pDb);
		virtual ~CharTypesTable() {};

		SQLINTEGER	m_idCharTypes;
		SQLWCHAR	m_varchar[128 + 1];
		SQLWCHAR	m_char[128 + 1];
	};

	// CharTypesTmpTable
	// --------------
	class CharTypesTmpTable : public exodbc::wxDbTable
	{
	public:

		CharTypesTmpTable(exodbc::wxDb* pDb);
		virtual ~CharTypesTmpTable() {};

		SQLINTEGER	m_idCharTypes;
		SQLWCHAR	m_varchar[128 + 1];
		SQLWCHAR	m_char[128 + 1];
	};

	// IntTypesTable
	// ----------
	class IntTypesTable : public exodbc::wxDbTable
	{
	public:

		IntTypesTable(exodbc::wxDb* pDb);
		virtual ~IntTypesTable() {};

		// Size of Type							Bytes		Min						Max
		SQLINTEGER		m_idIntegerTypes;	//	4
		
		SQLSMALLINT		m_smallInt;			//	2			-32768 					32767
		SQLUSMALLINT	m_usmallInt;		//	2 			0						65535

		SQLINTEGER		m_int;				//	4			-2147483648 			2147483647
		SQLUINTEGER		m_uint;				//	8			0						4294967295
		
		SQLBIGINT		m_bigInt;			//	8 			-9223372036854775808 	9223372036854775807
		SQLUBIGINT		m_ubigInt;			//	8 			0						18446744073709551615
	};

	// IntTypesTmpTable
	// ----------
	class IntTypesTmpTable : public exodbc::wxDbTable
	{
	public:

		IntTypesTmpTable(exodbc::wxDb* pDb);
		virtual ~IntTypesTmpTable() {};

		// Size of Type							Bytes		Min						Max
		SQLINTEGER		m_idIntegerTypes;	//	4

		SQLSMALLINT		m_smallInt;			//	2			-32768 					32767
		SQLUSMALLINT	m_usmallInt;		//	2 			0						65535

		SQLINTEGER		m_int;				//	4			-2147483648 			2147483647
		SQLUINTEGER		m_uint;				//	8			0						4294967295

		SQLBIGINT		m_bigInt;			//	8 			-9223372036854775808 	9223372036854775807
		SQLUBIGINT		m_ubigInt;			//	8 			0						18446744073709551615
	};

	// DateTypesTable
	// --------------
	class DateTypesTable : public exodbc::wxDbTable
	{
	public:
		DateTypesTable(exodbc::wxDb* pDb);
		virtual ~DateTypesTable() {};

		SQLINTEGER				m_idDateTypes;
		SQL_DATE_STRUCT			m_date;
		SQL_TIME_STRUCT			m_time;
		SQL_TIMESTAMP_STRUCT	m_timestamp;
	};

	// DateTypesTmpTable
	// --------------
	class DateTypesTmpTable : public exodbc::wxDbTable
	{
	public:
		DateTypesTmpTable(exodbc::wxDb* pDb);
		virtual ~DateTypesTmpTable() {};

		SQLINTEGER				m_idDateTypes;
		SQL_DATE_STRUCT			m_date;
		SQL_TIME_STRUCT			m_time;
		SQL_TIMESTAMP_STRUCT	m_timestamp;
	};

	// FloatTypesTable
	// --------------
	class FloatTypesTable : public exodbc::wxDbTable
	{
	public:
		FloatTypesTable(exodbc::wxDb* pDb);
		virtual ~FloatTypesTable() {};

		SQLINTEGER				m_idFloatTypes;
		SQLDOUBLE				m_double;
		SQLFLOAT				m_float;
	};

	// FloatTypesTmpTable
	// ------------------
	class FloatTypesTmpTable : public exodbc::wxDbTable
	{
	public:
		FloatTypesTmpTable(exodbc::wxDb* pDb);
		virtual ~FloatTypesTmpTable() {};

		SQLINTEGER				m_idFloatTypes;
		SQLDOUBLE				m_double;
		SQLFLOAT				m_float;
	};

	// NumbericTypesTable
	// ------------------
	class NumericTypesTable : public exodbc::wxDbTable
	{
	public:
		enum ReadMode { ReasAsNumeric, ReadAsChar };
		NumericTypesTable(exodbc::wxDb* pDb, ReadMode readMode);
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
	class NumericTypesTmpTable : public exodbc::wxDbTable
	{
	public:
		// TODO: This name is not correct. ? The test write as "numer".. mmh
		enum ReadMode { ReasAsNumeric, ReadAsChar };
		NumericTypesTmpTable(exodbc::wxDb* pDb, ReadMode readMode);
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
	class BlobTypesTable : public exodbc::wxDbTable
	{
	public:
		BlobTypesTable(exodbc::wxDb* pDb);
		virtual ~BlobTypesTable() {};

		SQLINTEGER		m_idBlobTypes;
		SQLCHAR			m_blob[16];
	};

	// BlobTypesTmpTable
	// --------------
	class BlobTypesTmpTable : public exodbc::wxDbTable
	{
	public:
		BlobTypesTmpTable(exodbc::wxDb* pDb);
		virtual ~BlobTypesTmpTable() {};

		SQLINTEGER		m_idBlobTypes;
		SQLCHAR			m_blob[16];
	};

	// CharTable
	// ---------
	class CharTable : public exodbc::wxDbTable
	{
	public:
		CharTable(exodbc::wxDb* pDb);
		virtual ~CharTable() {};

		SQLINTEGER	m_idCharTable;
		SQLWCHAR	m_col2[128 + 1];
		SQLWCHAR	m_col3[128 + 1];
		SQLWCHAR	m_col4[128 + 2];
	};

	// IncompleCharTable
	class IncompleteCharTable : public exodbc::wxDbTable
	{
	public:
		IncompleteCharTable(exodbc::wxDb* pDb);
		virtual ~IncompleteCharTable() {};

		SQLINTEGER	m_idCharTable;
		SQLWCHAR	m_col2[128 + 1];
		// we do not bind col3 and have a gap
		//SQLWCHAR	m_col3[128 + 1];
		SQLWCHAR	m_col4[128 + 2];
	};
}