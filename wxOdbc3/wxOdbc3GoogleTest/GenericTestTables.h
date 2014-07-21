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
#include <stdint.h>

// Forward declarations
// --------------------
class wxDb;

// Structs
// -------

namespace wxOdbc3Test
{

	// NotExistingTable
	// ----------------
	class NotExistingTable : public wxDbTable
	{
	public:
		NotExistingTable(wxDb* pDb);
		~NotExistingTable() {};

		int		m_idNotExisting;
	};

	// CharTypesTable
	// --------------
	class CharTypesTable : public wxDbTable
	{
	public:

		CharTypesTable(wxDb* pDb);
		virtual ~CharTypesTable() {};

		SQLINTEGER	m_idCharTypes;
		SQLWCHAR	m_varchar[128 + 1];
		SQLWCHAR	m_char[128 + 1];
	};

	// CharTypesTmpTable
	// --------------
	class CharTypesTmpTable : public wxDbTable
	{
	public:

		CharTypesTmpTable(wxDb* pDb);
		virtual ~CharTypesTmpTable() {};

		SQLINTEGER	m_idCharTypes;
		SQLWCHAR	m_varchar[128 + 1];
		SQLWCHAR	m_char[128 + 1];
	};

	// IntTypesTable
	// ----------
	class IntTypesTable : public wxDbTable
	{
	public:

		IntTypesTable(wxDb* pDb);
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
	class IntTypesTmpTable : public wxDbTable
	{
	public:

		IntTypesTmpTable(wxDb* pDb);
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
	class DateTypesTable : public wxDbTable
	{
	public:
		DateTypesTable(wxDb* pDb);
		virtual ~DateTypesTable() {};

		SQLINTEGER				m_idDateTypes;
		SQL_DATE_STRUCT			m_date;
		SQL_TIME_STRUCT			m_time;
		SQL_TIMESTAMP_STRUCT	m_timestamp;
	};

	// DateTypesTmpTable
	// --------------
	class DateTypesTmpTable : public wxDbTable
	{
	public:
		DateTypesTmpTable(wxDb* pDb);
		virtual ~DateTypesTmpTable() {};

		SQLINTEGER				m_idDateTypes;
		SQL_DATE_STRUCT			m_date;
		SQL_TIME_STRUCT			m_time;
		SQL_TIMESTAMP_STRUCT	m_timestamp;
	};

	// FloatTypesTable
	// --------------
	class FloatTypesTable : public wxDbTable
	{
	public:
		FloatTypesTable(wxDb* pDb);
		virtual ~FloatTypesTable() {};

		SQLINTEGER				m_idFloatTypes;
		SQLDOUBLE				m_double;
		SQLFLOAT				m_float;
	};

	// NumbericTypesTable
	// ------------------
	class NumericTypesTable : public wxDbTable
	{
	public:
		enum ReadMode { ReasAsNumeric, ReadAsChar };
		NumericTypesTable(wxDb* pDb, ReadMode readMode);
		virtual ~NumericTypesTable() {};

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
	class BlobTypesTable : public wxDbTable
	{
	public:
		BlobTypesTable(wxDb* pDb);
		virtual ~BlobTypesTable() {};

		SQLINTEGER		m_idBlobTypes;
		SQLCHAR			m_blob[16];
	};
}