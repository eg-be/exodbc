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

		int32_t		m_idCharTypes;
		wchar_t		m_varchar[128 + 1];
		wchar_t		m_char[128 + 1];
	};
}