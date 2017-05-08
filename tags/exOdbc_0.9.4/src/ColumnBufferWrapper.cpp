/*!
* \file ColumnBufferWrapper.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 25.03.2017
* \brief Source file for ColumnBufferWrapper.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "ColumnBufferWrapper.h"

// Same component headers
#include "SpecializedExceptions.h"

// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{

	SQLSMALLINT ColumnBufferWrapper::GetSqlCType() const
	{
		SqlCTypeVisitor visitor;
		SQLSMALLINT sqlCType = boost::apply_visitor(visitor, m_columnBufferVariant);
		return sqlCType;
	}


	bool ColumnBufferWrapper::IsNull() const
	{
		return boost::apply_visitor(IsNullVisitor(), m_columnBufferVariant);
	}


	void ColumnBufferWrapper::SetNull()
	{
		boost::apply_visitor(SetNullVisitor(), m_columnBufferVariant);
	}
}
