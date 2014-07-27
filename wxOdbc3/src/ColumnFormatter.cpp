/*!
* \file ColumnFormatter.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 26.07.2014
* 
* [Brief CPP-file description]
*/ 

//#include "stdafx.h"

// Own header
#include "ColumnFormatter.h"

// Same component headers
// Other headers
#include "boost/format.hpp"

namespace exodbc
{
	// Static consts
	// -------------

	// Construction
	// -------------
	/********** wxDbColFor Constructor **********/
	ColumnFormatter::ColumnFormatter()
	{
		Initialize();
	}  // wxDbColFor::wxDbColFor()
	
	// Destructor
	// -----------

	// Implementation
	// --------------


	/********** wxDbColFor::Initialize() **********/
	void ColumnFormatter::Initialize()
	{
		s_Field.empty();
		int i;
		for (i=0; i<7; i++)
		{
			s_Format[i].empty();
			s_Amount[i].empty();
			i_Amount[i] = 0;
		}
		i_Nation      = 0;                     // 0=EU, 1=UK, 2=International, 3=US
		i_dbDataType  = 0;
		i_sqlDataType = 0;
		Format(1,DB_DATA_TYPE_VARCHAR,0,0,0);  // the Function that does the work
	}  // wxDbColFor::Initialize()


	/********** wxDbColFor::Format() **********/
	int ColumnFormatter::Format(int Nation, int dbDataType, SWORD sqlDataType,
		short columnLength, short decimalDigits)
	{
		// ----------------------------------------------------------------------------------------
		// -- 19991224 : mj10777 : Create
		// There is still a lot of work to do here, but it is a start
		// It handles all the basic data-types that I have run into up to now
		// The main work will have be with Dates and float Formatting
		//    (US 1,000.00 ; EU 1.000,00)
		// There are wxWindow plans for locale support and the new wxDateTime.  If
		//    they define some constants (wxEUROPEAN) that can be gloably used,
		//    they should be used here.
		// ----------------------------------------------------------------------------------------
		// There should also be a function to scan in a string to fill the variable
		// ----------------------------------------------------------------------------------------
		std::wstring tempStr;
		i_Nation      = Nation;                                       // 0 = timestamp , 1=EU, 2=UK, 3=International, 4=US
		i_dbDataType  = dbDataType;
		i_sqlDataType = sqlDataType;
		s_Field = (boost::wformat(L"%s%d") % s_Amount[1] % i_Amount[1]).str(); // OK for VARCHAR, INTEGER and FLOAT

		if (i_dbDataType == 0)                                        // Filter unsupported dbDataTypes
		{
			if ((i_sqlDataType == SQL_VARCHAR)
#if defined(SQL_WCHAR)
				|| (i_sqlDataType == SQL_WCHAR)
#endif
#if defined(SQL_WVARCHAR)
				|| (i_sqlDataType == SQL_WVARCHAR)
#endif
				|| (i_sqlDataType == SQL_LONGVARCHAR))
				i_dbDataType = DB_DATA_TYPE_VARCHAR;
			if ((i_sqlDataType == SQL_C_DATE) || (i_sqlDataType == SQL_C_TIMESTAMP))
				i_dbDataType = DB_DATA_TYPE_DATE;
			if (i_sqlDataType == SQL_C_BIT)
				i_dbDataType = DB_DATA_TYPE_INTEGER;
			if (i_sqlDataType == SQL_NUMERIC)
				i_dbDataType = DB_DATA_TYPE_VARCHAR;   // glt - ??? is this right?
			if (i_sqlDataType == SQL_REAL)
				i_dbDataType = DB_DATA_TYPE_FLOAT;
			if (i_sqlDataType == SQL_C_BINARY)
				i_dbDataType = DB_DATA_TYPE_BLOB;
		}

		if ((i_dbDataType == DB_DATA_TYPE_INTEGER) && (i_sqlDataType == SQL_C_DOUBLE))
		{   // DBASE Numeric
			i_dbDataType = DB_DATA_TYPE_FLOAT;
		}

		switch(i_dbDataType)     // TBD: Still a lot of proper formatting to do
		{
		case DB_DATA_TYPE_VARCHAR:
			s_Field = L"%s";
			break;
		case DB_DATA_TYPE_INTEGER:
			s_Field = L"%d";
			break;
		case DB_DATA_TYPE_FLOAT:
			if (decimalDigits == 0)
				decimalDigits = 2;
			tempStr = (boost::wformat(L"%%%d.%d") % columnLength % decimalDigits).str();
			s_Field = (boost::wformat(L"%sf") % tempStr).str();
			break;
		case DB_DATA_TYPE_DATE:
			if (i_Nation == 0)      // timestamp       YYYY-MM-DD HH:MM:SS.SSS (tested for SYBASE)
			{
				s_Field = L"%04d-%02d-%02d %02d:%02d:%02d.%03d";
			}
			if (i_Nation == 1)      // European        DD.MM.YYYY HH:MM:SS.SSS
			{
				s_Field = L"%02d.%02d.%04d %02d:%02d:%02d.%03d";
			}
			if (i_Nation == 2)      // UK              DD/MM/YYYY HH:MM:SS.SSS
			{
				s_Field = L"%02d/%02d/%04d %02d:%02d:%02d.%03d";
			}
			if (i_Nation == 3)      // International   YYYY-MM-DD HH:MM:SS.SSS
			{
				s_Field = L"%04d-%02d-%02d %02d:%02d:%02d.%03d";
			}
			if (i_Nation == 4)      // US              MM/DD/YYYY HH:MM:SS.SSS
			{
				s_Field = L"%02d/%02d/%04d %02d:%02d:%02d.%03d";
			}
			break;
		case DB_DATA_TYPE_BLOB:
			s_Field = (boost::wformat(L"Unable to format(%d)-SQL(%d)") % dbDataType % sqlDataType).str();
			break;
		default:
			s_Field = (boost::wformat(L"Unknown Format(%d)-SQL(%d)") % dbDataType % sqlDataType).str();
			break;
		};
		return TRUE;
	}  // wxDbColFor::Format()

	// Interfaces
	// ----------

}