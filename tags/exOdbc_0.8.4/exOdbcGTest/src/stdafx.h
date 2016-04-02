// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma warning(disable: 4503)	// 'identifier' : decorated name length exceeded, name was truncated
#pragma warning(disable: 4251)	// 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
#define _SCL_SECURE_NO_WARNINGS 1	// 'function': was declared deprecated also 'std::<function name>': Function call with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct.
#define _CRT_SECURE_NO_WARNINGS 1	// 'function': This function or variable may be unsafe. Consider using strcpy_s instead.
#include "targetver.h"

// libs
#include "gtest/gtest.h"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"
#include "boost/iostreams/device/file_descriptor.hpp"
#include "boost/iostreams/stream.hpp"
#include "boost/filesystem.hpp"
#include "boost/algorithm/algorithm.hpp"
#include "boost/foreach.hpp"

// system
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <sstream>
#include <codecvt>

// TODO: reference additional headers your program requires here
