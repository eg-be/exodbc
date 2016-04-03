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
#include "AssertionException.h"

// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	LogManager g_logManager;

	LogManager::LogManager()
#ifdef _DEBUG
		: m_globalLogLevel(LogLevel::Debug)
#else
		: m_globalLogLevel(LogLevel::Info)
#endif
	{
		LogHandlerPtr pDefaultHandler = std::make_shared<StdErrLogHandler>();
		RegisterLogHandler(pDefaultHandler);
	}


	void LogManager::ClearLogHandlers() noexcept
	{
		lock_guard<mutex> lock(m_logHandlersMutex);
		m_logHandlers.clear();
	}


	void LogManager::RegisterLogHandler(LogHandlerPtr pHandler)
	{
		exASSERT(pHandler);
		
		lock_guard<mutex> lock(m_logHandlersMutex);
		// Only register if not already registerd
		for (auto it = m_logHandlers.begin(); it != m_logHandlers.end(); ++it)
		{
			LogHandlerPtr pLogHanlder = *it;
			if (*pLogHanlder == *pHandler)
			{
				return;
			}
		}
		m_logHandlers.push_back(pHandler);
	}


	void LogManager::LogMessage(LogLevel level, const std::wstring& msg, const std::wstring& file /* = L"" */, int line /* = 0 */, const std::wstring& functionName /* = L"" */) const
	{
		{
			lock_guard<mutex> lock(m_globalLogLevelMutex);
			if (level < m_globalLogLevel)
			{
				return;
			}
		}
		lock_guard<mutex> lock(m_logHandlersMutex);
		for (auto it = m_logHandlers.begin(); it != m_logHandlers.end(); ++it)
		{
			(*it)->OnLogMessage(level, msg, file, line, functionName);
		}
	}


	size_t LogManager::GetRegisteredLogHandlersCount() const noexcept
	{
		lock_guard<mutex> lock(m_logHandlersMutex);
		return m_logHandlers.size();
	}


	std::vector<LogHandlerPtr> LogManager::GetLogHandlers() const noexcept
	{
		lock_guard<mutex> lock(m_logHandlersMutex);
		return m_logHandlers;
	}


	void LogManager::SetGlobalLogLevel(LogLevel level) noexcept
	{
		lock_guard<mutex> lock(m_globalLogLevelMutex);
		m_globalLogLevel = level;
	}


	LogLevel LogManager::GetGlobalLogLevel() const noexcept
	{
		lock_guard<mutex> lock(m_globalLogLevelMutex);
		return m_globalLogLevel;
	}
}
