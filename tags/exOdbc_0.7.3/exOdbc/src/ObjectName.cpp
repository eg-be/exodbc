/*!
* \file ObjectName.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2015
* \brief Source file for name objects.
* \copyright wxWindows Library Licence, Version 3.1
*
*/

#include "stdafx.h"

// Own header
#include "ObjectName.h"

// Same component headers

// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	ObjectName::~ObjectName()
	{ }


	bool ObjectName::operator==(const ObjectName& other) const
	{
		// If the query name matches, they are equal
		if (other.GetQueryName() == GetQueryName())
		{
			return true;
		}
		return false;
	}
}
