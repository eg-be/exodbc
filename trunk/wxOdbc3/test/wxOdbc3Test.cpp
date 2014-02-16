// CoreTest.cpp : Defines the exported functions for the DLL application.
//

#include "wxOdbc3.h"

#include "db.h"

#include <iostream>

#include "cppunit/plugin/TestPlugIn.h"
CPPUNIT_PLUGIN_IMPLEMENT();

void PrintErrors(wxDb* pDb, const wxString& file, int line, const wxString& function)
{
	std::vector<wxString> errors = pDb->GetErrorList();
	bool first = false;
	for(size_t i = 0; i < errors.size(); i++)
	{
		if(errors[i].Len() > 0)
		{
			if(!first)
			{
				std::wcout << L"\n" << file << L"(" << line << L"):\n\t" << function << L"\n";
				first = true;
			}
			std::wcout << errors[i] << L"\n";
		}
	}
}