/*!
* \file InputGenerator.h
* \author Elias Gerber <eg@elisium.ch>
* \date 31.03.2017
* \brief Header file for CommandGenerator
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
// Other headers
// System headers
#include <string>
#include <memory>

// Forward declarations
// --------------------

namespace exodbcexec
{
	// Typedefs
	// --------

	// Structs
	// -------

	// Classes
	// -------

	/*!
	* \class InputGenerator
	* \brief Interface to get the next SQL command to execute.
	*/
	class InputGenerator
	{
	public:
		enum class GetCommandResult
		{
			HAVE_COMMAND,
			NO_COMMAND
		};

		virtual GetCommandResult GetNextCommand(std::string& command) = 0;

	private:
	};
	typedef std::shared_ptr<InputGenerator> InputGeneratorPtr;

	class StdInGenerator
		: public InputGenerator
	{
	public:
		GetCommandResult GetNextCommand(std::string& command);
	};
}
