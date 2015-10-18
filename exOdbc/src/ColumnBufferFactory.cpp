/*!
* \file ColumnBufferFactory.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 18.10.2015
* \brief Source file for info objects.
* \copyright GNU Lesser General Public License Version 3
*
*/

#include "stdafx.h"

// Own header
#include "ColumnBufferFactory.h"

// Same component headers
#include "Exception.h"

// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	// Construction
	// ------------


	// Implementation
	// --------------
	ColumnBufferFactory& ColumnBufferFactory::Instance() throw()
	{
		// this should be thread safe in c++11, see:
		// §6.7 [stmt.dcl] p4:
		// If control enters the declaration concurrently while the variable is being initialized, 
		// the concurrent execution shall wait for completion of the initialization.
		static ColumnBufferFactory instance;
		return instance;
	}


	void ColumnBufferFactory::RegisterColumnBufferCreationFunc(SQLSMALLINT sqlCBufferType, BufferCreationFunc func)
	{
		// Allow a type to be registered only once
		exASSERT(m_creatorFuncs.find(sqlCBufferType) == m_creatorFuncs.end());
		
		m_creatorFuncs[sqlCBufferType] = func;
	}


	std::shared_ptr<IColumnBuffer> ColumnBufferFactory::CreateColumnBuffer(SQLSMALLINT sqlCBufferType) const
	{
		BufferCreatorFuncsMap::const_iterator it = m_creatorFuncs.find(sqlCBufferType);
		if (it == m_creatorFuncs.end())
		{
			NotFoundException nfe(boost::str(boost::wformat(L"No Creator Function registered for SqlCBuffer type %s (%d)") % SqLCType2s(sqlCBufferType) % sqlCBufferType));
			SET_EXCEPTION_SOURCE(nfe);
			throw nfe;
		}

		return it->second(sqlCBufferType);
	}

	// Interfaces
	// ----------
}
