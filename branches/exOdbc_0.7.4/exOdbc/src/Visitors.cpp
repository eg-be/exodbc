/*!
* \file Visitors.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2015
* \brief Source file for the Visitors of the BufferPtrVariant.
* \copyright GNU Lesser General Public License Version 3
*
*/

#include "stdafx.h"

// Own header
#include "Visitors.h"

// Same component headers

// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	SQL_TIMESTAMP_STRUCT TimestampVisitor::operator()(SQL_TIME_STRUCT* pTime) const
	{
		SQL_TIMESTAMP_STRUCT timestamp;
		ZeroMemory(&timestamp, sizeof(timestamp));
		timestamp.hour = pTime->hour;
		timestamp.minute = pTime->minute;
		timestamp.second = pTime->second;
		return timestamp;
	}


	SQL_TIMESTAMP_STRUCT TimestampVisitor::operator()(SQL_DATE_STRUCT* pDate) const
	{
		SQL_TIMESTAMP_STRUCT timestamp;
		ZeroMemory(&timestamp, sizeof(timestamp));
		timestamp.day = pDate->day;
		timestamp.month = pDate->month;
		timestamp.year = pDate->year;
		return timestamp;
	};


	SQL_TIMESTAMP_STRUCT TimestampVisitor::operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const
	{
		SQL_TIMESTAMP_STRUCT timestamp = *pTimestamp;
		return timestamp;
	}


#if HAVE_MSODBCSQL_H
	SQL_TIMESTAMP_STRUCT TimestampVisitor::operator()(SQL_SS_TIME2_STRUCT* pTime) const
	{
		SQL_TIMESTAMP_STRUCT timestamp;
		ZeroMemory(&timestamp, sizeof(timestamp));
		timestamp.hour = pTime->hour;
		timestamp.minute = pTime->minute;
		timestamp.second = pTime->second;
		timestamp.fraction = pTime->fraction;
		//ColumnBuffer::TrimValue(m_decimalDigits, timestamp.fraction);
		return timestamp;
	};
#endif


	// Interfaces
	// ----------

}
