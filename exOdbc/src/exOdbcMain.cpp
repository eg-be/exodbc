/*!
* \file exOdbcMain.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 18.10.2015
* \brief Source file DLL main entry point
* \copyright GNU Lesser General Public License Version 3
*
*/

#include "stdafx.h"

// Own header
// Same component headers
#include "Exception.h"
#include "IntegerColumnBuffer.h"

// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	void RegisterColumnBuffers()
	{
		try
		{
			ColumnBufferFactory& fact = ColumnBufferFactory::Instance();
			std::set<SQLSMALLINT>::const_iterator it = IntegerColumnBuffer::s_supportedCTypes.begin();
			while (it != IntegerColumnBuffer::s_supportedCTypes.end())
			{
				fact.RegisterColumnBufferCreationFunc(*it, &IntegerColumnBuffer::CreateBuffer);
			}
		}
		catch (const Exception& ex)
		{
			LOG_ERROR(ex.ToString());
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		exodbc::RegisterColumnBuffers();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}



