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

#include "targetver.h"

// libs - boos stuff
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/signals2.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/variant.hpp"
#include "boost/variant/polymorphic_get.hpp"

// system
#include <windows.h>
#include <locale>
#include <codecvt>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>
#include <map>

// odbc-things
#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>
#include <odbcinst.h>

