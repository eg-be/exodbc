/*!
* \file ShortIntro.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 03.04.2016
* \copyright GNU Lesser General Public License Version 3
*
* ShortIntro Sample.
*/

#include <SDKDDKVer.h>
#include <iostream>

#include "exodbc/exOdbc.h"
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/Table.h"
#include "exodbc/ExecutableStatement.h"

int main()
{
	using namespace exodbc;

	try
	{
		// Parse arguments

		// Create an environment with ODBC Version 3.0
		EnvironmentPtr pEnv = Environment::Create(OdbcVersion::V_3);

		// And connect to a database using the environment.
		DatabasePtr pDb = Database::Create(pEnv);
		pDb->Open(L"Driver={SQL Server Native Client 11.0};Server=192.168.56.20\\EXODBC;Database=exodbc;Uid=ex;Pwd=extest;");


	}
	catch (const Exception& ex)
	{
		std::wcerr << ex.ToString() << std::endl;
	}
    return 0;
}

