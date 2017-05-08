/*!
 * \file TestParams.cpp
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright GNU Lesser General Public License Version 3
 * 
 * [Brief CPP-file description]
 */ 

// Own header
#include "TestParams.h"

// Same component headers
// Other headers
#include "exodbc/Exception.h"
#include "exodbc/LogManager.h"
#include "exodbc/LogHandler.h"
#include "exodbc/SpecializedExceptions.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"
#include "boost/iostreams/stream.hpp"

// Debug
#include "DebugNew.h"

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------

using namespace std;
using namespace exodbc;

namespace exodbctest
{
	::std::ostream& operator<<(::std::ostream& os, const TestParams& oi) {
		std::stringstream ss;
		if (oi.HasConnectionString())
		{
			ss << u8"Connection String: " << oi.m_connectionString;
		}
		else
		{
			ss << u8"DSN: " << oi.m_dsn
				<< u8"; Username: " << oi.m_username
				<< u8"; Password: " << oi.m_password;
		}
		ss << u8"; Names: " << (oi.m_namesCase == Case::LOWER ? u8"lowercase" : u8"uppercase");

		return os << ss.str();
	}


	::std::wostream& operator<<(::std::wostream& wos, const TestParams& oi) {
		std::stringstream ss;
		if (oi.HasConnectionString())
		{
			ss << u8"Connection String: " << oi.m_connectionString;
		}
		else
		{
			ss << u8"DSN: " << oi.m_dsn
				<< u8"; Username: " << oi.m_username
				<< u8"; Password: " << oi.m_password;
		}
		ss << u8"; Names: " << (oi.m_namesCase == Case::LOWER ? u8"lowercase" : u8"uppercase");
		std::wstring ws = utf8ToUtf16(ss.str());
		return wos << ws;
	}


	Case ReadCase(const boost::property_tree::ptree& subTree)
	{
		namespace pt = boost::property_tree;
		namespace ba = boost::algorithm;

		string namesCase = subTree.get<string>(u8"Case");
		if (ba::iequals(namesCase, u8"u") || ba::iequals(namesCase, u8"upper"))
		{
			return Case::UPPER;
		}
		else if (ba::iequals(namesCase, u8"l") || ba::iequals(namesCase, u8"lower"))
		{
			return Case::LOWER;
		}
		else
		{
			stringstream ss;
			ss << u8"TestSettings.Case must be either 'upper', 'u', 'lower' or 'l', but it is " << namesCase;
			IllegalArgumentException ae(ss.str());
			SET_EXCEPTION_SOURCE(ae);
			throw ae;
		}
	}


	std::vector<string> ReadSkipNames(const boost::property_tree::ptree& subTree)
	{
		namespace pt = boost::property_tree;

		std::vector<string> skipNames;
		for (const pt::ptree::value_type &c : subTree.get_child(u8""))
		{
			string elementName = c.first.data();
			if (elementName == u8"Skip")
			{
				pt::ptree child = c.second;
				skipNames.push_back(child.get<string>(u8""));
			}
		}
		return skipNames;
	}


	void TestParams::Load(const boost::filesystem::path& settingsFile, std::vector<string>& skipNames)
	{
		namespace pt = boost::property_tree;
		namespace io = boost::iostreams;
		namespace fs = boost::filesystem;
		namespace ba = boost::algorithm;

		try
		{
			pt::ptree tree;

			// Parse the XML into the property tree.
			pt::read_xml(settingsFile.string(), tree);

			for (const pt::ptree::value_type &v : tree.get_child(u8"TestSettings"))
			{
				string elementName = v.first.data();
				pt::ptree subTree = v.second;
				if (elementName == u8"Dsn")
				{
					bool disabled = subTree.get<bool>(u8"Disabled");
					if (!disabled)
					{
						m_dsn = subTree.get<string>(u8"Name");
						m_username = subTree.get<string>(u8"User");
						m_password = subTree.get<string>(u8"Pass");
						m_namesCase = ReadCase(subTree);
						skipNames = ReadSkipNames(subTree);
						break;
					}
				}
				if (elementName == u8"ConnectionString")
				{
					bool disabled = subTree.get<bool>(u8"Disabled");
					if (!disabled)
					{
						m_connectionString = subTree.get<string>(u8"Value");
						m_namesCase = ReadCase(subTree);
						skipNames = ReadSkipNames(subTree);
						break;
					}
				}
			}

			if (!IsUsable())
			{
				LOG_WARNING(u8"DSN and ConnectionString are both disabled");
			}

			// create db
			m_createDb = tree.get<bool>(u8"TestSettings.CreateDb", false);
			
			// loglevel
			string logLevelValue = tree.get<string>(u8"TestSettings.LogLevel", u8"Info");
			if (boost::iequals(logLevelValue, u8"Debug"))
			{
				LogManager::Get().SetGlobalLogLevel(LogLevel::Debug);
			}
			else if (boost::iequals(logLevelValue, u8"Info"))
			{
				LogManager::Get().SetGlobalLogLevel(LogLevel::Info);
			}
			else if (boost::iequals(logLevelValue, u8"Warning"))
			{
				LogManager::Get().SetGlobalLogLevel(LogLevel::Warning);
			}
			else if (boost::iequals(logLevelValue, u8"Error"))
			{
				LogManager::Get().SetGlobalLogLevel(LogLevel::Error);
			}
			// logfile
			bool logFile = tree.get<bool>(u8"TestSettings.LogFile", false);
			if (logFile)
			{
				FileLogHandlerPtr pFileLogger = std::make_shared<FileLogHandler>(u8"exodbctest.log", true);
				LogManager::Get().RegisterLogHandler(pFileLogger);
			}
		}
		catch (const std::exception& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw we;
		}
	}
	
} // namespace exodbctest