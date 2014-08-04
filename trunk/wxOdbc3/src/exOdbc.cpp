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
}