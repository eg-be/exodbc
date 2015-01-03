/*!
* \file ColumnBuffer.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 23.11.2014
* \brief Source file for the ColumnBuffer class and its helpers.
*
*/

#include "stdafx.h"

// Own header
#include "ColumnBuffer.h"

// Same component headers
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
	ColumnBuffer::ColumnBuffer(const SColumnInfo& columnInfo, AutoBindingMode mode, OdbcVersion odbcVersion, ColumnFlags flags /* = CF_SELECT */)
		: m_allocatedBuffer(false)
		, m_haveBuffer(false)
		, m_autoBindingMode(mode)
		, m_bufferType(0)
		, m_bufferSize(0)
		, m_columnNr((SQLUSMALLINT) columnInfo.m_ordinalPosition)
		, m_odbcVersion(odbcVersion)
		, m_decimalDigits(-1)
		, m_columnSize(-1)
		, m_hStmt(SQL_NULL_HSTMT)
		, m_flags(flags)
	{
		exASSERT(m_columnNr > 0);
		exASSERT(columnInfo.m_sqlDataType != 0);
		exASSERT(m_flags & CF_SELECT);

		// Remember some values from SColumnInfo
		m_queryName = columnInfo.GetSqlName();
		if (!columnInfo.m_isColumnSizeNull)
		{
			m_columnSize = columnInfo.m_columnSize;
		}
		if (!columnInfo.m_isDecimalDigitsNull)
		{
			m_decimalDigits = columnInfo.m_decimalDigits;
		}
		m_sqlType = columnInfo.m_sqlType;

		// Create buffer
		m_bufferType = DetermineBufferType(m_sqlType);
		exASSERT(m_bufferType != 0);
		m_bufferSize = DetermineBufferSize(columnInfo);
		exASSERT(m_bufferSize > 0);

		m_allocatedBuffer = AllocateBuffer(m_bufferType, m_bufferSize);
		m_haveBuffer = m_allocatedBuffer;
	}


	ColumnBuffer::ColumnBuffer(SQLSMALLINT sqlCType, SQLUSMALLINT ordinalPosition, BufferPtrVariant bufferPtrVariant, SQLLEN bufferSize, const std::wstring& queryName, ColumnFlags flags /* = CF_SELECT */, SQLINTEGER columnSize /* = -1 */, SQLSMALLINT decimalDigits /* = -1 */, SQLSMALLINT sqlType /* = SQL_UNKNOWN_TYPE */)
		: m_allocatedBuffer(false)
		, m_haveBuffer(true)
		, m_autoBindingMode(AutoBindingMode::BIND_AS_REPORTED)
		, m_bufferType(sqlCType)
		, m_bufferSize(bufferSize)
		, m_columnNr(ordinalPosition)
		, m_bufferPtr(bufferPtrVariant)
		, m_queryName(queryName)
		, m_odbcVersion(OV_UNKNOWN)
		, m_decimalDigits(decimalDigits)
		, m_columnSize(columnSize)
		, m_hStmt(SQL_NULL_HSTMT)
		, m_sqlType(sqlType)
		, m_flags(flags)
	{
		exASSERT(sqlCType != 0);
		exASSERT(ordinalPosition > 0);
		exASSERT(bufferSize > 0);
		exASSERT(!m_queryName.empty());
		exASSERT(m_flags & CF_SELECT);
	}

	// Destructor
	// -----------
	ColumnBuffer::~ColumnBuffer()
	{
		// Unbind column
		if (IsBound())
		{
			Unbind(m_hStmt);
		}

		// Unbind all parameters
		for (BoundParameterPtrsVector::const_iterator it = m_boundParameters.begin(); it != m_boundParameters.end(); it++)
		{
			BoundParameter* pParam = *it;
			UnbindParameter(pParam->m_hStmt, pParam->m_parameterNumber);
			delete pParam;
		}
		m_boundParameters.clear();
		
		// Free buffer
		if (m_allocatedBuffer)
		{
			try
			{
				switch (m_bufferType)
				{
				case SQL_C_SSHORT:
					delete GetSmallIntPtr();
					break;
				case SQL_C_SLONG:
					delete GetIntPtr();
					break;
				case SQL_C_SBIGINT:
					delete GetBigIntPtr();
					break;
				case SQL_C_CHAR:
				case SQL_C_BINARY:
					delete[] GetCharPtr();
					break;
				case SQL_C_WCHAR:
					delete[] GetWCharPtr();
					break;
				case SQL_C_DOUBLE:
					delete GetDoublePtr();
					break;
				case SQL_C_TYPE_DATE:
				case SQL_C_DATE:
					delete GetDatePtr();
					break;
				case SQL_C_TYPE_TIME:
				case SQL_C_TIME:
					delete GetTimePtr();
					break;
				case SQL_C_TYPE_TIMESTAMP:
				case SQL_C_TIMESTAMP:
					delete GetTimestampPtr();
					break;
#if HAVE_MSODBCSQL_H
				case SQL_C_SS_TIME2:
					delete GetTime2Ptr();
					break;
#endif
				case SQL_C_NUMERIC:
					delete GetNumericPtr();
					break;
				default:
					exASSERT(false);
				}
			}
			catch (boost::bad_get ex)
			{
				LOG_ERROR(ex.what());
			}
		}
	}


	// Implementation
	// --------------
	bool ColumnBuffer::BindParameter(SQLHSTMT hStmt, SQLSMALLINT parameterNumber)
	{
		exASSERT(m_haveBuffer);
		exASSERT(hStmt != SQL_NULL_HSTMT);
		exASSERT(m_bufferType != SQL_UNKNOWN_TYPE);
		exASSERT(m_bufferSize > 0);
		exASSERT(parameterNumber > 0);

		void* pBuffer = NULL;
		try
		{
			pBuffer = GetBuffer();
		}
		catch (boost::bad_get ex)
		{
			LOG_ERROR(L"Failed in GetBuffer() - probably allocated buffer type does not match sql type in SColumnInfo.");
			exASSERT(false);
			return false;
		}
		BoundParameter* pBp = new BoundParameter(hStmt, parameterNumber);
		SQLRETURN ret;
		if (m_bufferType != SQL_C_NUMERIC)
		{
			ret = SQLBindParameter(pBp->m_hStmt, pBp->m_parameterNumber, SQL_PARAM_INPUT, m_bufferType, m_sqlType, 0, 0, pBuffer, m_bufferSize, &(pBp->m_cb));
		}
		if (!SQL_SUCCEEDED(ret))
		{
			LOG_ERROR_STMT(hStmt, ret, SQLBindParameter);
			delete pBp;
		}
		if (ret == SQL_SUCCESS_WITH_INFO)
		{
			LOG_WARNING_STMT(hStmt, ret, SQLBindParameter);
		}
		if (SQL_SUCCEEDED(ret))
		{
			m_boundParameters.push_back(pBp);
		}

		return SQL_SUCCEEDED(ret);
	}


	bool ColumnBuffer::UnbindParameter(SQLHSTMT hStmt, SQLSMALLINT parameterNumber)
	{
		exASSERT(m_haveBuffer);
		exASSERT(hStmt != SQL_NULL_HSTMT);
		exASSERT(m_bufferType != SQL_UNKNOWN_TYPE);
		exASSERT(parameterNumber > 0);

		// \todo: Workaround for Ticket #59
		//SQLRETURN ret = SQLBindParameter(hStmt, parameterNumber, SQL_PARAM_INPUT, m_bufferType, m_sqlType, 0, 0, NULL, 0, NULL);
		//if (!SQL_SUCCEEDED(ret))
		//{
		//	LOG_ERROR_STMT(hStmt, ret, SQLBindParameter);
		//}
		SQLRETURN ret = SQLFreeStmt(hStmt, SQL_UNBIND);
		if (!SQL_SUCCEEDED(ret))
		{
			LOG_ERROR_STMT(hStmt, ret, SQLBindParameter);
		}

		return SQL_SUCCEEDED(ret);
	}


	bool ColumnBuffer::Unbind(SQLHSTMT hStmt)
	{
		exASSERT(IsBound());
		exASSERT(m_bufferType != SQL_UNKNOWN_TYPE);
		if (m_bufferType == SQL_C_NUMERIC)
		{
			SQLHDESC hDesc = SQL_NULL_HDESC;
			if (GetRowDescriptorHandle(hStmt, hDesc))
			{
				bool ok = true;
				ok = ok & SetDescriptionField(hDesc, m_columnNr, SQL_DESC_TYPE, m_bufferType);
				ok = ok & SetDescriptionField(hDesc, m_columnNr, SQL_DESC_DATA_PTR, (SQLINTEGER) NULL);
				ok = ok & SetDescriptionField(hDesc, m_columnNr, SQL_DESC_INDICATOR_PTR, (SQLINTEGER) NULL);
				ok = ok & SetDescriptionField(hDesc, m_columnNr, SQL_DESC_OCTET_LENGTH_PTR, (SQLINTEGER) NULL);
				if (ok)
				{
					m_hStmt = SQL_NULL_HSTMT;
				}
			}
		}
		else
		{
			SQLRETURN ret = SQLBindCol(m_hStmt, m_columnNr, m_bufferType, NULL, 0, NULL);
			if (SQL_SUCCEEDED(ret))
			{
				m_hStmt = SQL_NULL_HSTMT;
			}
			else
			{
				LOG_ERROR_STMT(m_hStmt, ret, SQLBindCol);
			}
		}
		return m_hStmt == SQL_NULL_HSTMT;
	}



	bool ColumnBuffer::Bind(SQLHSTMT hStmt)
	{
		exASSERT(m_haveBuffer);
		exASSERT(m_hStmt == SQL_NULL_HSTMT);
		exASSERT(hStmt != SQL_NULL_HSTMT);
		exASSERT(m_bufferType != SQL_UNKNOWN_TYPE);
		exASSERT(m_bufferSize > 0);

		void* pBuffer = NULL;
		try
		{
			pBuffer = GetBuffer();
		}
		catch (boost::bad_get ex)
		{
			LOG_ERROR(L"Failed in GetBuffer() - probably allocated buffer type does not match sql type in SColumnInfo.");
			exASSERT(false);
			return false;
		}

		// If we want to bind a numeric type, we must set the precision and scale before binding!
		// And binding somehow does only work if we set the Attributes manually using SetColDescField:
		// If we bind and set then the ColDescFields PRECISION and SCALE data is only transfered to the
		// NUMERIC_STRUCT if we explicitly call SQLGetData(.., SQL_ARD_TYPE, ..):
		// Notice that the TargetType (3rd Parameter) is SQL_ARD_TYPE, which  
		// forces the driver to use the Application Row Descriptor with the 
		// specified scale and precision.
		// But then we could also just not bind the column and transfer its data manually.
		// Therefore: Instead of calling SQLBindCol bind the columns manually using SetColDescField
		//
		// \note: Not sure: Maybe we would need to call SQLBindCol first? At least its done like that here:
		// http://www.ionu.ro/page/2/ - the only really working example I found on the web.
		// As the tests run fine without it, leave it out. I think its not needed, it will do nothing else
		// than what we do with the SetDescriptionField.
		// But it is important to set all those attributes, including octet_length_ptr! maybe the order
		// of setting them is also important.
		if (m_bufferType == SQL_C_NUMERIC)
		{
			exASSERT(m_columnSize > 0);
			exASSERT(m_decimalDigits >= 0);

			SQLHDESC hDesc = SQL_NULL_HDESC;
			if (GetRowDescriptorHandle(hStmt, hDesc))
			{
				bool ok = true;
				ok = ok & SetDescriptionField(hDesc, m_columnNr, SQL_DESC_TYPE, m_bufferType);
				ok = ok & SetDescriptionField(hDesc, m_columnNr, SQL_DESC_PRECISION, m_columnSize);
				ok = ok & SetDescriptionField(hDesc, m_columnNr, SQL_DESC_SCALE, m_decimalDigits);
				ok = ok & SetDescriptionField(hDesc, m_columnNr, SQL_DESC_DATA_PTR, (SQLINTEGER)pBuffer);
				ok = ok & SetDescriptionField(hDesc, m_columnNr, SQL_DESC_INDICATOR_PTR, (SQLINTEGER)&m_cb);
				ok = ok & SetDescriptionField(hDesc, m_columnNr, SQL_DESC_OCTET_LENGTH_PTR, (SQLINTEGER)&m_cb);
				if (ok)
				{
					m_hStmt = hStmt;
				}
			}
			if (m_hStmt == SQL_NULL_HSTMT)
			{
				// Something went wrong, the function above has already logged some detail.
				LOG_ERROR((boost::wformat(L"Failed to bind Numeric Column '%s' (columnNr: %d)") %m_queryName %m_columnNr).str());
			}
		}
		else
		{
			// Non numeric columns pass the tests fine by using just SQLBindCol
			SQLRETURN ret = SQLBindCol(hStmt, m_columnNr, m_bufferType, (SQLPOINTER*)pBuffer, m_bufferSize, &m_cb);
			// Note: We check on purpose here only for SUCCESS, we do not tolerate loosing precision
			if (ret != SQL_SUCCESS)
			{
				LOG_ERROR_STMT(hStmt, ret, SQLBindCol);
				m_hStmt = SQL_NULL_HSTMT;
			}
			m_hStmt = (ret == SQL_SUCCESS) ? hStmt : SQL_NULL_HSTMT;
		}

		return m_hStmt != SQL_NULL_HSTMT;
	}


	bool ColumnBuffer::AllocateBuffer(SQLSMALLINT bufferType, SQLINTEGER bufferSize)
	{
		exASSERT(!m_allocatedBuffer);
		exASSERT(!IsBound());
		exASSERT(!m_haveBuffer);
		exASSERT(bufferType != SQL_UNKNOWN_TYPE);
		exASSERT(bufferSize > 0);

		bool failed = false;
		switch (bufferType)
		{
		case SQL_C_SSHORT:
			m_bufferPtr = new SQLSMALLINT(0);
			break;
		case SQL_C_SLONG:
			m_bufferPtr = new SQLINTEGER(0);
			break;
		case SQL_C_SBIGINT:
			m_bufferPtr = new SQLBIGINT(0);
			break;
		case SQL_C_CHAR:
		case SQL_C_BINARY:
			m_bufferPtr = new SQLCHAR[bufferSize];
			break;
		case SQL_C_WCHAR:
			m_bufferPtr = new SQLWCHAR[bufferSize];
			break;
		case SQL_C_DOUBLE:
			m_bufferPtr = new SQLDOUBLE(0.0);
			break;
		case SQL_C_TYPE_DATE:
			m_bufferPtr = new SQL_DATE_STRUCT;
			break;
		case SQL_C_TYPE_TIME:
			m_bufferPtr = new SQL_TIME_STRUCT;
			break;
		case SQL_C_TYPE_TIMESTAMP:
			m_bufferPtr = new SQL_TIMESTAMP_STRUCT;
			break;
#if HAVE_MSODBCSQL_H
		case SQL_C_SS_TIME2:
			m_bufferPtr = new SQL_SS_TIME2_STRUCT;
			break;
#endif
		case SQL_C_NUMERIC:
			m_bufferPtr = new SQL_NUMERIC_STRUCT;
			break;
		default:
			LOG_ERROR((boost::wformat(L"Not implemented SqlDataType '%s' (%d)") % SqlType2s(bufferType) % bufferType).str());
			failed = true;
		}

		if (!failed)
		{
			m_allocatedBuffer = true;
		}
		return m_allocatedBuffer;
	}


	void* ColumnBuffer::GetBuffer() const
	{
		exASSERT(m_haveBuffer);

		void* pBuffer = NULL;
		switch (m_bufferType)
		{
		case SQL_C_SSHORT:
			return static_cast<void*>(boost::get<SQLSMALLINT*>(m_bufferPtr));
		case SQL_C_SLONG:
			return static_cast<void*>(boost::get<SQLINTEGER*>(m_bufferPtr));
		case SQL_C_SBIGINT:
			return static_cast<void*>(boost::get<SQLBIGINT*>(m_bufferPtr));
		case SQL_C_CHAR:
		case SQL_C_BINARY:
			return static_cast<void*>(boost::get<SQLCHAR*>(m_bufferPtr));
		case SQL_C_WCHAR:
			return static_cast<void*>(boost::get<SQLWCHAR*>(m_bufferPtr));
		case SQL_C_DOUBLE:
			return static_cast<void*>(boost::get<SQLDOUBLE*>(m_bufferPtr));
		case SQL_C_TYPE_DATE:
		case SQL_C_DATE:
			return static_cast<void*>(boost::get<SQL_DATE_STRUCT*>(m_bufferPtr));
		case SQL_C_TYPE_TIME:
		case SQL_C_TIME:
			return static_cast<void*>(boost::get<SQL_TIME_STRUCT*>(m_bufferPtr));
		case SQL_C_TYPE_TIMESTAMP:
		case SQL_C_TIMESTAMP:
			return static_cast<void*>(boost::get<SQL_TIMESTAMP_STRUCT*>(m_bufferPtr));
#if HAVE_MSODBCSQL_H
		case SQL_C_SS_TIME2:
			return static_cast<void*>(boost::get<SQL_SS_TIME2_STRUCT*>(m_bufferPtr));
#endif
		case SQL_C_NUMERIC:
			return static_cast<void*>(boost::get<SQL_NUMERIC_STRUCT*>(m_bufferPtr));
		default:
			exASSERT(false);
		}
		return pBuffer;
	}


	SQLINTEGER ColumnBuffer::DetermineCharSize(const SColumnInfo& columnInfo) const
	{
		exASSERT(columnInfo.m_sqlType != SQL_UNKNOWN_TYPE);

		switch (columnInfo.m_sqlType)
		{
		case SQL_SMALLINT:
		case SQL_INTEGER:
		case SQL_BIGINT:
			if (columnInfo.m_isColumnSizeNull || columnInfo.m_isNumPrecRadixNull || columnInfo.m_numPrecRadix != 10)
			{
				// just return some silly default value
				return DB_MAX_BIGINT_CHAR_LENGTH + 1;
			}
			if (!columnInfo.m_isDecimalDigitsNull && columnInfo.m_decimalDigits > 0)
			{
				// +3: 1 for '.' and one for trailing zero and one for a '-'
				return columnInfo.m_columnSize + 3; 
			}
			else
			{
				// +2: one for trailing zero and one for '-'
				return columnInfo.m_columnSize + 2;
			}
		case SQL_CHAR:
		case SQL_WCHAR:
		case SQL_VARCHAR:
		case SQL_WVARCHAR:
			// TODO: We could also calculate using the char_octet_length. Maybe this would be cleaner - some dbs
			// report higher values there than we calculate (like sizeof(SQLWCHAR) would be 3)
			return columnInfo.m_columnSize + 1;
		case SQL_DOUBLE:
		case SQL_FLOAT:
		case SQL_REAL:
			if (!columnInfo.m_isNumPrecRadixNull && columnInfo.m_numPrecRadix == 10)
			{
				// +3: 1 for '.' and one for trailing zero and one for a '-'
				return columnInfo.m_columnSize + 3;
			}
			else
			{
				// just return some silly default value
				return DB_MAX_DOUBLE_CHAR_LENGTH + 1;
			}
		case SQL_TYPE_DATE:
		case SQL_TYPE_TIME:
		case SQL_TYPE_TIMESTAMP:
#if HAVE_MSODBCSQL_H
		case SQL_SS_TIME2:
#endif
			exASSERT(!columnInfo.m_isColumnSizeNull);
			return columnInfo.m_columnSize + 1;
		default:
			LOG_ERROR((boost::wformat(L"Not implemented SqlDataType '%s' (%d)") % SqlType2s(columnInfo.m_sqlType) % columnInfo.m_sqlType).str());
		}

		return 0;
	}


	SQLINTEGER ColumnBuffer::DetermineBufferSize(const SColumnInfo& columnInfo) const
	{
		exASSERT(m_bufferType != 0);
		exASSERT(columnInfo.m_sqlDataType != SQL_UNKNOWN_TYPE);

		// if the determined buffer type is a simple type its just sizeof
		switch (m_bufferType)
		{
		case SQL_C_SSHORT:
			return sizeof(SQLSMALLINT);
		case SQL_C_SLONG:
			return sizeof(SQLINTEGER);
		case SQL_C_SBIGINT:
			return sizeof(SQLBIGINT);
		case SQL_C_CHAR:
			return DetermineCharSize(columnInfo) * sizeof(SQLCHAR);
		case SQL_C_WCHAR:
			return DetermineCharSize(columnInfo) * sizeof(SQLWCHAR);
		case SQL_C_DOUBLE:
			return sizeof(SQLDOUBLE);
		case SQL_C_TYPE_DATE:
		case SQL_C_DATE:
			return sizeof(SQL_DATE_STRUCT);
		case SQL_C_TYPE_TIME:
		case SQL_C_TIME:
			return sizeof(SQL_TIME_STRUCT);
		case SQL_C_TYPE_TIMESTAMP:
		case SQL_C_TIMESTAMP:
			return sizeof(SQL_TIMESTAMP_STRUCT);
			break;
#if HAVE_MSODBCSQL_H
		case SQL_C_SS_TIME2:
			return sizeof(SQL_SS_TIME2_STRUCT);
			break;
#endif
		case SQL_C_BINARY:
			exASSERT(!columnInfo.m_isColumnSizeNull);
			return columnInfo.m_columnSize * sizeof(SQLCHAR);
		case SQL_C_NUMERIC:
			return sizeof(SQL_NUMERIC_STRUCT);
		default:
			LOG_ERROR((boost::wformat(L"Not implemented SqlDataType '%s' (%d)") % SqlType2s(columnInfo.m_sqlDataType) % columnInfo.m_sqlDataType).str());
		}
		return 0;
	}


	SQLSMALLINT ColumnBuffer::DetermineBufferType(SQLSMALLINT sqlType) const
	{
		exASSERT(sqlType != SQL_UNKNOWN_TYPE);

		if (m_autoBindingMode == AutoBindingMode::BIND_ALL_AS_CHAR)
		{
			return SQL_C_CHAR;
		}
		else if (m_autoBindingMode == AutoBindingMode::BIND_ALL_AS_WCHAR)
		{
			return SQL_C_WCHAR;
		}

		switch (sqlType)
		{
		case SQL_SMALLINT:
			return SQL_C_SSHORT;
		case SQL_INTEGER:
			return SQL_C_SLONG;
		case SQL_BIGINT:
			return SQL_C_SBIGINT;
		case SQL_CHAR:
		case SQL_VARCHAR:
			if (m_autoBindingMode == AutoBindingMode::BIND_WCHAR_AS_CHAR || m_autoBindingMode == AutoBindingMode::BIND_AS_REPORTED)
			{
				return SQL_C_CHAR;
			}
			else
			{
				return SQL_C_WCHAR;
			}
		case SQL_WCHAR:
		case SQL_WVARCHAR:
			if (m_autoBindingMode == AutoBindingMode::BIND_CHAR_AS_WCHAR || m_autoBindingMode == AutoBindingMode::BIND_AS_REPORTED)
			{
				return SQL_C_WCHAR;
			}
			else
			{
				return SQL_C_CHAR;
			}
		case SQL_DOUBLE:
		case SQL_FLOAT:
		case SQL_REAL:
			return SQL_C_DOUBLE;
			// not valid without TYPE ?? http://msdn.microsoft.com/en-us/library/ms710150%28v=vs.85%29.aspx
		case SQL_DATE:
		case SQL_TYPE_DATE:
			return SQL_C_TYPE_DATE;
		case SQL_TIME:
		case SQL_TYPE_TIME:
			return SQL_C_TYPE_TIME;
		case SQL_TIMESTAMP:
		case SQL_TYPE_TIMESTAMP:
			return SQL_C_TYPE_TIMESTAMP;
#if HAVE_MSODBCSQL_H
		case SQL_SS_TIME2:
			if (m_odbcVersion >= OV_3_8)
			{
				return SQL_C_SS_TIME2;
			}
			else
			{
				return SQL_C_TYPE_TIME;
			}
#endif
		case SQL_BINARY:
		case SQL_VARBINARY:
		case SQL_LONGVARBINARY:
			return SQL_C_BINARY;
		case SQL_NUMERIC:
		case SQL_DECIMAL:
			return SQL_C_NUMERIC;
		default:
			LOG_ERROR((boost::wformat(L"Not implemented SqlDataType '%s' (%d)") % SqlType2s(sqlType) % sqlType).str());
		}
		return 0;
	}


	void ColumnBuffer::SetPrimaryKey(bool isPrimaryKey /* = true */)
	{
		if (isPrimaryKey)
		{
			m_flags |= CF_PRIMARY_KEY;
		}
		else
		{
			m_flags &= !CF_PRIMARY_KEY;
		}
	}


	void ColumnBuffer::operator=(const BufferVariant& var)
	{
		exASSERT(m_bufferType != SQL_UNKNOWN_TYPE);
		exASSERT(m_haveBuffer);

		// \todo: assign value to ptr for all types, sometimes we will need to memcopy and need to know the length of the data
		// For chars we could assume its null-terminated if no length is passed
		// But we probably need a second operator which needs as parameter the length of the data for binary stuff
		if (SQL_C_SSHORT == m_bufferType)
		{
			SQLSMALLINT* pColVal = GetSmallIntPtr();
			*pColVal = boost::get<SQLSMALLINT>(var);
		}
		else if (SQL_C_SLONG == m_bufferType)
		{
			SQLINTEGER* pColVal = GetIntPtr();
			*pColVal = boost::get<SQLINTEGER>(var);
		}
		else if (SQL_C_SBIGINT == m_bufferType)
		{
			SQLBIGINT* pColVal = GetBigIntPtr();
			*pColVal = boost::get<SQLBIGINT>(var);
		}
		else if (SQL_C_DOUBLE == m_bufferType)
		{
			SQLDOUBLE* pColVal = GetDoublePtr();
			*pColVal = boost::get<SQLDOUBLE>(var);
		}
		else if (SQL_C_TYPE_DATE == m_bufferType)
		{
			SQL_DATE_STRUCT* pColVal = GetDatePtr();
			*pColVal = boost::get<SQL_DATE_STRUCT>(var);
		}
		else if (SQL_C_TYPE_TIME == m_bufferType)
		{
			SQL_TIME_STRUCT* pColVal = GetTimePtr();
			*pColVal = boost::get<SQL_TIME_STRUCT>(var);
		}
		else if (SQL_C_TYPE_TIMESTAMP  == m_bufferType)
		{
			SQL_TIMESTAMP_STRUCT* pColVal = GetTimestampPtr();
			*pColVal = boost::get<SQL_TIMESTAMP_STRUCT>(var);
		}
		else if (SQL_C_NUMERIC == m_bufferType)
		{
			SQL_NUMERIC_STRUCT* pColVal = GetNumericPtr();
			*pColVal = boost::get<SQL_NUMERIC_STRUCT>(var);
		}
#if HAVE_MSODBCSQL_H
		else if (SQL_C_SS_TIME2 == m_bufferType)
		{
			SQL_SS_TIME2_STRUCT* pColVal = GetTime2Ptr();
			*pColVal = boost::get<SQL_SS_TIME2_STRUCT>(var);
		}
#endif
		else
		{
			LOG_ERROR((boost::wformat(L"Not implemented Sql C Type '%s' (%d)") % SqLCType2s(m_bufferType) % m_bufferType).str());
			exASSERT(false);
		}
	}


	ColumnBuffer::operator SQLSMALLINT() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(IsBound());

		// We use the BigIntVisitor here. It will always succeed to convert if 
		// the underlying Value is an int-value or throw otherwise
		SQLBIGINT bigVal = boost::apply_visitor(BigintVisitor(), m_bufferPtr);
		
		// But we are only allowed to Downcast this to a Smallint if the original value was a smallint
		if ( m_bufferType != SQL_C_SSHORT)
		{
			throw CastException(m_bufferType, SQL_C_SSHORT);
		}
		return (SQLSMALLINT)bigVal;
	}


	ColumnBuffer::operator SQLINTEGER() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(IsBound());

		// We use the BigIntVisitor here. It will always succeed to convert if 
		// the underlying Value is an int-value or throw otherwise
		SQLBIGINT bigVal = boost::apply_visitor(BigintVisitor(), m_bufferPtr);
		// But we are only allowed to downcast this to an Int if we are not loosing information
		if (!( m_bufferType == SQL_C_SSHORT || m_bufferType == SQL_C_SLONG))
		{
			// TODO: Fix IT
			throw CastException(m_bufferType, SQL_C_SLONG);
		}
		return (SQLINTEGER)bigVal;
	}


	ColumnBuffer::operator SQLBIGINT() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(IsBound());

		return boost::apply_visitor(BigintVisitor(), m_bufferPtr);
	}


	ColumnBuffer::operator std::wstring() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(IsBound());

		return boost::apply_visitor(WStringVisitor(), m_bufferPtr);
	}


	ColumnBuffer::operator std::string() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(IsBound());

		return boost::apply_visitor(StringVisitor(), m_bufferPtr);
	}


	ColumnBuffer::operator SQLDOUBLE() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(IsBound());

		return boost::apply_visitor(DoubleVisitor(), m_bufferPtr);
	}


	ColumnBuffer::operator SQL_DATE_STRUCT() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(IsBound());

		const SQL_TIMESTAMP_STRUCT& timeStamp = boost::apply_visitor(TimestampVisitor(), m_bufferPtr);

		SQL_DATE_STRUCT date;
		date.day = timeStamp.day;
		date.month = timeStamp.month;
		date.year = timeStamp.year;
		return date;
	}


	ColumnBuffer::operator SQL_TIME_STRUCT() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(IsBound());

		const SQL_TIMESTAMP_STRUCT& timeStamp = boost::apply_visitor(TimestampVisitor(), m_bufferPtr);

		SQL_TIME_STRUCT time;
		time.hour = timeStamp.hour;
		time.minute = timeStamp.minute;
		time.second = timeStamp.second;
		return time;
	}


	ColumnBuffer::operator SQL_SS_TIME2_STRUCT() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(IsBound());

		const SQL_TIMESTAMP_STRUCT& timeStamp = boost::apply_visitor(TimestampVisitor(), m_bufferPtr);

		SQL_SS_TIME2_STRUCT time;
		time.hour = timeStamp.hour;
		time.minute = timeStamp.minute;
		time.second = timeStamp.second;
		time.fraction = timeStamp.fraction;
		return time;
	}


	ColumnBuffer::operator SQL_TIMESTAMP_STRUCT() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(IsBound());

		return boost::apply_visitor(TimestampVisitor(), m_bufferPtr);
	}


	ColumnBuffer::operator SQL_NUMERIC_STRUCT() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(IsBound());

		return boost::apply_visitor(NumericVisitor(), m_bufferPtr);
	}


	ColumnBuffer::operator const SQLCHAR*() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(IsBound());

		return boost::apply_visitor(CharPtrVisitor(), m_bufferPtr);
	}


	SQLSMALLINT* ColumnBuffer::GetSmallIntPtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLSMALLINT*>(m_bufferPtr);
	}

	SQLINTEGER* ColumnBuffer::GetIntPtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLINTEGER*>(m_bufferPtr);
	}

	SQLBIGINT* ColumnBuffer::GetBigIntPtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLBIGINT*>(m_bufferPtr);
	}

	SQLCHAR* ColumnBuffer::GetCharPtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLCHAR*>(m_bufferPtr);
	}

	SQLWCHAR* ColumnBuffer::GetWCharPtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLWCHAR*>(m_bufferPtr);
	}

	SQLDOUBLE* ColumnBuffer::GetDoublePtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLDOUBLE*>(m_bufferPtr);
	}

	SQL_DATE_STRUCT* ColumnBuffer::GetDatePtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQL_DATE_STRUCT*>(m_bufferPtr);
	}


	SQL_TIME_STRUCT* ColumnBuffer::GetTimePtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQL_TIME_STRUCT*>(m_bufferPtr);
	}


	SQL_TIMESTAMP_STRUCT* ColumnBuffer::GetTimestampPtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQL_TIMESTAMP_STRUCT*>(m_bufferPtr);
	}


#if HAVE_MSODBCSQL_H
	SQL_SS_TIME2_STRUCT* ColumnBuffer::GetTime2Ptr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQL_SS_TIME2_STRUCT*>(m_bufferPtr);
	}
#endif


	SQL_NUMERIC_STRUCT* ColumnBuffer::GetNumericPtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQL_NUMERIC_STRUCT*>(m_bufferPtr);
	}


	SQL_TIMESTAMP_STRUCT TimestampVisitor::operator()(SQL_TIME_STRUCT* pTime) const
	{
		SQL_TIMESTAMP_STRUCT timestamp; 
		ZeroMemory(&timestamp, sizeof(timestamp));
		timestamp.hour = pTime->hour; 
		timestamp.minute = pTime->minute;
		timestamp.second = pTime->second;
		return timestamp;
	}


	SQL_TIMESTAMP_STRUCT TimestampVisitor::operator()(SQL_DATE_STRUCT* pDate) const 
	{ 
		SQL_TIMESTAMP_STRUCT timestamp; 
		ZeroMemory(&timestamp, sizeof(timestamp)); 
		timestamp.day = pDate->day; 
		timestamp.month = pDate->month;
		timestamp.year = pDate->year;
		return timestamp; 
	};


	SQL_TIMESTAMP_STRUCT TimestampVisitor::operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const
	{
		SQL_TIMESTAMP_STRUCT timestamp = *pTimestamp;
		return timestamp;
	}


#if HAVE_MSODBCSQL_H
	SQL_TIMESTAMP_STRUCT TimestampVisitor::operator()(SQL_SS_TIME2_STRUCT* pTime) const 
	{ 
		SQL_TIMESTAMP_STRUCT timestamp; 
		ZeroMemory(&timestamp, sizeof(timestamp)); 
		timestamp.hour = pTime->hour; 
		timestamp.minute = pTime->minute; 
		timestamp.second = pTime->second; 
		timestamp.fraction = pTime->fraction;
		//ColumnBuffer::TrimValue(m_decimalDigits, timestamp.fraction);
		return timestamp; 
	};
#endif


	// Interfaces
	// ----------

}
