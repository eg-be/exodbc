/*!
* \file SetDescriptionFieldWrapper.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 04.05.2017
* \brief Source file for the SetDescriptionFieldWrapper
* \copyright GNU Lesser General Public License Version 3
*/ 

// Own header
#include "SetDescriptionFieldWrapper.h"

// Same component headers
#include "LogManagerOdbcMacros.h"

// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------
namespace exodbc
{

	// Construction
	// -------------

	// Destructor
	// -----------

	// Implementation
	// --------------
	void SetDescriptionFieldWrapper::SetDescriptionField(SQLHDESC hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value)
	{
		exASSERT(hDesc != SQL_NULL_HDESC);
		exASSERT(recordNumber > 0);
		SQLRETURN ret = SQLSetDescField(hDesc, recordNumber, descriptionField, value, 0);
		THROW_IFN_SUCCEEDED(SQLSetDescField, ret, SQL_HANDLE_DESC, hDesc);
	}


	void SetDescriptionFieldWrapper::SetDescriptionField(ConstSqlDescHandlePtr pHDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value)
	{
		exASSERT(pHDesc);
		exASSERT(pHDesc->IsAllocated());
		SetDescriptionField(pHDesc->GetHandle(), recordNumber, descriptionField, value);
	}


	void SetDescriptionFieldWrapper::SetDescriptionField(const SqlDescHandle& hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value)
	{
		exASSERT(hDesc.IsAllocated());
		SetDescriptionField(hDesc.GetHandle(), recordNumber, descriptionField, value);
	}
}


