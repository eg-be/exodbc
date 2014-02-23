/*!
* \file MySqlTestTables.h
* \author Elias Gerber <eg@zame.ch>
* \date 09.02.2014
* 
* [Brief Header-file description]
*/ 

#pragma once
#ifndef MYSQLTESTTABLES_H
#define MYSQLTESTTABLES_H

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

namespace MySql
{


	// IntTypesTable
	// ----------
	class IntTypesTable : public wxDbTable
	{
	public:

		IntTypesTable(wxDb* pDb);
		virtual ~IntTypesTable() {};

		// Sizes of Types					Bytes		Min						Max
		int32_t		m_idIntegerTypes;	//	4
		int8_t		m_tinyInt;			//	1			-128 					127
		int16_t		m_smallInt;			//	2			-32768 					32767
		int32_t		m_mediumInt;		//	3			-8388608 				8388607
		int32_t		m_int;				//	4			-2147483648 			2147483647
		int64_t		m_bigInt;			//	8			-9223372036854775808 	9223372036854775807

		uint8_t		m_utinyInt;			//	1			0 						255
		uint16_t	m_usmallInt;		//	2 			0						65535
		uint32_t	m_umediumInt;		//	3			0					 	16777215
		uint32_t	m_uint;				//	4 			0						4294967295
		uint64_t	m_ubigInt;			//	8 			0						18446744073709551615

	};

	// CharTypesTable
	// --------------
	class CharTypesTable : public wxDbTable
	{
	public:

		CharTypesTable(wxDb* pDb);
		virtual ~CharTypesTable() {};

		int32_t		m_idCharTypes;
		wchar_t		m_varchar[128 + 1];
		wchar_t		m_char[128 + 1];
	};

	// FloatTypesTable
	// ---------------
	class FloatTypesTable : public wxDbTable
	{
	public:
		FloatTypesTable(wxDb* pDb);
		virtual ~FloatTypesTable() {};

		int32_t				m_idFloatTypes;
		double				m_float;
		double				m_double;
		wchar_t				m_decimal_15_10[15 + 3];
		wchar_t				m_decimal_10_0[10 + 3];
		//SQL_NUMERIC_STRUCT	m_decimal;
	};

	// DateTypesTable
	// --------------
	class DateTypesTable : public wxDbTable
	{
	public:
		DateTypesTable(wxDb* pDb);
		virtual ~DateTypesTable() {};

		int32_t					m_idDateTypes;
		SQL_DATE_STRUCT			m_date;
		SQL_TIMESTAMP_STRUCT	m_datetime;
		SQL_TIME_STRUCT			m_time;
		SQL_TIME_STRUCT			m_timestamp;
	};

}

#endif // MYSQLTESTTABLES_H
