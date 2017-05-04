/*!
* \file GetDataWrapper.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 04.05.2017
* \brief Source file for the GetDataWrapper
* \copyright GNU Lesser General Public License Version 3
*/ 

// Own header
#include "GetDataWrapper.h"

// Same component headers
#include "LogManagerOdbcMacros.h"

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
	void GetDataWrapper::GetData(ConstSqlStmtHandlePtr pHStmt, SQLUSMALLINT colOrParamNr, SQLSMALLINT targetType, SQLPOINTER pTargetValue, SQLLEN bufferLen, SQLLEN* strLenOrIndPtr, bool* pIsNull)
	{
		exASSERT(pHStmt);
		exASSERT(pHStmt->IsAllocated());
		exASSERT(strLenOrIndPtr != NULL);

		bool isNull;
		SQLRETURN ret = SQLGetData(pHStmt->GetHandle(), colOrParamNr, targetType, pTargetValue, bufferLen, strLenOrIndPtr);
		THROW_IFN_SUCCEEDED_MSG(SQLGetData, ret, SQL_HANDLE_STMT, pHStmt->GetHandle(), (boost::format(u8"SGLGetData failed for Column %d") % colOrParamNr).str());

		isNull = (*strLenOrIndPtr == SQL_NULL_DATA);
		if (pIsNull)
		{
			*pIsNull = isNull;
		}
	}


	void GetDataWrapper::GetData(ConstSqlStmtHandlePtr pHStmt, SQLUSMALLINT colOrParamNr, size_t maxNrOfChars, std::string& value, bool* pIsNull /* = NULL */)
	{
		value = u8"";
		std::unique_ptr<SQLAPICHARTYPE[]> buffer(new SQLAPICHARTYPE[maxNrOfChars + 1]);
		size_t buffSize = sizeof(SQLAPICHARTYPE) * (maxNrOfChars + 1);
		SQLLEN cb;
		bool isNull = false;

		GetData(pHStmt, colOrParamNr, SQLAPICHARTYPENAME, buffer.get(), buffSize, &cb, &isNull);

		if(!isNull)
		{
			value = SQLAPICHARPTR_TO_EXODBCSTR(buffer.get());
		}
		if(pIsNull)
        {
            *pIsNull = isNull;
        }
	}
}


