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
	* \brief An e
	*/
	class InputGenerator
	{
	public:
		virtual std::string GetNextSqlCommand() = 0;
	private:
	};
	typedef std::shared_ptr<InputGenerator> InputGeneratorPtr;

	class StdInGenerator
		: public InputGenerator
	{
	public:
		std::string GetNextSqlCommand() { return u8""; };
	};
}
