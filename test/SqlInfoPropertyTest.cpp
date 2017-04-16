/*!
* \file SqlInfoPropertyTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 16.04.2017
* \copyright GNU Lesser General Public License Version 3
* 
* [Brief CPP-file description]
*/ 

// Own header
#include "SqlInfoPropertyTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

// Other headers
// Debug
#include "DebugNew.h"

using namespace exodbc;
using namespace std;

using it = SqlInfoProperty::InfoType;
using vt = SqlInfoProperty::ValueType;

namespace exodbctest
{


	// Static consts
	// -------------

	// Construction
	// -------------

	// Destructor
	// -----------

	// Implementation
	// --------------
	TEST_F(SqlInfoPropertyTest, ConstructDefaultValue)
	{
		// Test that default values are set and flag read is set to false
		SqlInfoProperty propUSmallInt(SQL_MAX_DRIVER_CONNECTIONS, u8"SQL_MAX_DRIVER_CONNECTIONS", it::Driver, vt::USmallInt);
		SqlInfoProperty propUInt(SQL_FILE_USAGE, u8"SQL_FILE_USAGE", it::Driver, vt::UInt);
		SqlInfoProperty propNYString(SQL_ROW_UPDATES, u8"SQL_ROW_UPDATES", it::Driver, vt::String_N_Y);
		SqlInfoProperty propAnyString(SQL_ODBC_VER, u8"SQL_ODBC_VER", it::Driver, vt::String_Any);

		EXPECT_EQ((SQLUSMALLINT)0, boost::get<SQLUSMALLINT>(propUSmallInt.GetValue()));
		EXPECT_EQ((SQLUINTEGER)0, boost::get<SQLUINTEGER>(propUInt.GetValue()));
		EXPECT_EQ(u8"N", boost::get<string>(propNYString.GetValue()));
		EXPECT_EQ(u8"", boost::get<string>(propAnyString.GetValue()));
	}


	TEST_F(SqlInfoPropertyTest, SqlInfoPropertyStringValueVisitor)
	{
		SqlInfoProperty propUSmallInt(SQL_MAX_DRIVER_CONNECTIONS, u8"SQL_MAX_DRIVER_CONNECTIONS", it::Driver, vt::USmallInt);
		SqlInfoProperty propUInt(SQL_FILE_USAGE, u8"SQL_FILE_USAGE", it::Driver, vt::UInt);
		SqlInfoProperty propNYString(SQL_ROW_UPDATES, u8"SQL_ROW_UPDATES", it::Driver, vt::String_N_Y);
		SqlInfoProperty propAnyString(SQL_ODBC_VER, u8"SQL_ODBC_VER", it::Driver, vt::String_Any);

		EXPECT_EQ(u8"0 (0x0)", boost::apply_visitor(SqlInfoPropertyStringValueVisitor(), propUSmallInt.GetValue()));
		EXPECT_EQ(u8"0 (0x0)", boost::apply_visitor(SqlInfoPropertyStringValueVisitor(), propUInt.GetValue()));
		EXPECT_EQ(u8"N", boost::apply_visitor(SqlInfoPropertyStringValueVisitor(), propNYString.GetValue()));
		EXPECT_EQ(u8"", boost::apply_visitor(SqlInfoPropertyStringValueVisitor(), propAnyString.GetValue()));
	}


	TEST_F(SqlInfoPropertyTest, ReadUSmallIntValue)
	{
		DatabasePtr pDb = OpenTestDb(OdbcVersion::V_3);

		// Try to read two values and hope that the driver will return a non-default value
		SqlInfoProperty prop1(SQL_NULL_COLLATION, u8"SQL_NULL_COLLATION", it::DataSource, vt::USmallInt);
		SqlInfoProperty prop2(SQL_TXN_CAPABLE, u8"SQL_TXN_CAPABLE", it::DataSource, vt::USmallInt);

		SQLSMALLINT v1 = boost::get<SQLUSMALLINT>(prop1.GetValue());
		SQLSMALLINT v2 = boost::get<SQLUSMALLINT>(prop2.GetValue());

		EXPECT_EQ(0, v1);
		EXPECT_EQ(0, v2);

		prop1.ReadProperty(pDb->GetSqlDbcHandle());
		prop2.ReadProperty(pDb->GetSqlDbcHandle());
		v1 = boost::get<SQLUSMALLINT>(prop1.GetValue());
		v2 = boost::get<SQLUSMALLINT>(prop2.GetValue());

		EXPECT_TRUE(v1 != 0 || v2 != 0);
	}


	TEST_F(SqlInfoPropertyTest, ReadUIntValue)
	{
		DatabasePtr pDb = OpenTestDb(OdbcVersion::V_3);

		// Try to read two values and hope that the driver will return a non-default value
		SqlInfoProperty prop1(SQL_ODBC_INTERFACE_CONFORMANCE, u8"SQL_ODBC_INTERFACE_CONFORMANCE", it::DataSource, vt::UInt);

		SQLUINTEGER v1 = boost::get<SQLUINTEGER>(prop1.GetValue());

		EXPECT_EQ(0, v1);

		prop1.ReadProperty(pDb->GetSqlDbcHandle());
		v1 = boost::get<SQLUINTEGER>(prop1.GetValue());

		EXPECT_NE(0, v1);
	}


	TEST_F(SqlInfoPropertyTest, ReadStringValue)
	{
		DatabasePtr pDb = OpenTestDb(OdbcVersion::V_3);

		// Try to read two values and hope that the driver will return a non-default value
		SqlInfoProperty prop1(SQL_DATA_SOURCE_NAME, u8"SQL_DATA_SOURCE_NAME", it::DataSource, vt::String_Any);

		string v1 = boost::get<string>(prop1.GetValue());

		EXPECT_EQ(u8"", v1);

		prop1.ReadProperty(pDb->GetSqlDbcHandle());
		v1 = boost::get<string>(prop1.GetValue());

		EXPECT_NE(u8"", v1);
	}


	TEST_F(SqlInfoPropertiesTest, Construct)
	{
		// depending on odbc version, we have more and more elements
		// but depending on the driver, we have more and more warnings about
		// not supported properties, so:

		DatabasePtr pDb = OpenTestDb(OdbcVersion::V_3);

		LogLevelSetter lls(LogLevel::Error);

		SqlInfoProperties props2(pDb->GetSqlDbcHandle(), OdbcVersion::V_2);
		SqlInfoProperties props3(pDb->GetSqlDbcHandle(), OdbcVersion::V_3);
		SqlInfoProperties props38(pDb->GetSqlDbcHandle(), OdbcVersion::V_3_8);

		EXPECT_GT(props3.GetPropertyCount(), props2.GetPropertyCount());
		EXPECT_GT(props38.GetPropertyCount(), props3.GetPropertyCount());
	}


	TEST_F(SqlInfoPropertiesTest, Reset)
	{
		// Must clear all props on reset
		DatabasePtr pDb = OpenTestDb(OdbcVersion::V_3);

		SqlInfoProperties props;
		EXPECT_EQ(0, props.GetPropertyCount());
		props.Init(pDb->GetSqlDbcHandle());
		EXPECT_GT(props.GetPropertyCount(), (size_t)0);
		props.Reset();
		EXPECT_EQ(0, props.GetPropertyCount());
	}


	TEST_F(SqlInfoPropertiesTest, GetProperty)
	{
		// Fail to get unknown prop
		DatabasePtr pDb = OpenTestDb(OdbcVersion::V_3);

		SqlInfoProperties props(pDb->GetSqlDbcHandle(), OdbcVersion::V_2);

		EXPECT_NO_THROW(props.GetProperty(SQL_DATA_SOURCE_NAME));
		EXPECT_THROW(props.GetProperty(SQL_ASYNC_DBC_FUNCTIONS), NotFoundException);
	}

} // namespace exodbctest
