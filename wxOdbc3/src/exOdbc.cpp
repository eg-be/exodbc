/*!
* \file wxOdbc3.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 23.07.2014
* 
* [Brief CPP-file description]
*/ 

#include "stdafx.h"

// Own header
#include "exOdbc.h"

// Same component headers
#include "Helpers.h"

// Other headers

namespace exodbc {

	// Static consts
	// -------------
	const wchar_t* emptyString				= L"";
	const wchar_t* SQL_LOG_FILENAME         = L"sqllog.txt";
	const wchar_t* SQL_CATALOG_FILENAME     = L"catalog.txt";

	// Implementation
	// --------------
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
}