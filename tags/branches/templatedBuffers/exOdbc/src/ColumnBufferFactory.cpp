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
#include "SqlCBuffer.h"

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
		// �6.7 [stmt.dcl] p4:
		// If control enters the declaration concurrently while the variable is being initialized, 
		// the concurrent execution shall wait for completion of the initialization.
		static ColumnBufferFactory instance;
		return instance;
	}


	void ColumnBufferFactory::RegisterColumnBufferCreationFunc(SQLSMALLINT sqlCBufferType, BufferCreationFunc func)
	{
//		std::unique_lock<std::mutex> lock(m_creatorFuncsMutex);

		// Allow a type to be registered only once
		exASSERT(m_creatorFuncs.find(sqlCBufferType) == m_creatorFuncs.end());
		
		m_creatorFuncs[sqlCBufferType] = func;
	}


	IColumnBufferPtr ColumnBufferFactory::CreateColumnBuffer(SQLSMALLINT sqlCBufferType)
	{
//		std::lock_guard<std::mutex> lock(m_creatorFuncsMutex);
		BufferCreatorFuncsMap::const_iterator it = m_creatorFuncs.find(sqlCBufferType);
		if (it == m_creatorFuncs.end())
		{
			NotFoundException nfe(boost::str(boost::wformat(L"No Creator Function registered for SqlCBuffer type %s (%d)") % SqLCType2s(sqlCBufferType) % sqlCBufferType));
			SET_EXCEPTION_SOURCE(nfe);
			throw nfe;
		}

		return it->second(sqlCBufferType);
	}


	class Foo
	{
	public:
		void Bar(int i) const {};
	};

	boost::any ColumnBufferFactory::CreateSqlCBuffer(SQLSMALLINT sqlCBufferType) const
	{
		SqlCBufferAnyWrapper wrapper;

		std::function<void(const Foo&, int)> f_add_display = &Foo::Bar;
//		std::function<void(const Foo&)> f = &Foo::Bar;

		switch (sqlCBufferType)
		{
		case SQL_C_USHORT:
			wrapper.m_sqlCBuffer = SqlUShortBuffer();
			break;
		case SQL_C_SSHORT:
			return SqlShortBuffer();
		case SQL_C_ULONG:
			return SqlULongBuffer();
		case SQL_C_SLONG:
			return SqlLongBuffer();
		case SQL_C_UBIGINT:
			return SqlUBigIntBuffer();
		case SQL_C_SBIGINT:
			return SqlBigIntBuffer();
		}

		NotSupportedException nse(NotSupportedException::Type::SQL_C_TYPE, sqlCBufferType);
		SET_EXCEPTION_SOURCE(nse);
		throw nse;
	}

	// Interfaces
	// ----------
}
