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

// Other headers
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
	class IColumnBuffer
	{
	public:
	};

	class ColumnBufferFactory
	{
		typedef std::function<std::shared_ptr<IColumnBuffer>(SQLSMALLINT)> BufferCreationFunc;
		typedef std::map<SQLSMALLINT, BufferCreationFunc> BufferCreatorFuncsMap;

	public:
		static ColumnBufferFactory& Instance() throw();
		~ColumnBufferFactory() {};

	private:
		ColumnBufferFactory() {};
		ColumnBufferFactory(const ColumnBufferFactory& other) {};

		std::shared_ptr<IColumnBuffer> CreateColumnBuffer(SQLSMALLINT sqlCBufferType) const;
		void RegisterColumnBufferCreationFunc(SQLSMALLINT sqlCBufferType, BufferCreationFunc func);

	private:

		BufferCreatorFuncsMap m_creatorFuncs;
		mutable std::mutex m_creatorFuncsMutex;
	};


} // namespace exodbc
