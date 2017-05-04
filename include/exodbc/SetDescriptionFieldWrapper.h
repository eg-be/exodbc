/*!
* \file SetDescriptionFieldWrapper.h
* \author Elias Gerber <eg@elisium.ch>
* \date 04.05.2017
* \brief Header file for SetDescriptionFieldWrapper.
* \copyright GNU Lesser General Public License Version 3
*/ 

#pragma once

// Same component headers
#include "exOdbc.h"
#include "SqlHandle.h"

// Other headers
// System headers

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \class SetDescriptionFieldWrapper
	* \brief Provides convenience wrappers for SQLSetDescField
	*/
	class EXODBCAPI SetDescriptionFieldWrapper
	{
	public:
		/*!
		* \see SetDescriptionField(SQLHDESC hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value)
		*/
		static void		SetDescriptionField(ConstSqlDescHandlePtr pHDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value);


		/*!
		* \see SetDescriptionField(SQLHDESC hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value)
		*/
		static void		SetDescriptionField(const SqlDescHandle& hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value);


	private:
		/*!
		* \brief	A wrapper to SQLSetDescField
		*
		* \param	hDesc		 	The description handle to set an attribute for.
		* \param	recordNumber	Column number to set the attribute for.
		* \param	descriptionField Attribute to set.
		* \param	value			Value to set.
		* \throw	Exception
		*/
		static void		SetDescriptionField(SQLHDESC hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value);
	};
}

