

/*!
 * \file GenericTestTables.h
 * \author Elias Gerber <eg@zame.ch>
 * \date 23.02.2014
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef GENERICTESTTABLES_H
#define GENERICTESTTABLES_H

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


#endif // GENERICTESTTABLES_H
