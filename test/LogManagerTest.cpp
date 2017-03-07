/*!
* \file LogManagerTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.01.2016
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

// Own header
#include "LogManagerTest.h"

// Same component headers
// Other headers
#include "exodbc/LogManager.h"
#include "exodbc/LogHandler.h"

// Debug
#include "DebugNew.h"

using namespace exodbc;
using namespace std;

namespace exodbctest
{
	TEST_F(LogManagerTest, Construction)
	{
		// After construction, one StdErrLogger must have been created
		LogManager& mg = LogManager::Get();

		ASSERT_EQ(1, mg.GetRegisteredLogHandlersCount());
		vector<LogHandlerPtr> handlers = mg.GetLogHandlers();

		ASSERT_EQ(1, handlers.size());

		StdErrLogHandlerPtr pStdErrLogger = dynamic_pointer_cast<StdLogHandler>(handlers[0]);
		EXPECT_FALSE(pStdErrLogger == NULL);
	}


	TEST_F(LogManagerTest, ClearLogHandlers)
	{
		LogManager& mg = LogManager::Get();
		EXPECT_EQ(1, mg.GetRegisteredLogHandlersCount());
		mg.ClearLogHandlers();
		EXPECT_EQ(0, mg.GetRegisteredLogHandlersCount());
	}
} // namespace exodbctest