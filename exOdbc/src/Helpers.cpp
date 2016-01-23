/*!
* \file Helpers.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.07.2014
* \brief Source file for the Helpers
* \copyright GNU Lesser General Public License Version 3
*/ 

#include "stdafx.h"

// Own header
#include "Helpers.h"

// Same component headers
#include "Exception.h"

// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------
namespace exodbc
{

	// Construction
	// -------------

	// Destructor
	// -----------

	// Implementation
	// --------------
	void GetInfo(ConstSqlDbcHandlePtr pHDbc, SQLUSMALLINT fInfoType, std::wstring& sValue)
	{
		// Determine buffer length
		exASSERT(pHDbc);
		exASSERT(pHDbc->IsAllocated());
		SQLSMALLINT bufferSize = 0;
		SQLRETURN ret = SQLGetInfo(pHDbc->GetHandle(), fInfoType, NULL, NULL, &bufferSize);
		{
			// \note: DB2 will here always return SQL_SUCCESS_WITH_INFO to report that data got truncated, although we didnt even feed in a buffer.
			// To avoid having tons of warning with the (wrong) info that data has been truncated, we just hide those messages here
			THROW_IFN_SUCCEEDED_SILENT_MSG(SQLGetInfo, ret, SQL_HANDLE_DBC, pHDbc->GetHandle(), (boost::wformat(L"GetInfo for fInfoType %d failed") % fInfoType).str());
		}

		// According to the doc SQLGetInfo will always return byte-size. Therefore:
		exASSERT((bufferSize % sizeof(SQLWCHAR)) == 0);

		// Allocate buffer, add one for terminating 0 char.
		SQLSMALLINT charSize = (bufferSize / sizeof(SQLWCHAR)) + 1;
		bufferSize = charSize * sizeof(SQLWCHAR);
		std::unique_ptr<SQLWCHAR[]> buff(new SQLWCHAR[charSize]);
		buff[0] = 0;
		SQLSMALLINT cb;

		GetInfo(pHDbc, fInfoType, (SQLPOINTER)buff.get(), bufferSize, &cb);

		sValue = buff.get();
	}


	void GetInfo(ConstSqlDbcHandlePtr pHDbc, SQLUSMALLINT fInfoType, SQLPOINTER rgbInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue)
	{
		exASSERT(pHDbc);
		exASSERT(pHDbc->IsAllocated());
		SQLRETURN ret = SQLGetInfo(pHDbc->GetHandle(), fInfoType, rgbInfoValue, cbInfoValueMax, pcbInfoValue);
		THROW_IFN_SUCCEEDED_MSG(SQLGetInfo, ret, SQL_HANDLE_DBC, pHDbc->GetHandle(), (boost::wformat(L"GetInfo for fInfoType %d failed") % fInfoType).str());
	}


	void GetData(ConstSqlStmtHandlePtr pHStmt, SQLUSMALLINT colOrParamNr, SQLSMALLINT targetType, SQLPOINTER pTargetValue, SQLLEN bufferLen, SQLLEN* strLenOrIndPtr, bool* pIsNull)
	{
		exASSERT(pHStmt);
		exASSERT(pHStmt->IsAllocated());
		exASSERT(strLenOrIndPtr != NULL);

		bool isNull;
		SQLRETURN ret = SQLGetData(pHStmt->GetHandle(), colOrParamNr, targetType, pTargetValue, bufferLen, strLenOrIndPtr);
		THROW_IFN_SUCCEEDED_MSG(SQLGetData, ret, SQL_HANDLE_STMT, pHStmt->GetHandle(), (boost::wformat(L"SGLGetData failed for Column %d") % colOrParamNr).str());

		isNull = (*strLenOrIndPtr == SQL_NULL_DATA);
		if (pIsNull)
		{
			*pIsNull = isNull;
		}
	}


	void GetData(ConstSqlStmtHandlePtr pHStmt, SQLUSMALLINT colOrParamNr, size_t maxNrOfChars, std::wstring& value, bool* pIsNull /* = NULL */)
	{
		value = L"";
		std::unique_ptr<SQLWCHAR[]> buffer(new SQLWCHAR[maxNrOfChars + 1]);
		size_t buffSize = sizeof(SQLWCHAR) * (maxNrOfChars + 1);
		SQLLEN cb;
		bool isNull = false;

		GetData(pHStmt, colOrParamNr, SQL_C_WCHAR, buffer.get(), buffSize, &cb, &isNull);

		if(!isNull)
		{
			value = buffer.get();
		}
		if(pIsNull)
			*pIsNull = isNull;
	}


	void SetDescriptionField(SQLHDESC hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value)
	{
		exASSERT(hDesc != SQL_NULL_HDESC);
		exASSERT(recordNumber > 0);
		SQLRETURN ret = SQLSetDescField(hDesc, recordNumber, descriptionField, value, 0);
		THROW_IFN_SUCCEEDED(SQLSetDescField, ret, SQL_HANDLE_DESC, hDesc);
	}


	void SetDescriptionField(ConstSqlDescHandlePtr pHDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value)
	{
		exASSERT(pHDesc);
		exASSERT(pHDesc->IsAllocated());
		SetDescriptionField(pHDesc->GetHandle(), recordNumber, descriptionField, value);
	}


	void SetDescriptionField(const SqlDescHandle& hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value)
	{
		exASSERT(hDesc.IsAllocated());
		SetDescriptionField(hDesc.GetHandle(), recordNumber, descriptionField, value);
	}


	SqlDescHandlePtr GetRowDescriptorHandle(ConstSqlStmtHandlePtr pHStmt, RowDescriptorType type)
	{
		exASSERT(pHStmt);
		exASSERT(pHStmt->IsAllocated());

		auto pHDesc = std::make_shared<SqlDescHandle>(pHStmt, type);
		return pHDesc;
	}


	SQL_TIME_STRUCT InitTime(SQLUSMALLINT hour, SQLUSMALLINT minute, SQLUSMALLINT second) noexcept
	{
		SQL_TIME_STRUCT time;
		time.hour = hour;
		time.minute = minute;
		time.second = second;

		return time;
	}


	SQL_DATE_STRUCT InitDate(SQLUSMALLINT day, SQLUSMALLINT month, SQLSMALLINT year) noexcept
	{
		SQL_DATE_STRUCT date;
		date.year = year;
		date.month = month;
		date.day = day;

		return date;
	}


	SQL_TIMESTAMP_STRUCT InitTimestamp(SQLUSMALLINT hour, SQLUSMALLINT minute, SQLUSMALLINT second, SQLUINTEGER fraction, SQLUSMALLINT day, SQLUSMALLINT month, SQLSMALLINT year) noexcept
	{
		SQL_TIMESTAMP_STRUCT timestamp;
		timestamp.hour = hour;
		timestamp.minute = minute;
		timestamp.second = second;
		timestamp.fraction = fraction;
		timestamp.day = day;
		timestamp.month = month;
		timestamp.year = year;

		return timestamp;
	}


	bool IsTimeEqual(const SQL_TIME_STRUCT& t1, const SQL_TIME_STRUCT& t2) noexcept
	{
		return t1.hour == t2.hour
			&& t1.minute == t2.minute
			&& t1.second == t2.second;
	}


	bool IsDateEqual(const SQL_DATE_STRUCT& d1, const SQL_DATE_STRUCT& d2) noexcept
	{
		return d1.day == d2.day
			&& d1.month == d2.month
			&& d1.year == d2.year;
	}


	bool IsTimestampEqual(const SQL_TIMESTAMP_STRUCT& ts1, const SQL_TIMESTAMP_STRUCT& ts2) noexcept
	{
		return ts1.hour == ts2.hour
			&& ts1.minute == ts2.minute
			&& ts1.second == ts2.second
			&& ts1.fraction == ts2.fraction
			&& ts1.day == ts2.day
			&& ts1.month == ts2.month
			&& ts1.year == ts2.year;
	}


	SQL_NUMERIC_STRUCT InitNumeric(SQLCHAR precision, SQLSCHAR scale, SQLCHAR sign, SQLCHAR val[SQL_MAX_NUMERIC_LEN]) noexcept
	{
		SQL_NUMERIC_STRUCT num;
		num.precision = precision;
		num.scale = scale;
		num.sign = sign;
		memcpy(num.val, val, SQL_MAX_NUMERIC_LEN);
		
		return num;
	}


	SQL_NUMERIC_STRUCT InitNullNumeric() noexcept
	{
		SQL_NUMERIC_STRUCT num;
		ZeroMemory(&num, sizeof(num));
		return num;
	}


	long Str2Hex2Long(unsigned char hexValue[16])
	{
		long val = 0;
		long value = 0;
		int i = 1;
		int last = 1;
		int current;
		int a = 0;
		int b = 0;

		for (i = 0; i <= 15; i++)
		{
			current = (int)hexValue[i];
			a = current % 16; //Obtain LSD
			b = current / 16; //Obtain MSD

			value += last* a;
			last = last * 16;
			value += last* b;
			last = last * 16;
		}
		return value;
	}
}

// Interfaces
// ----------

