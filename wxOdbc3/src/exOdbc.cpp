/*!
* \file wxOdbc3.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 23.07.2014
* 
* [Brief CPP-file description]
*/ 

#include "stdafx.h"

// Own header
#include "exOdbc.h"

// Same component headers
#include "Helpers.h"

// Other headers

namespace exodbc {

	// Static consts
	// -------------
	const wchar_t* emptyString				= L"";
	const wchar_t* SQL_LOG_FILENAME         = L"sqllog.txt";
	const wchar_t* SQL_CATALOG_FILENAME     = L"catalog.txt";

	// Implementation
	// --------------
	SDbInfo::SDbInfo()
	{
		dbmsName[0] = 0;
		dbmsVer[0] = 0;
		driverName[0] = 0;
		odbcVer[0] = 0;
		drvMgrOdbcVer[0] = 0;
		driverVer[0] = 0;
		serverName[0] = 0;
		databaseName[0] = 0;
		outerJoins[0] = 0;
		procedureSupport[0] = 0;
		accessibleTables[0] = 0;
		maxConnections = 0;
		maxStmts = 0;
		cliConfLvl = 0;
		cursorCommitBehavior = 0;
		cursorRollbackBehavior = 0;
		supportNotNullClause = 0;
		supportIEF[0] = 0;
		txnIsolation = 0;
		txnIsolationOptions = 0;
		posOperations = 0;
		posStmts = 0;
		scrollOptions = 0;
		txnCapable = 0;
		maxCatalogNameLen = 0;
		maxSchemaNameLen = 0;
		maxTableNameLen = 0;
	}

	std::wstring SDbInfo::ToStr() const
	{
		std::wstringstream ws;
		ws << L"***** DATA SOURCE INFORMATION *****" << std::endl;
		ws << L"SERVER Name: " << serverName << std::endl;
		ws << L"DBMS Name: " << dbmsName << L"; DBMS Version: " << dbmsVer << std::endl;
		ws << L"ODBC Version: " << odbcVer << L"; Driver Version: " << driverVer << std::endl;

		ws << L"SAG CLI Conf. Level: ";
		switch(cliConfLvl)
		{
		case SQL_OSCC_NOT_COMPLIANT:    ws << L"Not Compliant";    break;
		case SQL_OSCC_COMPLIANT:        ws << L"Compliant";        break;
		}
		ws << std::endl;

		ws << L"Max. Connections: "       << maxConnections   << std::endl;
		ws << L"Outer Joins: "            << outerJoins       << std::endl;
		ws << L"Support for Procedures: " << procedureSupport << std::endl;
		ws << L"All tables accessible : " << accessibleTables << std::endl;
		ws << L"Cursor COMMIT Behavior: ";
		switch(cursorCommitBehavior)
		{
		case SQL_CB_DELETE:        ws << L"Delete cursors";      break;
		case SQL_CB_CLOSE:         ws << L"Close cursors";       break;
		case SQL_CB_PRESERVE:      ws << L"Preserve cursors";    break;
		}
		ws << std::endl;

		ws << L"Cursor ROLLBACK Behavior: ";
		switch(cursorRollbackBehavior)
		{
		case SQL_CB_DELETE:      ws << L"Delete cursors";      break;
		case SQL_CB_CLOSE:       ws << L"Close cursors";       break;
		case SQL_CB_PRESERVE:    ws << L"Preserve cursors";    break;
		}
		ws << std::endl;

		ws << L"Support NOT NULL clause: ";
		switch(supportNotNullClause)
		{
		case SQL_NNC_NULL:        ws << L"No";        break;
		case SQL_NNC_NON_NULL:    ws << L"Yes";       break;
		}
		ws << std::endl;

		ws << L"Support IEF (Ref. Integrity): " << supportIEF   << std::endl;

		ws << L"Default Transaction Isolation: ";
		switch(txnIsolation)
		{
		case SQL_TXN_READ_UNCOMMITTED:  ws << L"Read Uncommitted";    break;
		case SQL_TXN_READ_COMMITTED:    ws << L"Read Committed";      break;
		case SQL_TXN_REPEATABLE_READ:   ws << L"Repeatable Read";     break;
		case SQL_TXN_SERIALIZABLE:      ws << L"Serializable";        break;
		}
		ws << std::endl;

		ws << L"Transaction Isolation Options: ";
		if (txnIsolationOptions & SQL_TXN_READ_UNCOMMITTED)
			ws << L"Read Uncommitted, ";
		if (txnIsolationOptions & SQL_TXN_READ_COMMITTED)
			ws << L"Read Committed, ";
		if (txnIsolationOptions & SQL_TXN_REPEATABLE_READ)
			ws << L"Repeatable Read, ";
		if (txnIsolationOptions & SQL_TXN_SERIALIZABLE)
			ws << L"Serializable, ";
		ws << std::endl;

		ws << L"Position Operations Supported (SQLSetPos): ";
		if (posOperations & SQL_POS_POSITION)
			ws << L"Position, ";
		if (posOperations & SQL_POS_REFRESH)
			ws << L"Refresh, ";
		if (posOperations & SQL_POS_UPDATE)
			ws << L"Upd, ";
		if (posOperations & SQL_POS_DELETE)
			ws << L"Del, ";
		if (posOperations & SQL_POS_ADD)
			ws << L"Add";
		ws << std::endl;

		ws << L"Positioned Statements Supported: ";
		if (posStmts & SQL_PS_POSITIONED_DELETE)
			ws << L"Pos delete, ";
		if (posStmts & SQL_PS_POSITIONED_UPDATE)
			ws << L"Pos update, ";
		if (posStmts & SQL_PS_SELECT_FOR_UPDATE)
			ws << L"Select for update";
		ws << std::endl;

		ws << L"Scroll Options: ";
		if (scrollOptions & SQL_SO_FORWARD_ONLY)
			ws << L"Fwd Only, ";
		if (scrollOptions & SQL_SO_STATIC)
			ws << L"Static, ";
		if (scrollOptions & SQL_SO_KEYSET_DRIVEN)
			ws << L"Keyset Driven, ";
		if (scrollOptions & SQL_SO_DYNAMIC)
			ws << L"Dynamic, ";
		if (scrollOptions & SQL_SO_MIXED)
			ws << L"Mixed";
		ws << std::endl;

		ws << L"Transaction Capable?: ";
		switch(txnCapable)
		{
		case SQL_TC_NONE:          ws << L"No";            break;
		case SQL_TC_DML:           ws << L"DML Only";      break;
		case SQL_TC_DDL_COMMIT:    ws << L"DDL Commit";    break;
		case SQL_TC_DDL_IGNORE:    ws << L"DDL Ignore";    break;
		case SQL_TC_ALL:           ws << L"DDL & DML";     break;
		}
		ws << std::endl;

		ws << std::endl;

		return ws.str();
	}


	SSqlTypeInfo::SSqlTypeInfo()
	{
		FsqlType = 0;
		Precision = 0;
		ColumSizeIsNull = false;
		LiteralPrefixIsNull = false;
		LiteralSuffixIsNull = false;
		CreateParamsIsNull = false;
		Nullable = SQL_NULLABLE_UNKNOWN;
		CaseSensitive = SQL_FALSE;
		Searchable = SQL_PRED_NONE;
		Unsigned = SQL_FALSE;
		UnsignedIsNull = false;
		FixedPrecisionScale = SQL_FALSE;
		AutoUniqueValue = SQL_FALSE;
		AutoUniqueValueIsNull = false;
		LocalTypeNameIsNull = false;
		MinimumScale = 0;
		MinimumScaleIsNull = false;
		MaximumScale = 0;
		MaximumScaleIsNull = false;
		SqlDataType = 0;
		SqlDateTimeSub = 0;
		SqlDateTimeSubIsNull = false;
		NumPrecRadix = 0;
		NumPrecRadixIsNull = false;
		IntervalPrecision = 0;
		IntervalPrecisionIsNull = false;
	}

}