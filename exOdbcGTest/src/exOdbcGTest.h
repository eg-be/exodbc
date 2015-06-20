/*!
* \file wxOdbc3GoogleTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 22.07.2014
* \copyright wxWindows Library Licence, Version 3.1
* 
* [Brief Header-file description]
*/ 

#pragma once
#ifndef EXODBCGTEST_H
#define EXODBCGTEST_H

// Same component headers
#include "TestParams.h"

// Other headers
#include "boost/log/trivial.hpp"
#include "boost/log/core.hpp"
#include "boost/log/expressions.hpp"

// System headers
#include <vector>

// Forward declarations
// --------------------

// Globals
// -------
namespace exodbc
{
	extern std::vector<SOdbcInfo> g_odbcInfos;

	extern boost::log::trivial::severity_level g_logSeverity;
	

	// Structs
	// -------

	// Classes
	// -------

	/*
	trace,
	debug,
	info,
	warning,
	error,
	fatal
	*/

	struct LogLevelTrace
	{
		LogLevelTrace() { m_originalLevel = g_logSeverity; g_logSeverity = boost::log::trivial::trace; };

		~LogLevelTrace() { g_logSeverity = m_originalLevel; };

	private:
		boost::log::trivial::severity_level m_originalLevel;
	};


	struct LogLevelDebug
	{
		LogLevelDebug() { m_originalLevel = g_logSeverity; g_logSeverity = boost::log::trivial::debug; };

		~LogLevelDebug() { g_logSeverity = m_originalLevel; };

	private:
		boost::log::trivial::severity_level m_originalLevel;
	};


	struct LogLevelInfo
	{
		LogLevelInfo() { m_originalLevel = g_logSeverity; g_logSeverity = boost::log::trivial::info; };

		~LogLevelInfo() { g_logSeverity = m_originalLevel; };

	private:
		boost::log::trivial::severity_level m_originalLevel;
	};


	struct LogLevelWarning
	{
		LogLevelWarning() { m_originalLevel = g_logSeverity; g_logSeverity = boost::log::trivial::warning; };

		~LogLevelWarning() { g_logSeverity = m_originalLevel; };

	private:
		boost::log::trivial::severity_level m_originalLevel;
	};


	struct LogLevelError
	{
	public:
		LogLevelError() { m_originalLevel = g_logSeverity; g_logSeverity = boost::log::trivial::error; };

		~LogLevelError() { g_logSeverity = m_originalLevel; };
	
	private:
		boost::log::trivial::severity_level m_originalLevel;
	};


	struct LogLevelFatal
	{
		LogLevelFatal() { m_originalLevel = g_logSeverity; g_logSeverity = boost::log::trivial::fatal; };

		~LogLevelFatal() { g_logSeverity = m_originalLevel; };

	private:
		boost::log::trivial::severity_level m_originalLevel;
	};

} // namespace exodbc


#endif // EXODBCGTEST_H
