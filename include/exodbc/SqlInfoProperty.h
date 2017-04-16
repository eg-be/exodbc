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
		Value GetValue() const noexcept { return m_value; };
		SQLUSMALLINT GetInfoId() const noexcept { return m_infoId; };
		std::string GetName() const noexcept { return m_infoName; };
		bool GetValueRead() const noexcept { return m_valueRead; };

		void ReadProperty(ConstSqlDbcHandlePtr pHdbc);
		std::string GetStringValue() const;

	private:
		void ReadInfoValue(ConstSqlDbcHandlePtr pHDbc, SQLUSMALLINT fInfoType, std::string& sValue) const;
		void ReadInfoValue(ConstSqlDbcHandlePtr pHDbc, SQLUSMALLINT fInfoType, SQLPOINTER rgbInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue) const;

		SQLUSMALLINT m_infoId;
		std::string m_infoName;
		InfoType m_infoType;
		ValueType m_valueType;
		Value m_value;
		bool m_valueRead;
	};


	class EXODBCAPI SqlInfoPropertyNameVisitor
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
		SqlInfoProperties()
		{};

		SqlInfoProperties(ConstSqlDbcHandlePtr pHdbc, bool readAllProperties);

		void Init(ConstSqlDbcHandlePtr pHdbc, bool readAllProperties);

		struct SLexicalCompare {
			bool operator() (const SqlInfoProperty& lhs, const SqlInfoProperty& rhs) const {
				return lhs.GetName() < rhs.GetName();
			}
		};
		std::set<SqlInfoProperty, SLexicalCompare> GetProperties(SqlInfoProperty::InfoType infoType) const noexcept;

		SqlInfoProperty GetProperty(SQLUSMALLINT infoId);

	private:
		void ReadAllProperties();
		void RegisterDriverInformation();
		void RegisterDbmsProperties();
		void RegisterDataSourceProperties();

		void RegisterProperty(SQLUSMALLINT id, const std::string& name, SqlInfoProperty::InfoType infoType, SqlInfoProperty::ValueType valueType);

		typedef std::map<SQLUSMALLINT, SqlInfoProperty> PropsMap;
		PropsMap m_props;
		ConstSqlDbcHandlePtr m_pHdbc;
	};
} // namespace exodbc
