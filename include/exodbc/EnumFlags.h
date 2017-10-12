/*!
* \file EnumFlags.h
* \author Elias Gerber <eg@elisium.ch>
* \date 21.11.2015
* \brief Header file for info objects.
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "bitmask_operators.hpp"

// Other headers

// System headers

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \class EnumFlags
	* \brief Wrapper around a enum class defining flags.
	*/
	template<typename ET, typename std::enable_if<std::is_enum<ET>::value, void>::type* = nullptr>
	class EnumFlags
	{
	public:
		EnumFlags()
		{
			m_flags = static_cast<ET>(0);
		};

		EnumFlags(ET flags)
			: m_flags(flags)
		{};

		virtual ~EnumFlags() 
		{};

		EnumFlags(const EnumFlags& other) = default;
		EnumFlags& operator=(const EnumFlags& other) = default;

		bool Test(ET flag) const noexcept
		{
			return (m_flags & flag) == flag;
		};

		void Set(ET flag) noexcept
		{
			m_flags |= flag;
		};

		void Set(const EnumFlags& other) noexcept
		{
			m_flags = other.m_flags;
		}

		void Clear(ET flag) noexcept
		{
			m_flags &= ~flag;
		};

		bool operator==(const EnumFlags& other) const noexcept
		{
			return m_flags == other.m_flags;
		}

		ET GetFlags() const noexcept
		{
			return m_flags;
		}

	protected:
		ET m_flags;
	};

	/*!
	* \enum ColumnFlag
	* \brief Define flags of a Column.
	*/
	enum class ColumnFlag
	{
		CF_NONE = 0x0,		///< No flags.

		CF_SELECT = 0x1,	///< Include Column in Selects.
		CF_UPDATE = 0x2,	///< Include Column in Updates.
		CF_INSERT = 0x4,	///< Include Column in Inserts.
		CF_NULLABLE = 0x8,	///< Column is null able.
		CF_PRIMARY_KEY = 0x10,	///< Column is primary key.

		CF_READ = CF_SELECT,	///< CF_SELECT
		CF_WRITE = CF_UPDATE | CF_INSERT,	///< CF_UPDATE | CF_INSERT
		CF_READ_WRITE = CF_SELECT | CF_UPDATE | CF_INSERT	///< CF_SELECT | CF_UPDATE | CF_INSERT
	};
	template<>
	struct enable_bitmask_operators<ColumnFlag> {
		static const bool enable = true;
	};

	typedef EnumFlags<ColumnFlag> ColumnFlags;
	typedef std::shared_ptr<ColumnFlags> ColumnFlagsPtr;

	/*!
	* \enum TableAccessFlag
	* \brief Defines how to Access a table using Table functions like Select(), Insert(), Update(), etc.
	*/
	enum class TableAccessFlag
	{
		AF_NONE = 0x0,			///< Do not create any ColumnBuffer for a Table, but still query its TableInfo if requested.

		AF_SELECT_PK = 0x1,		///< Access for SELECting rows identified by bound primary key values.
		AF_SELECT_WHERE = 0x2,	///< Access for SELECTing rows by manually passing a where clause.

		AF_COUNT_WHERE = 0x80,	///< Access for COUNTing number of rows by manually passing a where clause.

		AF_UPDATE_PK = 0x4,		///< Access for UPDATEing where rows to update are identified by the bound primary key values.
		AF_UPDATE_WHERE = 0x8,	///< Access for UPDATEing where rows to update are identified using a manually passed where clause.

		AF_INSERT = 0x10,		///< Access for INSERTing.

		AF_DELETE_PK = 0x20,	///< Access for DELETEing where rows to delete are identified by the bound primary key values.
		AF_DELETE_WHERE = 0x40,	///< Access for DELETEing where rows to delete are identified using a manually passed where clause.

		AF_READ = AF_SELECT_WHERE | AF_SELECT_PK | AF_COUNT_WHERE,	///< All select and count
		AF_READ_WITHOUT_PK = AF_SELECT_WHERE | AF_COUNT_WHERE,	///< Select where no primary keys are required

		AF_WRITE = AF_UPDATE_PK | AF_UPDATE_WHERE | AF_INSERT | AF_DELETE_PK | AF_DELETE_WHERE,	///< All update / insert / delete
		AF_WRITE_WITHOUT_PK = AF_UPDATE_WHERE | AF_INSERT | AF_DELETE_WHERE,	///< Update / Insert / Delete without need for primary keys

		AF_READ_WRITE = AF_READ | AF_WRITE,	///< Everything 
		AF_READ_WRITE_WITHOUT_PK = AF_READ_WITHOUT_PK | AF_WRITE_WITHOUT_PK	///< Everything without need for primary keys
	}; 
	template<>
	struct enable_bitmask_operators<TableAccessFlag> {
		static const bool enable = true;
	};

	typedef EnumFlags<TableAccessFlag> TableAccessFlags;


	/*!
	* \enum TableOpenFlag
	* \brief Defines how to open a table. See Table::Open()
	*/
	enum class TableOpenFlag
	{
		/// No special flags are set.
		TOF_NONE = 0x0,		
		
		/// If set, the Database will always be queried for a matching Table (even if the Table has a valid STableInfo set).		
		TOF_CHECK_EXISTANCE = 0x1,	

		/// If set, columns that are not supported are ignored: <br>
		/// If ColumnBuffers are created automatically, and a table column is encountered for which we do not 
		/// know how to map its SLQ Type against an SQL C Type, this column is skipped. <br>
		/// If Columns have been defined manually, and the flag TOF_CHECK_DB_TYPE_INFOS is set, 
		/// and a column has a SQL Type that is not supported by the Database, it is removed from the defined columns.
		TOF_SKIP_UNSUPPORTED_COLUMNS = 0x4,	
		
		/// If set, primary keys are not queried from the Database during Open().
		TOF_DO_NOT_QUERY_PRIMARY_KEYS = 0x20, 
		
		/// Has only an effect if ColumnBuffers are set manually prior to calling Open(). <br>
		/// If set, it is checked that the SQL Type defined for every ColumnBuffer is supported by the Database.		
		TOF_CHECK_DB_TYPE_INFOS = 0x40, 

		/// If set, forward-only cursors are used only, even if the Database would support Scrollable cursors.
		TOF_FORWARD_ONLY_CURSORS = 0x80, 
	};
	template<>
	struct enable_bitmask_operators<TableOpenFlag> {
		static const bool enable = true;
	};

	typedef EnumFlags<TableOpenFlag> TableOpenFlags;
}
