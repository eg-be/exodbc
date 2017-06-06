/*!
* \file SqlInfoProperty.h
* \author Elias Gerber <eg@elisium.ch>
* \date 12.04.2017
* \brief Header file for the SqlInfo class.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "SqlHandle.h"

// Other headers
#include <boost/variant.hpp>

// System headers
#include <map>
#include <string>
#include <set>

// Forward declarations
// --------------------

namespace exodbc
{
	// Consts
	// ------

	// Structs
	// -------

	// Classes
	// -------

	/*!
	* \class SqlInfoProperty
	* \brief Holds a value read from SqlGetInfo().
	* \details When being constructed, the value is initialized 
	* with a default value. The Default value is 'N' for SqlInfoProperty::ValueType::String_N_Y,
	* an empty String for SqlInfoProperty::ValueType::String_Any or 0 for other value types.
	*/	
	class EXODBCAPI SqlInfoProperty
	{
	public:
		enum class InfoType
		{
			Driver,
			DBMS,
			DataSource,
			SupportedSql,
			SqlLimits,
			ScalarFunction,
			Conversion
		};

		enum class ValueType
		{
			USmallInt,
			UInt,
			String_N_Y,
			String_Any
		};

		typedef boost::variant<
			SQLUSMALLINT,
			SQLUINTEGER,
			std::string
		> Value;

		SqlInfoProperty(SQLUSMALLINT infoId, const std::string& InfoName, InfoType infoType, ValueType valueType);

		ValueType GetValueType() const noexcept { return m_valueType; };
		InfoType GetInfoType() const noexcept { return m_infoType; };
		Value GetValue() const noexcept;
		SQLUSMALLINT GetInfoId() const noexcept { return m_infoId; };
		std::string GetName() const noexcept { return m_infoName; };
		bool GetValueRead() const noexcept { return m_valueRead; };

		void ReadProperty(ConstSqlDbcHandlePtr pHdbc);
		std::string GetStringValue() const;

		bool GetIsUnsupported() const noexcept { return m_unsupported; };
		void SetUnsupported(bool isUnsupported) noexcept { m_unsupported = isUnsupported; };

	private:
		/*!
		* \brief	A wrapper to SQLGetInfo to read a String info.
		*
		* \param	pHDbc					The Database connection handle.
		* \param	fInfoType				Type of the information.
		* \param [in,out]	sValue			String to receive the value read.
		* \details This will first call SQLGetInfo to determine the size of the buffer, then allocate a
		*			corresponding buffer and call GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER pInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue)
		* \see		SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER pInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue)
		* \throw	Exception
		*/
		void ReadInfoValue(ConstSqlDbcHandlePtr pHDbc, SQLUSMALLINT fInfoType, std::string& sValue) const;

		/*!
		* \brief	A wrapper to SQLGetInfo.
		*
		* \param	pHDbc					The Database connection handle.
		* \param	fInfoType				Type of the information.
		* \param	pInfoValue			Output buffer pointer.
		* \param	cbInfoValueMax			Length of buffer.
		* \param [in,out]	pcbInfoValue	Out-pointer for total length in bytes (excluding null-terminate char for string-values).
		* \see		http://msdn.microsoft.com/en-us/library/ms711681%28v=vs.85%29.aspx
		* \throw	Exception
		*/
		void ReadInfoValue(ConstSqlDbcHandlePtr pHDbc, SQLUSMALLINT fInfoType, SQLPOINTER rgbInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue) const;

		SQLUSMALLINT m_infoId;
		std::string m_infoName;
		InfoType m_infoType;
		ValueType m_valueType;
		Value m_value;
		bool m_valueRead;
		bool m_unsupported;
	};


	/*!
	* \class SqlInfoPropertyStringValueVisitor
	* \brief Get the value of a SqlInfoProperty as std::string.
	*/
	class EXODBCAPI SqlInfoPropertyStringValueVisitor
		: public boost::static_visitor<std::string>
	{
	public:
		std::string operator()(SQLUSMALLINT ui) const;
		std::string operator()(SQLUINTEGER ui) const;
		std::string operator()(const std::string& ui) const;
	};


	class EXODBCAPI SqlInfoProperties
	{
	public:
		/*!
		* \brief Create a new instance. Call Init() later to read all registered values.
		*/
		SqlInfoProperties()
			: m_dbms(DatabaseProduct::UNKNOWN)
		{};

		/*!
		* \brief Creates a new instance and calls Init on that instance, using the
		*		passed odbcVersion.
		* \see Init(OdbcVersion odbcVersion, ConstSqlDbcHandlePtr pHdbc)
		*/
		SqlInfoProperties(ConstSqlDbcHandlePtr pHdbc, OdbcVersion odbcVersion);


		/*!
		* \brief Creates a new instance and calls Init on that instance.
		* \see Init(ConstSqlDbcHandlePtr pHdbc)
		*/
		SqlInfoProperties(ConstSqlDbcHandlePtr pHdbc);


		/*!
		* \brief Registers all properties that are available within the passed odbcVersion
		*		and tries to read their values from the passed Connection handle.
		*		If reading a value fails, a warning is logged and the default value is set.
		*/
		void Init(ConstSqlDbcHandlePtr pHdbc, OdbcVersion odbcVersion);


		/*!
		* \brief Queries the driver about its odbc version, then 
		*	calls Init(ConstSqlDbcHandlePtr pHdbc, OdbcVersion odbcVersion)
		* \see Init(ConstSqlDbcHandlePtr pHdbc, OdbcVersion odbcVersion)
		*/
		void Init(ConstSqlDbcHandlePtr pHdbc);


		/*!
		* \brief Clears all properties read.
		*/
		void Reset() noexcept;


		struct SLexicalCompare {
			bool operator() (const SqlInfoProperty& lhs, const SqlInfoProperty& rhs) const {
				return lhs.GetName() < rhs.GetName();
			}
		};
		typedef std::set<SqlInfoProperty, SLexicalCompare> PropertiesSet;
		PropertiesSet GetSubset(SqlInfoProperty::InfoType infoType) const noexcept;


		/*!
		* \brief	Return property by id. Property must have been registered prior.
		* \throw	Exception
		*/
		SqlInfoProperty GetProperty(SQLUSMALLINT infoId) const;


		/*!
		* \brief Returns true if a SqlInfoProperty with passed infoId is registered.
		*/
		bool IsPropertyRegistered(SQLUSMALLINT infoId) const noexcept;


		/*!
		* \brief Reads a single property from the passed connection handle, if 
		*		property has not already been read.
		*		Property must be registered.
		*		If forceUpdate is set to true, property will be read guaranteed.
		* \throw NotFoundException if passed info id is not registered.
		*/
		void EnsurePropertyRead(ConstSqlDbcHandlePtr pHdbc, SQLUSMALLINT infoId, bool forceUpdate);


		/*!
		* \brief Reads all registered properties from the passed connection handle.
		*		Does not throw but logs a warning if reading a property fails.
		*/
		void ReadAllProperties(ConstSqlDbcHandlePtr pHdbc);


		/*!
		* \brief Get the number of registered properties.
		*/
		size_t GetPropertyCount() const noexcept { return m_props.size(); };


		/*!
		* \brief Parses the string value of property SQL_DRIVER_ODBC_VER into
		*		a value of OdbcVersion. Returns OdbcVersion::UNKNOWN if the string value is not in the form
		*		##.## or if parsing the values to numeric values fail, or if unknown values are encountered.
		*/
		OdbcVersion GetDriverOdbcVersion() const;


		/*!
		* \brief Return the value of property SQL_DBMS_NAME
		*/
		std::string GetDbmsName() const;


		/*!
		* \brief Return the value of property SQL_DRIVER_NAME
		*/
		std::string GetDriverName() const;

		
		/*!
		* \brief Checks if value of property SQL_TXN_CAPABLE is not set to SQL_TC_NONE.
		*/
		bool GetSupportsTransactions() const;


		/*!
		* \brief Returns true if property SQL_CATALOG_NAME is set (from ODBC 3.x on) and its value is set to 'Y'.
		*/
		bool GetSupportsCatalogs() const;


		/*!
		* \brief Returns the value of property SQL_SCHEMA_TERM. Empty value might indicate no support for schemas.
		*/
		std::string GetSchemaTerm() const;


		/*!
		* \brief Returns the value of property SQL_CATALOG_TERM. Empty values indicates no support for catalogs.
		*/
		std::string GetCatalogTerm() const;


		/*!
		* \brief If property value SQL_MAX_TABLE_NAME_LEN is > 0, the value is returned, else DB_MAX_TABLE_NAME_LEN_DEFAULT
		*/
		SQLUSMALLINT GetMaxTableNameLen() const;


		/*!
		* \brief If property value SQL_MAX_CATALOG_NAME_LEN is > 0, the value is returned, else DB_MAX_CATALOG_NAME_LEN_DEFAULT
		*/

		SQLUSMALLINT GetMaxCatalogNameLen() const;


		/*!
		* \brief If property value SQL_MAX_SCHEMA_NAME_LEN is > 0, the value is returned, else DB_MAX_SCHEMA_NAME_LEN_DEFAULT
		*/
		SQLUSMALLINT GetMaxSchemaNameLen() const;


		/*!
		* \brief If property value SQL_MAX_COLUMN_NAME_LEN is > 0, the value is returned, else DB_MAX_COLUMN_NAME_LEN_DEFAULT
		*/
		SQLUSMALLINT GetMaxColumnNameLen() const;


		/*
		* \brief Return value of property SQL_SEARCH_PATTERN_ESCAPE.
		*/
		std::string GetSearchPatternEscape() const;


		/*!
		* \brief Returns true if SQL_SO_FORWARD_ONLY is set in the bitmask value of property SQL_SCROLL_OPTIONS.
		*/
		bool GetForwardOnlyCursors() const;


		/*!
		* \brief Tries to extract a matching OdbcVersion from a string in the form '##.##'.
		*		Returns OdbcVersion::UNKOWN if parsing fails.
		*/
		static OdbcVersion ParseOdbcVersion(const std::string& versionString);


		/*!
		* \brief Returns DatabaseProduct value parsed during Init().
		* \see DetectDbms()
		*/
		DatabaseProduct GetDbms() const noexcept { return m_dbms; };


	private:
		/*!
		* \brief Parses the string value of property SQL_DBMS_NAME  and tries to match it against a known DatabaseProduct.
		*		If Property SQL_DBMS_NAME  has not yet been read, it is read first.
		*		Returns DatabaseProduct::UNKNOWN string does not match any known DatabaseProduct.
		* \throw SqlResultException If reading property fails.
		*/
		DatabaseProduct DetectDbms(ConstSqlDbcHandlePtr pHdbc);

		void RegisterDriverProperties(OdbcVersion odbcVersion);
		void RegisterDbmsProperties(OdbcVersion odbcVersion);
		void RegisterDataSourceProperties(OdbcVersion odbcVersion);
		void RegisterSupportedSqlProperties(OdbcVersion odbcVersion);
		void RegisterSqlLimitsProperties(OdbcVersion odbcVersion);
		void RegisterScalerFunctionProperties(OdbcVersion odbcVersion);
		void RegisterConversionProperties(OdbcVersion odbcVersion);

		void RegisterProperty(SQLUSMALLINT id, const std::string& name, SqlInfoProperty::InfoType infoType, SqlInfoProperty::ValueType valueType);

		void MarkAsUnsupported(SQLUSMALLINT infoId);

		typedef std::map<SQLUSMALLINT, SqlInfoProperty> PropsMap;
		PropsMap m_props;
		DatabaseProduct m_dbms;	///< Remember parsed value of dbms.
	};
} // namespace exodbc
