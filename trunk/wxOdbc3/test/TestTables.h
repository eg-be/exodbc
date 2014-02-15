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


// QueryTypesTable
// ----------
class IntTypesTable : public wxDbTable
{
public:
	IntTypesTable(wxDb* pDb);
	~IntTypesTable() {};

	
	int32_t		m_idQueryTypes;
};


#endif // TESTTABLES_H
