/*!
* \file CreateTracPages.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 07.05.2017
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "CreateTracPages.h"

// Same component headers
#include "ExodbcExec.h"

// Other headers
#include "exodbc/exOdbc.h"
#include "exodbc/LogManager.h"
#include "exodbc/LogHandler.h"
#include "exodbc/Table.h"

// Debug
#include "DebugNew.h"

using namespace std;
using namespace exodbc;

namespace exodbcexec
{
	string CreateTracPages::GetArgumentsSyntax() const noexcept
	{
		return u8"[outfile]";
	}


	vector<string> CreateTracPages::GetAliases() const noexcept {
		switch (m_mode)
		{
		case Mode::DbInfo:
			return{ u8"tracDbInfo", u8"tdbi" };
		case Mode::TestTables:
			return{ u8"tracTestTableInfo", u8"ttti"};
		}
		exASSERT(false);
		return{};
	}



	string CreateTracPages::GetHelp() const noexcept
	{
		switch (m_mode)
		{
		case Mode::DbInfo:
			return u8"Create a Trac page with database and driver information";
		case Mode::TestTables:
			return u8"Create a Trac page with test tables information.";
		}
		exASSERT(false);
		return{};
	}


	void CreateTracPages::Execute(const std::vector<std::string> & args)
	{
		FileLogHandlerPtr pFileLogger = nullptr;

		if (!args.empty())
		{
			LOG_INFO(boost::str(boost::format(u8"Writting all output for Trac to '%s'") % args[0]));
			pFileLogger = std::make_shared<FileLogHandler>(args[0], false);
			pFileLogger->SetShowLogLevel(false);
			pFileLogger->SetShowFileInfo(false);
			pFileLogger->SetLogLevel(LogLevel::Output);
			LogManager::Get().RegisterLogHandler(pFileLogger);
		}

		// a common header
		vector<string> lines;
		vector<string> header = GetHeaderLines();
		lines.insert(lines.end(), header.begin(), header.end());
		vector<string> connectionLines = GetConnectionLines();
		lines.insert(lines.end(), connectionLines.begin(), connectionLines.end());

		// then cmd-specific
		if (m_mode == Mode::DbInfo)
		{
			vector<string> driverInfo = GetDbInfoLines(SqlInfoProperty::InfoType::Driver);
			lines.insert(lines.end(), driverInfo.begin(), driverInfo.end());
			vector<string> dsInfo = GetDbInfoLines(SqlInfoProperty::InfoType::DataSource);
			lines.insert(lines.end(), dsInfo.begin(), dsInfo.end());
			vector<string> dbmsInfo = GetDbInfoLines(SqlInfoProperty::InfoType::DBMS);
			lines.insert(lines.end(), dbmsInfo.begin(), dbmsInfo.end());
			vector<string> limitsInfo = GetDbInfoLines(SqlInfoProperty::InfoType::SqlLimits);
			lines.insert(lines.end(), limitsInfo.begin(), limitsInfo.end());
			vector<string> supportedInfo = GetDbInfoLines(SqlInfoProperty::InfoType::SupportedSql);
			lines.insert(lines.end(), supportedInfo.begin(), supportedInfo.end());
			vector<string> scalarInfo = GetDbInfoLines(SqlInfoProperty::InfoType::ScalarFunction);
			lines.insert(lines.end(), scalarInfo.begin(), scalarInfo.end());
			vector<string> convInfo = GetDbInfoLines(SqlInfoProperty::InfoType::Conversion);
			lines.insert(lines.end(), convInfo.begin(), convInfo.end());

			lines.push_back(u8"== Datatypes Information ==");
			vector<string> typeInfo = GetTypeLines();
			lines.insert(lines.end(), typeInfo.begin(), typeInfo.end());
		}
		else
		{
			vector<string> tableLines = GetTestTableLines();
			lines.insert(lines.end(), tableLines.begin(), tableLines.end());
		}

		for (vector<string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			LOG_OUTPUT(*it);
		}

		if (pFileLogger != nullptr)
		{
			LogManager::Get().RemoveLogHandler(pFileLogger);
		}
	}


	vector<string> CreateTracPages::GetTestTableLines()
	{
		// Try to find tables first:
		const set<string> tableNames = {
			u8"blobtypes",
			u8"blobtypes_tmp",
			u8"chartable",
			u8"chartypes",
			u8"chartypes_tmp",
			u8"datetypes",
			u8"datetypes_tmp",
			u8"floattypes",
			u8"floattypes_tmp",
			u8"integertypes",
			u8"integertypes_tmp",
			u8"multikey",
			u8"numerictypes",
			u8"numerictypes_tmp",
			u8"unicodetable",
			u8"unicodetable_tmp"
		};

		TableInfoVector tables;
		vector<string> lines;
		DatabaseCatalogPtr pDbCat = m_pDb->GetDbCatalog();

		if (m_pDb->GetProperties().GetDbms() == DatabaseProduct::EXCEL)
		{
			tables = pDbCat->SearchTables(u8"%");
		}
		else
		{
			for (set<string>::const_iterator it = tableNames.begin(); it != tableNames.end(); ++it)
			{
				// try to find table with upper or lowercase name:
				TableInfo ti;
				string tableSearchName = boost::algorithm::to_lower_copy(*it);
				try
				{
					ti = pDbCat->FindOneTable(tableSearchName);
				}
				catch (const NotFoundException& nfe)
				{
					HIDE_UNUSED(nfe);
					boost::algorithm::to_upper(tableSearchName);
					try
					{
						ti = pDbCat->FindOneTable(tableSearchName);
					}
					catch (const NotFoundException& nfe)
					{
						HIDE_UNUSED(nfe);
						lines.push_back(boost::str(boost::format(u8"== %s ==") % tableSearchName));
						lines.push_back(boost::str(boost::format(u8"[[span(style=color: #FF0000, **WARNING:**)]] No table was found while searching for a table '%s'/'%s'!")
							% boost::algorithm::to_lower_copy(tableSearchName) % tableSearchName));
						continue;
					}
				}
				catch (const Exception& ex)
				{
					lines.push_back(boost::str(boost::format(u8"== %s ==") % tableSearchName));
					lines.push_back(boost::str(boost::format(u8"[[span(style=color: #FF0000, **ERROR:**)]] Exeption catched while searching for table '%s'/'%s': '%s'!")
						% boost::algorithm::to_lower_copy(tableSearchName) % tableSearchName % ex.ToString()));
					continue;
				}
				tables.push_back(ti);
			}
		}

		for (TableInfoVector::const_iterator it = tables.begin(); it != tables.end(); ++it)
		{
			// try to find table with upper or lowercase name:
			const TableInfo& ti = *it;
			// Table was found, add search name and full name
			lines.push_back(boost::str(boost::format(u8"== %s ==") % ti.GetQueryName()));

			try
			{
				// Add structure of table
				lines.push_back(u8"=== Structure ===");
				vector<string> structureLines = GetTestTableStructureLines(ti);
				lines.insert(lines.end(), structureLines.begin(), structureLines.end());
			}
			catch (const Exception& ex)
			{
				lines.push_back(boost::str(boost::format(u8"[[span(style=color: #FF0000, **ERROR:**)]] Exeption catched while reading structure of '%s': '%s'!")
					% ti.GetQueryName() % ex.ToString()));
			}
			try
			{
				// Only add content if it is not a tmp-table
				if (!boost::algorithm::iends_with(ti.GetQueryName(), u8"_tmp"))
				{
					lines.push_back(u8"=== Content ===");
					vector<string> contentLines = GetTestTableContentLines(ti);
					lines.insert(lines.end(), contentLines.begin(), contentLines.end());
				}
			}
			catch (const Exception& ex)
			{
				lines.push_back(boost::str(boost::format(u8"[[span(style=color: #FF0000, **ERROR:**)]] Exeption catched while reading content of '%s': '%s'!")
					% ti.GetQueryName() % ex.ToString()));
			}
		}

		return lines;
	}


	vector<string> CreateTracPages::GetTestTableStructureLines(const TableInfo& ti)
	{
		boost::format numberFormat(u8"%d");
		vector<string> lines;
		lines.push_back(boost::str(boost::format(u8"||=Column Name =||= SQL Type =||= Column Size=||= Decimal Digits=||")));
		DatabaseCatalogPtr pDbCat = m_pDb->GetDbCatalog();
		ColumnInfoVector cols = pDbCat->ReadColumnInfo(ti);
		for (ColumnInfoVector::const_iterator it = cols.begin(); it != cols.end(); ++it)
		{
			const ColumnInfo& ci = *it;
			lines.push_back(boost::str(boost::format(u8"||%s || %s (%d) || %s|| %s||")
				% ci.GetColumnName()
				% Sql2StringHelper::SqlType2s(ci.GetSqlDataType()) % ci.GetSqlDataType()
				% (ci.IsColumnSizeNull() ? u8"NULL" : boost::str(numberFormat % ci.GetColumnSize()))
				% (ci.IsDecimalDigitsNull() ? u8"NULL" : boost::str(numberFormat % ci.GetDecimalDigits()))
			));
		}
		return lines;
	}


	vector<string> CreateTracPages::GetTestTableContentLines(const exodbc::TableInfo& ti)
	{
		Sql2BufferTypeMapPtr pBufferTypeMap;
#ifdef _WIN32
		pBufferTypeMap = make_shared<WCharSql2BufferMap>();
#else
		pBufferTypeMap = make_shared<CharSql2BufferMap>();
#endif
		vector<string> lines;
		Table tbl(m_pDb, TableAccessFlag::AF_SELECT_WHERE, ti);
		tbl.SetSql2BufferTypeMap(pBufferTypeMap);
		tbl.Open(TableOpenFlag::TOF_NONE);
		set<SQLUSMALLINT> colIndexes = tbl.GetColumnBufferIndexes();
		// Prepare the header
		stringstream ss;
		QueryNameVisitor qnv;
		for (set<SQLUSMALLINT>::const_iterator it = colIndexes.begin(); it != colIndexes.end(); ++it)
		{
			ColumnBufferPtrVariant pColVar = tbl.GetColumnBufferPtrVariant(*it);
			ss << u8"||= " << boost::apply_visitor(qnv, pColVar) << u8" =";
		}
		ss << u8"||";
		lines.push_back(ss.str());
		// MySql somehow does not convert binary columns to strings if we do not ask it to do so.
		// workaround it. See http://stackoverflow.com/questions/43846489/sqlbindcol-binary-column-to-character-buffer-does-not-get-converted-to-string
		if (m_pDb->GetDbms() == DatabaseProduct::MY_SQL && boost::algorithm::istarts_with(ti.GetName(), u8"blobtypes"))
		{
			string sqlstmt = u8"SELECT ";
			set<SQLUSMALLINT>::const_iterator it = colIndexes.begin();
			while(it != colIndexes.end())
			{
				ColumnBufferPtrVariant pColVar = tbl.GetColumnBufferPtrVariant(*it);
				SQLSMALLINT sqlType = boost::apply_visitor(SqlTypeVisitor(), pColVar);
				if (sqlType == SQL_BINARY || sqlType == SQL_VARBINARY)
					sqlstmt += boost::str(boost::format(u8"HEX(%s)") % boost::apply_visitor(qnv, pColVar));
				else
					sqlstmt += boost::apply_visitor(qnv, pColVar);
				++it;
				if (it != colIndexes.end())
					sqlstmt += u8", ";
			}
			sqlstmt += u8" FROM ";
			sqlstmt += ti.GetQueryName();
			tbl.SelectBySqlStmt(sqlstmt);
		}
		else
		{
			tbl.Select();
		}
		while (tbl.SelectNext())
		{
			stringstream ss;
			for (set<SQLUSMALLINT>::const_iterator it = colIndexes.begin(); it != colIndexes.end(); ++it)
			{
				ColumnBufferPtrVariant pColVar = tbl.GetColumnBufferPtrVariant(*it);
				StringColumnWrapper wa(pColVar);
				ss << u8"||= ";
				if (wa.IsNull())
				{
					ss << u8"NULL";
				}
				else
				{
					ss << wa.GetValue<std::string>();
				}
				ss << u8" =";
			}
			ss << u8"||";
			lines.push_back(ss.str());
		}
		return lines;
	}


	vector<string> CreateTracPages::GetDbInfoLines(SqlInfoProperty::InfoType infoType)
	{
		const SqlInfoProperties::PropertiesSet& props = m_pDb->GetProperties().GetSubset(infoType);
		vector<string> lines;
		switch (infoType)
		{
		case SqlInfoProperty::InfoType::Conversion:
			lines.push_back(u8"== Conversion ==");
			break;
		case SqlInfoProperty::InfoType::DataSource:
			lines.push_back(u8"== Data Source Information ==");
			break;
		case SqlInfoProperty::InfoType::DBMS:
			lines.push_back(u8"== DBMS Product Information ==");
			break;
		case SqlInfoProperty::InfoType::Driver:
			lines.push_back(u8"== Driver Information ==");
			break;
		case SqlInfoProperty::InfoType::ScalarFunction:
			lines.push_back(u8"== Scalar Function Information ==");
			break;
		case SqlInfoProperty::InfoType::SqlLimits:
			lines.push_back(u8"== SQL Limits Information ==");
			break;
		case SqlInfoProperty::InfoType::SupportedSql:
			lines.push_back(u8"== Supported SQL Information ==");
			break;
		}
		lines.push_back(u8"||=Property Name =||= Property Value =||");
		for (auto it = props.begin(); it != props.end(); ++it)
		{
			SqlInfoProperty::ValueType vt = it->GetValueType();
			string strVal = it->GetStringValue();
			// output of SQL_KEYWORDS has no ' ' after ',', what looks ugle
			if (it->GetInfoId() == SQL_KEYWORDS)
				boost::algorithm::replace_all(strVal, u8",", u8", ");
			// format numberic values right and center text, except Y_N values
			if(vt == SqlInfoProperty::ValueType::UInt || vt == SqlInfoProperty::ValueType::USmallInt)
				lines.push_back(boost::str(boost::format(u8"|| %s || %s||") % it->GetName() % strVal));
			else
				lines.push_back(boost::str(boost::format(u8"|| %s ||  %s  ||") % it->GetName() % strVal));
		}
		return lines;
	}


	vector<string> CreateTracPages::GetHeaderLines()
	{
		vector<string> lines;
		string addon;
		if (m_mode == Mode::DbInfo)
			addon = u8"Database and Driver Information";
		else
			addon = u8"Test Table Information";
		const SqlInfoProperties& props = m_pDb->GetProperties();
		lines.push_back(boost::str(boost::format(u8"= %s: %s =") % props.GetDbmsName() % addon));
		lines.push_back(u8"[[PageOutline]]");
		return lines;
	}


	vector<string> CreateTracPages::GetConnectionLines()
	{
		vector<string> lines;
		lines.push_back(u8"== Connection Information ==");
		lines.push_back(u8"The following connection information was used to create this page:");
		if (m_pDb->OpenedWithConnectionString())
		{
			lines.push_back(boost::str(boost::format(u8"* Connection String: //%s//") % m_pDb->GetConnectionInStr()));
		}
		else
		{
			lines.push_back(boost::str(boost::format(u8"* DSN: //%s//") % m_pDb->GetDataSourceName()));
			lines.push_back(boost::str(boost::format(u8"* User: //%s//") % m_pDb->GetUsername()));
			lines.push_back(boost::str(boost::format(u8"* Pass: //%s//") % m_pDb->GetPassword()));
		}
		return lines;
	}


	vector<string> CreateTracPages::GetTypeLines()
	{
		vector<string> lines;
		DatabaseCatalogPtr pDbCat = m_pDb->GetDbCatalog();
		SqlTypeInfoVector types = pDbCat->ReadSqlTypeInfo();

		string header = u8"||= SQLType =||= SQL Data Type (3) =||= !TypeName =||=  Local !TypeName =||= Unsigned =||= Column Size =||= Nullable ="
						u8"||= Auto Unique =||= Case Sensitive =||= Searchable =||= Prefix =||= Suffix =||= Fixed Prec. Scale =||= Min. Scale ="
						u8"||= Max. Scale =||= Sql !DateTimeSub =||= Num. Prec. Radix =||= Interval Precision =||= Create Params =||";
		lines.push_back(header);

		boost::format numberFormat(u8"%d");
		boost::format lineFormat(
			u8"||= %s (%d) =||= %s (%d) =||= %s =||= %s =||= %s =||= %d =||= %s ="
			u8"||= %s =||= %s =||= %s =||= %s =||= %s =||= %s =||= %s ="
			u8"||= %s =||= %s =||= %s =||= %s =||= %s =||");
		for (SqlTypeInfoVector::const_iterator it = types.begin(); it != types.end(); ++it)
		{
			const SqlTypeInfo& ti = *it;
			string s = boost::str(lineFormat 
				% Sql2StringHelper::SqlType2s(ti.GetSqlType()) % ti.GetSqlType() 
				% Sql2StringHelper::SqlType2s(ti.GetSqlDataType()) % ti.GetSqlDataType() 
				% ti.GetTypeName()
				% (ti.IsLocalTypeNameNull() ? u8"NULL" : ti.GetLocalTypeName())
				% (ti.IsUnsignedNull() ? u8"NULL" : Sql2StringHelper::SqlTrueFalse2s(ti.GetUnsigned()))
				% (ti.IsColumnSizeNull() ? 0 : ti.GetColumnSize())
				% Sql2StringHelper::SqlNullable2s(ti.GetNullable())
				% (ti.IsAutoUniqueValueNull() ? u8"NULL" : Sql2StringHelper::SqlTrueFalse2s(ti.GetAutoUniqueValue()))
				% Sql2StringHelper::SqlTrueFalse2s(ti.GetCaseSensitive())
				% Sql2StringHelper::SqlSearchable2s(ti.GetSearchable())
				% (ti.IsLiteralPrefixNull() ? u8"NULL" : ti.GetLiteralPrefix())
				% (ti.IsLiteralSuffixNull() ? u8"NULL" : ti.GetLiteralSuffix())
				% Sql2StringHelper::SqlTrueFalse2s(ti.GetFixedPrecisionScale())
				% (ti.IsMinimumScaleNull() ? u8"NULL" : boost::str(numberFormat % ti.GetMinimumScale()))
				% (ti.IsMaximumScaleNull() ? u8"NULL" : boost::str(numberFormat % ti.GetMaximumScale()))
				% (ti.IsSqlDateTimeSubNull() ? u8"NULL" : boost::str(numberFormat % ti.GetSqlDateTimeSub()))
				% (ti.IsNumPrecRadixNull() ? u8"NULL" : boost::str(numberFormat % ti.GetNumPrecRadix()))
				% (ti.IsIntervalPrecisionNull() ? u8"NULL" : boost::str(numberFormat % ti.GetIntervalPrecision()))
				% (ti.IsCreateParamsNull() ? u8"NULL" : ti.GetCreateParams())
			);
			lines.push_back(s);
		}

		return lines;
	}


	void CreateTracPages::AddEmptyLine(std::vector<std::string>& lines) const noexcept
	{
		lines.push_back(u8"");
	}
}