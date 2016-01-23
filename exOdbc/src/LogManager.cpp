/*!
* \file LogManager.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.01.2016
* \brief Source file for name objects.
* \copyright GNU Lesser General Public License Version 3
*
*/

#include "stdafx.h"

// Own header
#include "LogManager.h"

// Same component headers
#include "LogHandler.h"

// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	LogManager::LogManager()
	{
		LogHandlerPtr pDefaultHandler = std::make_shared<StdErrLogHandler>();
		RegisterLogHandler(pDefaultHandler);
	}


	void LogManager::ClearLogHandlers()
	{
		lock_guard<mutex> lock(m_logHandlersMutex);
		m_logHandlers.clear();
	}


	void LogManager::RegisterLogHandler(LogHandlerPtr pHandler)
	{
		exASSERT(pHandler);
		
		lock_guard<mutex> lock(m_logHandlersMutex);
		m_logHandlers.push_back(pHandler);
	}


	void LogManager::LogMessage(LogLevel level, const std::wstring& msg, const std::wstring& file /* = L"" */, int line /* = 0 */, const std::wstring& functionName /* = L"" */) const
	{
		lock_guard<mutex> lock(m_logHandlersMutex);
		for (auto it = m_logHandlers.begin(); it != m_logHandlers.end(); ++it)
		{
			(*it)->LogMessage(level, msg, file, line, functionName);
		}
	}
}
