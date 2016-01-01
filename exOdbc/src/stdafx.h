/*!
* \file stdafx.h
* \author Elias Gerber <eg@elisium.ch>
* \date 23.07.2014
* \brief Goes into precompiled header.
* \copyright GNU Lesser General Public License Version 3
*
* Include file for standard system include files,
* or project specific include files that are used frequently, but
* are changed infrequently
*/


#pragma once
#pragma warning(disable:4503)

#include "targetver.h"

// libs
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/log/trivial.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/signals2.hpp"

// system
#include <windows.h>
#include <sstream>
#include <string>
#include <set>
#include <locale>
#include <codecvt>
//#include <cstdio>
//#include <tchar>
//#include <string.h>
//#include <assert.h>
//#include <stdlib.h>
//#include <ctype.h>
//#include <vector>
//#include <iostream>

// odbc-things
#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>
#include <odbcinst.h>

// our stuff
#include "exOdbc.h"

// TODO: reference additional headers your program requires here
