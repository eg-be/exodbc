/*!
* \file Helpers.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 23.07.2014
* 
* [Brief CPP-file description]
*/ 

#include "stdafx.h"

// Own header
#include "Helpers.h"

// Same component headers
// Other headers

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
	void exOnAssert(const char* file, int line, const char* function, const char* condition, const char* msg)
	{
		std::wstringstream ws;
		ws 	<< L"ASSERTION failure!" << std::endl;
		ws	<< L" File:      " << file << std::endl;
		ws	<< L" Line:      " << line << std::endl;
		ws	<< L" Function:  " << function << std::endl;
		ws	<< L" Condition: " << condition << std::endl;
		if(msg)
		{
			ws << L" Msg:       " << msg << std::endl;
		}
		BOOST_LOG_TRIVIAL(error) << ws.str();
	}


	std::wostream& operator<< (std::wostream &out, const SErrorInfo& ei)
	{
		out << L"SQLSTATE " << ei.SqlState << L"; Native Error: " << ei.NativeError << L"; " << ei.Msg.c_str();
		return out;
	}

	std::ostream& operator<< (std::ostream &out, const SErrorInfo& ei)
	{

		out << "SQLSTATE " << w2s(ei.SqlState) << "; Native Error: " << ei.NativeError << "; " << w2s(ei.Msg);
		return out;
	}

	// Ticket #44
	std::string w2s(const std::wstring& w)
	{
		std::stringstream ss;

		for(size_t i = 0; i < w.length(); i++)
		{
			char c = (char) w[i];
			ss << c;
		}

		return ss.str();
	}

	std::vector<SErrorInfo> GetAllErrors(SQLHANDLE hEnv /* = NULL */, SQLHANDLE hDbc /* = NULL */, SQLHANDLE hStmt /* = NULL */)
	{
		exASSERT(hEnv != NULL || hDbc != NULL || hStmt != NULL);


		std::vector<SErrorInfo> errors;
		SQLHANDLE handle = NULL;
		SQLSMALLINT handleType = NULL;

		for(int i = 0; i < 3; i++)
		{
			if(i == 0 && hEnv)
			{
				handle = hEnv;
				handleType = SQL_HANDLE_ENV;
			}
			else if(i == 1 && hDbc)
			{
				handle = hDbc;
				handleType = SQL_HANDLE_DBC;
			}
			else if(i == 2 && hStmt)
			{
				handle = hStmt;
				handleType = SQL_HANDLE_STMT;
			}
			else
				continue;

			SQLSMALLINT recNr = 1;
			SQLRETURN ret = SQL_SUCCESS;

			while(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
			{
				SErrorInfo errInfo;
				SQLWCHAR errMsg[SQL_MAX_MESSAGE_LENGTH + 1];
				errMsg[0] = 0;
				SQLSMALLINT cb = 0;
				ret = SQLGetDiagRec(handleType, handle, recNr, errInfo.SqlState, &errInfo.NativeError, errMsg, SQL_MAX_MESSAGE_LENGTH + 1, &cb);
				if(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
				{
					errInfo.ErrorHandleType = handleType;
					errInfo.Msg = errMsg;
					errors.push_back(errInfo);
					if(ret == SQL_SUCCESS_WITH_INFO)
						BOOST_LOG_TRIVIAL(warning) << L"Error msg from recNr " << recNr << L" got truncated";
				}
				++recNr;
			}

			if(ret != SQL_NO_DATA)
			{
				BOOST_LOG_TRIVIAL(warning) << L"SQLGetDiagRec did not end with SQL_NO_DATE (100) but with " << ret;
			}
		}

		return errors;
	}


	SErrorInfo GetLastEnvError(SQLHANDLE hEnv, SQLSMALLINT& totalErrors)
	{
		exASSERT(hEnv);

		std::vector<SErrorInfo> errs = GetAllErrors(hEnv);
		totalErrors = errs.size();
		if(totalErrors > 0)
			return errs[0];

		return SErrorInfo();
	}


	SErrorInfo GetLastDbcError(SQLHANDLE hDbc, SQLSMALLINT& totalErrors)
	{
		exASSERT(hDbc);

		std::vector<SErrorInfo> errs = GetAllErrors(NULL, hDbc);
		totalErrors = errs.size();
		if(totalErrors > 0)
			return errs[0];

		return SErrorInfo();
	}


	SErrorInfo GetLastStmtError(SQLHANDLE hStmt, SQLSMALLINT& totalErrors)
	{
		exASSERT(hStmt);

		std::vector<SErrorInfo> errs = GetAllErrors(NULL, NULL, hStmt);
		totalErrors = errs.size();
		if(totalErrors > 0)
			return errs[0];

		return SErrorInfo();
	}


	SErrorInfo GetLastEnvError(SQLHANDLE hEnv)
	{
		exASSERT(hEnv);

		SQLSMALLINT tot = 0;
		return GetLastEnvError(hEnv, tot);
	}

	SErrorInfo GetLastDbcError(SQLHANDLE hDbc)
	{
		exASSERT(hDbc);

		SQLSMALLINT tot = 0;
		return GetLastDbcError(hDbc, tot);
	}


	SErrorInfo GetLastStmtError(SQLHANDLE hStmt)
	{
		exASSERT(hStmt);

		SQLSMALLINT tot = 0;
		return GetLastStmtError(hStmt, tot);
	}

}

// Interfaces
// ----------

