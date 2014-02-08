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
