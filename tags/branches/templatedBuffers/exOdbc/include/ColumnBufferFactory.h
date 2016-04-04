/*!
* \file ColumnBufferFactory.h
* \author Elias Gerber <eg@elisium.ch>
* \date 18.10.2015
* \brief Header file for the CBufferType interface.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "Exception.h"
#include "InfoObject.h"

// Other headers
#include <boost/any.hpp>

// System headers
#include <mutex>

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
	class EXODBCAPI IColumnBuffer
	{
	public:
		virtual void BindSelect(SQLHSTMT hStmt, SQLSMALLINT columnNr) const = 0;
		virtual SQLSMALLINT GetSqlCType() const = 0;

		virtual bool IsNull() const = 0;
		virtual void SetNull() = 0;
	};

	class EXODBCAPI ColumnBufferFactory
	{
		typedef std::function<std::shared_ptr<IColumnBuffer>(SQLSMALLINT)> BufferCreationFunc;
		typedef std::map<SQLSMALLINT, BufferCreationFunc> BufferCreatorFuncsMap;

	public:
		static ColumnBufferFactory& Instance() throw();
		~ColumnBufferFactory() {};

	private:
		ColumnBufferFactory() {};
		ColumnBufferFactory(const ColumnBufferFactory& other) {};

	public:
		std::shared_ptr<IColumnBuffer> CreateColumnBuffer(SQLSMALLINT sqlCBufferType);

		boost::any CreateSqlCBuffer(SQLSMALLINT sqlCBufferType) const;

		void RegisterColumnBufferCreationFunc(SQLSMALLINT sqlCBufferType, BufferCreationFunc func);

	private:
		BufferCreatorFuncsMap m_creatorFuncs;

		// todo: cannot use mutex if we do from dllmain, remove from dllmain
//		std::mutex m_creatorFuncsMutex;
	};

	typedef std::shared_ptr<IColumnBuffer> IColumnBufferPtr;


} // namespace exodbc
