/*!
* \file CreateTestDbPageCommand.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 07.05.2017
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "CreateTestDbPageCommand.h"

// Same component headers
#include "ExodbcExec.h"

// Other headers
#include "exodbc/exOdbc.h"
#include "exodbc/LogManager.h"
#include "exodbc/LogHandler.h"

// Debug
#include "DebugNew.h"

using namespace std;
using namespace exodbc;

namespace exodbcexec
{
	string CreateTestDbPageCommand::GetArgumentsSyntax() const noexcept
	{
		return u8"[outfile]";
	}

	void CreateTestDbPageCommand::Execute(const std::vector<std::string> & args)
	{
		FileLogHandlerPtr pFileLogger = nullptr;

		if (!args.empty())
		{
			LOG_INFO(boost::str(boost::format(u8"Writting all output for Trac to '%s'") % args[0]));
			pFileLogger = std::make_shared<FileLogHandler>(args[0], false);
			pFileLogger->SetShowLogLevel(false);
			pFileLogger->SetShowFileInfo(false);
			LogManager::Get().RegisterLogHandler(pFileLogger);
		}

		vector<string> lines;
		vector<string> header = GetHeaderLines();
		lines.insert(lines.end(), header.begin(), header.end());
		vector<string> connectionLines = GetConnectionLines();
		lines.insert(lines.end(), connectionLines.begin(), connectionLines.end());
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

		for (vector<string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			LOG_OUTPUT(*it);
		}

		if (pFileLogger != nullptr)
		{
			LogManager::Get().RemoveLogHandler(pFileLogger);
		}
	}


	vector<string> CreateTestDbPageCommand::GetDbInfoLines(SqlInfoProperty::InfoType infoType)
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
		lines.push_back(GetPropertyTableNameHeader());
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


	vector<string> CreateTestDbPageCommand::GetHeaderLines()
	{
		vector<string> lines;
		const SqlInfoProperties& props = m_pDb->GetProperties();
		lines.push_back(boost::str(boost::format(u8"= %s: Database and Driver information =") % props.GetDbmsName()));
		lines.push_back(u8"[[PageOutline]]");
		return lines;
	}


	string CreateTestDbPageCommand::GetPropertyTableNameHeader()
	{
		return u8"||=Property Name =||= Property Value =||";
	}


	vector<string> CreateTestDbPageCommand::GetConnectionLines()
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


	vector<string> CreateTestDbPageCommand::GetTypeLines()
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


	void CreateTestDbPageCommand::AddEmptyLine(std::vector<std::string>& lines) const noexcept
	{
		lines.push_back(u8"");
	}
}