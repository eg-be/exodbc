/*!
* \file exOdbc.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.07.2014
* \brief Source file for the stuff from exOdbc.h
* \copyright GNU Lesser General Public License Version 3
*/ 

// Own header
#include "exOdbc.h"

// Same component headers
#include "LogManager.h"
#include "SpecializedExceptions.h"
#include "AssertionException.h"
#include "Sql2StringHelper.h"

// Other headers
// Debug
#include "DebugNew.h"

namespace exodbc {

	// Static consts
	// -------------
	const SQLCHAR* ErrorHelper::SQLSTATE_OPTIONAL_FEATURE_NOT_IMPLEMENTED = (const SQLCHAR*)"HYC00";

	// Implementation
	// --------------
	std::string utf16ToUtf8(const std::wstring& w)
	{
		try
		{
			std::wstring_convert<
				std::codecvt_utf8_utf16< std::wstring::value_type >,
				std::wstring::value_type
			> utf16conv;
			return utf16conv.to_bytes(w);
		}
		catch (const std::exception& ex)
		{
			ConversionException ce(ConversionException::Type::UTF16_TO_UTF8, ex.what());
			SET_EXCEPTION_SOURCE(ce);
			throw ce;
		}
	}

	
	std::string utf16ToUtf8(const SQLWCHAR* w)
	{
		std::wstring ws = reinterpret_cast<const wchar_t*>(w);
        return utf16ToUtf8(ws);
    }
    
	
	std::wstring utf8ToUtf16(const std::string& s)
	{
		try
		{
			std::wstring_convert<
				std::codecvt_utf8_utf16< std::wstring::value_type >,
				std::wstring::value_type
			> utf16conv;
			return utf16conv.from_bytes(s);
		}
		catch (const std::exception& ex)
		{
			ConversionException ce(ConversionException::Type::UTF8_TO_UTF16, ex.what());
			SET_EXCEPTION_SOURCE(ce);
			throw ce;
		}
	}

	
	std::wstring utf8ToUtf16(const SQLCHAR* s)
	{
        return utf8ToUtf16(reinterpret_cast<const char*>(s));
    }


	ErrorHelper::SErrorInfoVector ErrorHelper::GetAllErrors(SQLHANDLE hEnv, SQLHANDLE hDbc, SQLHANDLE hStmt, SQLHANDLE hDesc)
	{
		exASSERT(hEnv != SQL_NULL_HENV || hDbc != SQL_NULL_HDBC || hStmt != SQL_NULL_HSTMT || hDesc != SQL_NULL_HDESC);

		SErrorInfoVector errors;
		SQLHANDLE handle = NULL;
		SQLSMALLINT handleType = 0;

		for (int i = 0; i < 4; i++)
		{
			if (i == 0 && hEnv)
			{
				handle = hEnv;
				handleType = SQL_HANDLE_ENV;
			}
			else if (i == 1 && hDbc)
			{
				handle = hDbc;
				handleType = SQL_HANDLE_DBC;
			}
			else if (i == 2 && hStmt)
			{
				handle = hStmt;
				handleType = SQL_HANDLE_STMT;
			}
			else if (i == 3 && hDesc)
			{
				handle = hDesc;
				handleType = SQL_HANDLE_DESC;
			}
			else
				continue;

			SQLSMALLINT recNr = 1;
			SQLRETURN ret = SQL_SUCCESS;

			while (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
			{
				SErrorInfo errInfo;
				SQLAPICHARTYPE errMsg[SQL_MAX_MESSAGE_LENGTH + 1];
                SQLAPICHARTYPE sqlState[5 + 1];
				errMsg[0] = '\0';
                sqlState[0] = '\0';
				SQLSMALLINT cb = 0;
				ret = SQLGetDiagRec(handleType, handle, recNr, sqlState, &errInfo.NativeError, errMsg, SQL_MAX_MESSAGE_LENGTH + 1, &cb);
				if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
				{
                    std::string ssqlState = SQLAPICHARPTR_TO_EXODBCSTR(sqlState);
                    memcpy(errInfo.SqlState, ssqlState.c_str(), std::min<size_t>(ssqlState.length(), 5));
                    errInfo.SqlState[std::min<size_t>(ssqlState.length(), 5)] = '\0';
					errInfo.ErrorHandleType = handleType;
					errInfo.Msg = SQLAPICHARPTR_TO_EXODBCSTR(errMsg);
					errors.push_back(errInfo);
					if (ret == SQL_SUCCESS_WITH_INFO)
					{
						std::stringstream ws;
						ws << u8"Error msg from recNr " << recNr << u8" got truncated";
						LOG_WARNING(ws.str());
					}
				}
				++recNr;
			}

			if (ret != SQL_NO_DATA)
			{
				LOG_WARNING(boost::str(boost::format(u8"Calling SQLGetDiagRec did not end with %s (%d) but with %s (%d)") % Sql2StringHelper::SqlReturn2s(SQL_NO_DATA) % SQL_NO_DATA % Sql2StringHelper::SqlReturn2s(ret) % ret));
			}
		}

		return errors;
	}


	ErrorHelper::SErrorInfoVector ErrorHelper::GetAllErrors(SQLSMALLINT handleType, SQLHANDLE handle)
	{
		switch (handleType)
		{
		case SQL_HANDLE_ENV:
			return GetAllErrors(handle, SQL_NULL_HDBC, SQL_NULL_HSTMT, SQL_NULL_HDESC);
		case SQL_HANDLE_DBC:
			return GetAllErrors(SQL_NULL_HENV, handle, SQL_NULL_HSTMT, SQL_NULL_HDESC);
		case SQL_HANDLE_STMT:
			return GetAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, handle, SQL_NULL_HDESC);
		case SQL_HANDLE_DESC:
			return GetAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, SQL_NULL_HSTMT, handle);
		default:
			exASSERT_MSG(false, u8"Unknown handleType");
		}
		return SErrorInfoVector();
	}


	std::string ErrorHelper::FormatOdbcMessages(SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt, SQLHDESC hDesc, SQLRETURN ret, std::string sqlFunctionName, std::string msg)
	{
		std::string msgStr(msg);
		SErrorInfoVector errs = ErrorHelper::GetAllErrors(hEnv, hDbc, hStmt, hDesc);
		SErrorInfoVector::const_iterator it;
		std::stringstream handles;
		if (hEnv)
			handles << u8"Env=" << hEnv << u8";";
		if (hDbc)
			handles << u8"Dbc=" << hDbc << u8";";
		if (hStmt)
			handles << u8"Stmt=" << hStmt << u8";";
		if (hDesc)
			handles << u8"Desc=" << hDesc << u8";";

		std::string type = u8"Error(s)";
		if (ret == SQL_SUCCESS_WITH_INFO)
		{
			type = u8"Info(s)";
		}
		std::stringstream ss;
		if (msgStr.length() > 0)
			ss << msgStr << u8": ";
		ss << u8"ODBC-Function '" << sqlFunctionName << u8"' returned " << exodbc::Sql2StringHelper::SqlReturn2s(ret) << u8" (" << ret << u8"), with " << errs.size() << u8" ODBC-" << type << u8" from handle(s) '" << handles.str() << u8"': ";
		for (it = errs.begin(); it != errs.end(); it++)
		{
			const SErrorInfo& err = *it;
			ss << std::endl << u8"\t" << err.ToString();
		}
		return ss.str();
	}


	std::string ErrorHelper::SErrorInfo::ToString() const
	{
		std::stringstream ss;
		ss << u8"SQLSTATE " << SqlState << u8"; Native Error: " << NativeError << u8"; " << Msg.c_str();
		return ss.str();
	}
}
