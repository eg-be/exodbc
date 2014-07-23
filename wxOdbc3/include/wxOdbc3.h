/*!
 * \file wxOdbc3.h
 * \author Elias Gerber <egerber@gmx.net>
 * \date 09.02.2014
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef WXODBC3_H
#define WXODBC3_H

// Defines to dll-import/export
// ----------------------------

#define WXODBC3EXPORT __declspec(dllexport)
#define WXODBC3IMPORT __declspec(dllimport)

#ifdef WXODBC3_EXPORTS
#    define WXDLLIMPEXP_ODBC WXODBC3EXPORT
#    define WXDLLIMPEXP_DATA_ODBC(type) WXODBC3EXPORT type
#elif defined(WXODBC3_IMPORTS)
#    define WXDLLIMPEXP_ODBC WXODBC3IMPORT
#    define WXDLLIMPEXP_DATA_ODBC(type) WXODBC3IMPORT type
#else /* not making nor using DLL */
#    define WXDLLIMPEXP_ODBC
#    define WXDLLIMPEXP_DATA_ODBC(type) type
#endif

// Global Consts
// -------------
WXDLLIMPEXP_ODBC extern const wchar_t* emptyString;


#endif // WXODBC3_H
