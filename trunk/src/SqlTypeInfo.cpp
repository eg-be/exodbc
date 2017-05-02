/*!
* \file SqlTypeInfo.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 01.05.2017
* \brief Source file for TableInfo.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "SqlTypeInfo.h"

// Same component headers
#include "AssertionException.h"
#include "Helpers.h"

// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	SqlTypeInfo::SqlTypeInfo()
	: m_sqlType(0)
	, m_columnSize(0)
	, m_columnSizeIsNull(true)
	, m_literalPrefixIsNull(true)
	, m_literalSuffixIsNull(true)
	, m_createParamsIsNull(true)
	, m_nullable(SQL_NULLABLE_UNKNOWN)
	, m_caseSensitive(SQL_FALSE)
	, m_searchable(SQL_PRED_NONE)
	, m_unsigned(SQL_FALSE)
	, m_unsignedIsNull(true)
	, m_fixedPrecisionScale(SQL_FALSE)
	, m_autoUniqueValue(SQL_FALSE)
	, m_autoUniqueValueIsNull(true)
	, m_localTypeNameIsNull(true)
	, m_minimumScale(0)
	, m_minimumScaleIsNull(true)
	, m_maximumScale(0)
	, m_maximumScaleIsNull(true)
	, m_sqlDataType(0)
	, m_sqlDateTimeSub(0)
	, m_sqlDateTimeSubIsNull(true)
	, m_numPrecRadix(0)
	, m_numPrecRadixIsNull(true)
	, m_intervalPrecision(0)
	, m_intervalPrecisionIsNull(true)
	{ }


	SqlTypeInfo::SqlTypeInfo(ConstSqlStmtHandlePtr pStmt, const SqlInfoProperties& props)
	{
		exASSERT(pStmt);
		exASSERT(pStmt->IsAllocated());

		SQLLEN cb = 0;

		GetData(pStmt, 1, DB_MAX_TYPE_NAME_LEN, m_typeName);
		GetData(pStmt, 2, SQL_C_SSHORT, &m_sqlType, sizeof(m_sqlType), &cb, nullptr);
		GetData(pStmt, 3, SQL_C_SLONG, &m_columnSize, sizeof(m_columnSize), &cb, &m_columnSizeIsNull);
		GetData(pStmt, 4, DB_MAX_LITERAL_PREFIX_LEN, m_literalPrefix, &m_literalPrefixIsNull);
		GetData(pStmt, 5, DB_MAX_LITERAL_SUFFIX_LEN, m_literalSuffix, &m_literalSuffixIsNull);
		GetData(pStmt, 6, DB_MAX_CREATE_PARAMS_LIST_LEN, m_createParams, &m_createParamsIsNull);
		GetData(pStmt, 7, SQL_C_SSHORT, &m_nullable, sizeof(m_nullable), &cb, nullptr);
		GetData(pStmt, 8, SQL_C_SSHORT, &m_caseSensitive, sizeof(m_caseSensitive), &cb, nullptr);
		GetData(pStmt, 9, SQL_C_SSHORT, &m_searchable, sizeof(m_searchable), &cb, nullptr);
		GetData(pStmt, 10, SQL_C_SSHORT, &m_unsigned, sizeof(m_unsigned), &cb, &m_unsignedIsNull);
		GetData(pStmt, 11, SQL_C_SSHORT, &m_fixedPrecisionScale, sizeof(m_fixedPrecisionScale), &cb, nullptr);
		GetData(pStmt, 12, SQL_C_SSHORT, &m_autoUniqueValue, sizeof(m_autoUniqueValue), &cb, &m_autoUniqueValueIsNull);
		GetData(pStmt, 13, DB_MAX_LOCAL_TYPE_NAME_LEN, m_localTypeName, &m_localTypeNameIsNull);
		GetData(pStmt, 14, SQL_C_SSHORT, &m_minimumScale, sizeof(m_minimumScale), &cb, &m_minimumScaleIsNull);
		GetData(pStmt, 15, SQL_C_SSHORT, &m_maximumScale, sizeof(m_maximumScale), &cb, &m_maximumScaleIsNull);
		GetData(pStmt, 16, SQL_C_SSHORT, &m_sqlDataType, sizeof(m_sqlDataType), &cb, nullptr);
		GetData(pStmt, 17, SQL_C_SSHORT, &m_sqlDateTimeSub, sizeof(m_sqlDateTimeSub), &cb, &m_sqlDateTimeSubIsNull);
		GetData(pStmt, 18, SQL_C_SSHORT, &m_numPrecRadix, sizeof(m_numPrecRadix), &cb, &m_numPrecRadixIsNull);
		GetData(pStmt, 19, SQL_C_SSHORT, &m_intervalPrecision, sizeof(m_intervalPrecision), &cb, &m_intervalPrecisionIsNull);
	}


	std::string SqlTypeInfo::ToOneLineStrForTrac(bool withHeaderLine /* = false */) const
	{
		std::stringstream ss;
		if (withHeaderLine)
		{
			ss << (boost::format(u8"||= %25s=||= %25s=||= %34s =||= %45s =||= %5s =||= %10s =||= %8s =||= %6s =||= %6s =||= %10s =||= %10s =||= %10s =||= %5s =||= %5s =||= %5s =||= %5s =||= %5s =||= %5s =||= %34s =||") % u8"SQLType" %u8"SQL Data Type (3)" %u8"!TypeName" %u8"Local !TypeName" %u8"Unsigned" %u8"Precision" %u8"Nullable" %u8"Auto Inc." %u8"Case Sens." %u8"Searchable" %u8"Prefix" %u8"Suffix" %u8"Fixed Prec. Scale" %u8"Min. Scale" %u8"Max. Scale" %u8"Sql DateTimeSub" %u8"Num. Prec. Radix" %u8"Interval Precision" %u8"Create Params").str() << std::endl;
		}

		std::string sFSqlType = (boost::format(u8"%18s (%4d)") % SqlType2s(m_sqlType) % m_sqlType).str();
		std::string sSqlDataType = (boost::format(u8"%18s (%4d)") % SqlType2s(m_sqlDataType) % m_sqlDataType).str();
		std::string sPrecision = (m_columnSizeIsNull ? u8"NULL" : (boost::format(u8"%d") % m_columnSize).str());
		std::string sLiteralPrefix = m_literalPrefixIsNull ? u8"NULL" : m_literalPrefix;
		std::string sLiteralSuffix = m_literalSuffixIsNull ? u8"NULL" : m_literalSuffix;
		std::string sCreateParams = m_createParamsIsNull ? u8"NULL" : m_createParams;
		std::string sNullable = u8"???";
		std::string sCaseSensitive = SqlTrueFalse2s(m_caseSensitive);
		std::string sSearchable = u8"???";
		std::string sUnsigned = m_unsignedIsNull ? u8"NULL" : SqlTrueFalse2s(m_unsigned);
		std::string sFixedPrecisionScale = SqlTrueFalse2s(m_fixedPrecisionScale);
		std::string sAutoUniqueValue = m_autoUniqueValueIsNull ? u8"NULL" : SqlTrueFalse2s(m_autoUniqueValue);
		std::string sLocalTypeName = m_localTypeNameIsNull ? u8"NULL" : m_localTypeName;
		std::string sMinimumScale = m_minimumScaleIsNull ? u8"NULL" : (boost::format(u8"%d") % m_minimumScale).str();
		std::string sMaximumScale = m_maximumScaleIsNull ? u8"NULL" : (boost::format(u8"%d") % m_maximumScale).str();
		std::string sSqlDateTimeSub = m_sqlDateTimeSubIsNull ? u8"NULL" : (boost::format(u8"%d") % m_sqlDateTimeSub).str();
		std::string sNumPrecRadix = m_numPrecRadixIsNull ? u8"NULL" : (boost::format(u8"%d") % m_numPrecRadix).str();
		std::string sIntervalPrecision = m_intervalPrecisionIsNull ? u8"NULL" : (boost::format(u8"%d") % m_intervalPrecision).str();

		switch (m_nullable)
		{
		case SQL_NO_NULLS:
			sNullable = u8"NO_NULLS";
			break;
		case SQL_NULLABLE:
			sNullable = u8"NULLABLE";
			break;
		default:
			sNullable = u8"UNKNOWN";
		}
		switch (m_searchable)
		{
		case SQL_PRED_NONE:
			sSearchable = u8"PRED_NONE";
			break;
		case SQL_PRED_CHAR:
			sSearchable = u8"PRED_CHAR";
			break;
		case SQL_PRED_BASIC:
			sSearchable = u8"PRED_BASIC";
			break;
		case SQL_SEARCHABLE:
			sSearchable = u8"SEARCHABLE";
			break;
		}

		std::string s = (boost::format(u8"|| %25s|| %2s|| %34s || %45s || %5s || %10s || %8s || %6s || %6s || %10s || %10s || %10s || %5s || %5s || %5s || %5s || %5s || %5s || %34s ||") % sFSqlType %sSqlDataType %m_typeName %sLocalTypeName %sUnsigned %sPrecision %sNullable %sAutoUniqueValue %sCaseSensitive %sSearchable %sLiteralPrefix %sLiteralSuffix %sFixedPrecisionScale %sMinimumScale %sMaximumScale %sSqlDateTimeSub %sNumPrecRadix %sIntervalPrecision %sCreateParams).str();

		ss << s;

		return ss.str();
	}


	std::string SqlTypeInfo::ToOneLineStr(bool withHeaderLines /* = false */, bool withEndLine /* = false */) const
	{
		std::stringstream ss;
		if (withHeaderLines)
		{
			ss << (boost::format(u8"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------")).str() << std::endl;
			ss << (boost::format(u8"| %25s | %25s | %34s | %45s | %5s | %10s | %8s | %6s | %6s | %10s | %10s | %10s | %5s | %5s | %5s | %5s | %5s | %5s | %34s |") % u8"SQLType" %u8"SQL Data Type (3)" %u8"TypeName" %u8"Local TypeName" %u8"Unsig" %u8"Precision" %u8"Nullable" %u8"AutoI" %u8"CaseS." %u8"Searchable" %u8"Prefix" %u8"Suffix" %u8"FixPS" %u8"MinSc" %u8"MaxSc" %u8"DTS3" %u8"NuPR" %u8"IntPr" %u8"Create Params").str() << std::endl;
			ss << (boost::format(u8"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------")).str() << std::endl;
		}

		std::string sFSqlType = (boost::format(u8"%18s (%4d)") % SqlType2s(m_sqlType) % m_sqlType).str();
		std::string sSqlDataType = (boost::format(u8"%18s (%4d)") % SqlType2s(m_sqlDataType) % m_sqlDataType).str();
		std::string sPrecision = (m_columnSizeIsNull ? u8"NULL" : (boost::format(u8"%d") % m_columnSize).str());
		std::string sLiteralPrefix = m_literalPrefixIsNull ? u8"NULL" : m_literalPrefix;
		std::string sLiteralSuffix = m_literalSuffixIsNull ? u8"NULL" : m_literalSuffix;
		std::string sCreateParams = m_createParamsIsNull ? u8"NULL" : m_createParams;
		std::string sNullable = u8"???";
		std::string sCaseSensitive = SqlTrueFalse2s(m_caseSensitive);
		std::string sSearchable = u8"???";
		std::string sUnsigned = m_unsignedIsNull ? u8"NULL" : SqlTrueFalse2s(m_unsigned);
		std::string sFixedPrecisionScale = SqlTrueFalse2s(m_fixedPrecisionScale);
		std::string sAutoUniqueValue = m_autoUniqueValueIsNull ? u8"NULL" : SqlTrueFalse2s(m_autoUniqueValue);
		std::string sLocalTypeName = m_localTypeNameIsNull ? u8"NULL" : m_localTypeName;
		std::string sMinimumScale = m_minimumScaleIsNull ? u8"NULL" : (boost::format(u8"%d") % m_minimumScale).str();
		std::string sMaximumScale = m_maximumScaleIsNull ? u8"NULL" : (boost::format(u8"%d") % m_maximumScale).str();
		std::string sSqlDateTimeSub = m_sqlDateTimeSubIsNull ? u8"NULL" : (boost::format(u8"%d") % m_sqlDateTimeSub).str();
		std::string sNumPrecRadix = m_numPrecRadixIsNull ? u8"NULL" : (boost::format(u8"%d") % m_numPrecRadix).str();
		std::string sIntervalPrecision = m_intervalPrecisionIsNull ? u8"NULL" : (boost::format(u8"%d") % m_intervalPrecision).str();

		switch (m_nullable)
		{
		case SQL_NO_NULLS:
			sNullable = u8"NO_NULLS";
			break;
		case SQL_NULLABLE:
			sNullable = u8"NULLABLE";
			break;
		default:
			sNullable = u8"UNKNOWN";
		}
		switch (m_searchable)
		{
		case SQL_PRED_NONE:
			sSearchable = u8"PRED_NONE";
			break;
		case SQL_PRED_CHAR:
			sSearchable = u8"PRED_CHAR";
			break;
		case SQL_PRED_BASIC:
			sSearchable = u8"PRED_BASIC";
			break;
		case SQL_SEARCHABLE:
			sSearchable = u8"SEARCHABLE";
			break;
		}

		std::string s = (boost::format(u8"| %25s | %2s | %34s | %45s | %5s | %10s | %8s | %6s | %6s | %10s | %10s | %10s | %5s | %5s | %5s | %5s | %5s | %5s | %34s |") % sFSqlType %sSqlDataType %m_typeName %sLocalTypeName %sUnsigned %sPrecision %sNullable %sAutoUniqueValue %sCaseSensitive %sSearchable %sLiteralPrefix %sLiteralSuffix %sFixedPrecisionScale %sMinimumScale %sMinimumScale %sSqlDateTimeSub %sNumPrecRadix %sIntervalPrecision %sCreateParams).str();

		ss << s;

		if (withEndLine)
		{
			ss << std::endl;
			ss << (boost::format(u8"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------")).str() << std::endl;
		}

		return ss.str();
	}

	std::string SqlTypeInfo::ToStr() const
	{
		std::stringstream ss;
		std::string sNull = u8"NULL";

		ss << u8"***** DATA TYPE INFORMATION *****" << std::endl;
		ss << u8" Name:                   " << m_typeName << std::endl;
		ss << u8"  SQL Type:              " << m_sqlType << std::endl;
		ss << u8"  Precision:             " << (m_columnSizeIsNull ? u8"NULL" : (boost::format(u8"%d") % m_columnSize).str()) << std::endl;
		ss << u8"  Literal Prefix:        " << (m_literalPrefixIsNull ? u8"NULL" : m_literalPrefix) << std::endl;
		ss << u8"  Literal Suffix:        " << (m_literalSuffixIsNull ? u8"NULL" : m_literalSuffix) << std::endl;
		ss << u8"  Create Params:         " << (m_createParamsIsNull ? u8"NULL" : m_createParams) << std::endl;

		ss << u8"  Nullable:              ";
		switch (m_nullable)
		{
		case SQL_NO_NULLS:
			ss << u8"SQL_NO_NULL";
			break;
		case SQL_NULLABLE:
			ss << u8"SQL_NULLABLE";
			break;
		case SQL_NULLABLE_UNKNOWN:
			ss << u8"SQL_NULLABLE_UNKNOWN";
			break;
		default:
			ss << u8"???";
			break;
		}
		ss << std::endl;

		ss << u8"  Case Sensitive:        " << (m_caseSensitive == SQL_TRUE ? u8"SQL_TRUE" : u8"SQL_FALSE") << std::endl;
		ss << u8"  Searchable:            ";
		switch (m_searchable)
		{
		case SQL_PRED_NONE:
			ss << u8"SQL_PRED_NONE";
			break;
		case SQL_PRED_CHAR:
			ss << u8"SQL_PRED_CHAR";
			break;
		case SQL_PRED_BASIC:
			ss << u8"SQL_PRED_BASIC";
			break;
		case SQL_SEARCHABLE:
			ss << u8"SQL_SEARCHABLE";
			break;
		default:
			ss << u8"???";
			break;
		}
		ss << std::endl;

		ss << u8"  Unsigned:              " << (m_unsignedIsNull ? u8"NULL" : (m_unsigned == SQL_TRUE ? u8"SQL_TRUE" : u8"SQL_FALSE")) << std::endl;
		ss << u8"  Fixed Precision Scale: " << (m_fixedPrecisionScale == SQL_TRUE ? u8"SQL_TRUE" : u8"SQL_FALSE") << std::endl;
		ss << u8"  Auto Unique Value:     " << (m_autoUniqueValueIsNull ? u8"NULL" : (m_autoUniqueValue == SQL_TRUE ? u8"SQL_TRUE" : u8"SQL_FALSE")) << std::endl;
		ss << u8"  Local Type Name:       " << (m_localTypeNameIsNull ? u8"NULL" : m_localTypeName) << std::endl;
		ss << u8"  Minimum Scale:         " << (m_minimumScaleIsNull ? u8"NULL" : (boost::format(u8"%d") % m_minimumScale).str()) << std::endl;
		ss << u8"  Maximum Scale:         " << (m_maximumScaleIsNull ? u8"NULL" : (boost::format(u8"%d") % m_maximumScale).str()) << std::endl;
		ss << u8"  SQL Data type:         " << m_sqlDataType << std::endl;
		ss << u8"  SQL Date Time Sub:     " << (m_sqlDateTimeSubIsNull ? u8"NULL" : (boost::format(u8"%d") % m_sqlDateTimeSub).str()) << std::endl;
		ss << u8"  Num Prec Radix:        " << (m_numPrecRadixIsNull ? u8"NULL" : (boost::format(u8"%d") % m_numPrecRadix).str()) << std::endl;
		ss << u8"  Interval Precision:    " << (m_intervalPrecisionIsNull ? u8"NULL" : (boost::format(u8"%d") % m_intervalPrecision).str()) << std::endl;
		return ss.str();
	}


	bool SqlTypeInfo::operator<(const SqlTypeInfo& other) const
	{
		return m_sqlDataType < other.m_sqlDataType;
	}
}
