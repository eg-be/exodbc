/*!
* \file TablePrivileges.h
* \author Elias Gerber <eg@elisium.ch>
* \date 31.12.2014
* \brief Header file for the TablePrivileges class.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "InfoObject.h"
#include "EnumFlags.h"
#include "Database.h"
#include "Exception.h"

// Other headers
// System headers

// Forward declarations
// --------------------
namespace exodbc
{
	class Database;
}

namespace exodbc
{
	// Consts
	// ------

	// Structs
	// -------

	// Classes
	// -------
	enum class TablePrivilege
	{
		NONE = 0x0,

		SELECT = 0x1,
		UPDATE = 0x2,
		INSERT = 0x4,
		DEL = 0x8
	};
	template<>
	struct enable_bitmask_operators<TablePrivilege> {
		static const bool enable = true;
	};


	/*!
	* \class TablePrivileges
	*
	* \brief Parses Table Privileges read from the database and caches them for later use.
	*
	*/
	class TablePrivileges
		: public EnumFlags<TablePrivilege>
	{
	public:

		static std::wstring ToString(TablePrivilege priv);

		/*!
		* \brief	Create an empty TablePrivileges with no TablePrivilege set.
		*/
		TablePrivileges()
			: EnumFlags()
		{};

		/*!
		* \brief	Query database about the Privileges of the passed Table.
		*			Overrides any privileges set with the values read from database.
		* \throw	Exception If querying or parsing fails.
		*/
		void Init(ConstDatabasePtr pDb, const TableInfo& tableInfo);


		/*!
		* \brief	Overrides privileges with passed values.
		* \param	tablePrivs
		* \throw	Exception if Parsing passed data fails.
		*/
		void Init(const TablePrivilegesVector& tablePrivs);
	};


	/*!
	* \class	PrivilegeException
	* \brief	Thrown if Privileges are not okay to do a given operation
	*/
	class EXODBCAPI MissingTablePrivilegeException
		: public Exception
	{
	public:
		MissingTablePrivilegeException() = delete;
		MissingTablePrivilegeException(TablePrivilege missingPriv, const TableInfo& tableInfo)
			: Exception()
			, m_missingPriv(missingPriv)
			, m_tableInfo(tableInfo)
		{
			m_what = w2s(ToString());
		}

		virtual ~MissingTablePrivilegeException() {};

		virtual std::wstring ToString() const throw();

		virtual std::wstring GetName() const noexcept { return L"exodbc::MissingTablePrivilegeException"; };

	protected:
		TablePrivilege m_missingPriv;
		TableInfo m_tableInfo;
	};

} // namespace exodbc
