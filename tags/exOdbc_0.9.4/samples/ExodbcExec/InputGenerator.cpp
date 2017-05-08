/*!
* \file InputGenerator.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 31.03.2017
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "InputGenerator.h"

// Same component headers
// Other headers
#include "exodbc/exOdbc.h"
#include <iostream>
#include <string>

// Debug
#include "DebugNew.h"

using namespace std;
using namespace exodbc;

namespace exodbcexec
{
	InputGenerator::GetCommandResult StdInGenerator::GetNextCommand(std::string& command)
	{
		std::wstring line;
		std::getline(std::wcin, line);
		command = utf16ToUtf8(line);
		return GetCommandResult::HAVE_COMMAND;
	}
}