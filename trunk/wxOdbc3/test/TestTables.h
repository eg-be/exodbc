/*!
 * \file TestTables.h
 * \author Elias Gerber <egerber@gmx.net>
 * \date 09.02.2014
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef TESTTABLES_H
#define TESTTABLES_H

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

// NotExistingTable
// ----------------
class NotExistingTable : public wxDbTable
{
public:
	NotExistingTable(wxDb* pDb);
	~NotExistingTable() {};

	int		m_idNotExisting;
};


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

#endif // TESTTABLES_H
