/*!
* \file AssertionException.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.01.2016
* \brief Source file for the AssertionException and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "AssertionException.h"

// Same component headers
#include "LogManager.h"

// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	// Construction
	// ------------

	// Destructor
	// -----------

	// Implementation
	// --------------
	void exOnAssert(const std::string& file, int line, const std::string& function, const std::string& condition, const std::string& msg)
	{
		std::stringstream ss;
		ss << u8"ASSERTION failure!" << std::endl;
		ss << u8" File:      " << file << std::endl;
		ss << u8" Line:      " << line << std::endl;
		ss << u8" Function:  " << function << std::endl;
		ss << u8" Condition: " << condition << std::endl;
		if (msg.length() > 0)
		{
			ss << u8" Msg:       " << msg << std::endl;
		}
		LOG_ERROR(ss.str());

		// Throw exception
		throw AssertionException(line, file, function, condition, msg);
	}


	std::string AssertionException::ToString() const noexcept
	{
		std::stringstream ss;
		ss << Exception::ToString();
		ss << u8"\n";
		ss << u8"\tCondition: " << m_condition;
		return ss.str();
	}
}
