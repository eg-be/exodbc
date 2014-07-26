/*!
* \file ColumnFormatter.h
* \author Elias Gerber <egerber@gmx.net>
* \date 26.07.2014
* 
* Contains a Formatter class that is UNUSED (?)
*/ 

#pragma once
#ifndef COLUMNFORMATTER_H
#define COLUMNFORMATTER_H

// Same component headers
#include "wxOdbc3.h"

// Other headers
#include <sqltypes.h>

// System headers
#include <string>

namespace exodbc
{


	// Forward declarations
	// --------------------

	// Structs
	// -------

	// Classes
	// -------
	class WXDLLIMPEXP_ODBC ColumnFormatter
	{
	public:
		std::wstring       s_Field;              // Formatted String for Output
		std::wstring       s_Format[7];          // Formatted Objects - TIMESTAMP has the biggest (7)
		std::wstring       s_Amount[7];          // Formatted Objects - amount of things that can be formatted
		int            i_Amount[7];          // Formatted Objects - TT MM YYYY HH MM SS m
		int            i_Nation;             // 0 = timestamp , 1=EU, 2=UK, 3=International, 4=US
		int            i_dbDataType;         // conversion of the 'sqlDataType' to the generic data type used by these classes
		SWORD          i_sqlDataType;

		ColumnFormatter();
		~ColumnFormatter(){}

		void           Initialize();
		int            Format(int Nation, int dbDataType, SWORD sqlDataType, short columnLength, short decimalDigits);
	};
}

#endif // COLUMNFORMATTER_H
