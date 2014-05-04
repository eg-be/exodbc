
#include "TestParams.h"

#include <sstream>

namespace wxOdbc3Test
{
	::std::ostream& operator<<(::std::ostream& os, const SOdbcInfo& oi) {
		std::wstringstream wos;
		wos << L"DSN: " << oi.m_dsn.c_str() 
			<< L"; Username: " << oi.m_username.c_str()
			<< L"; Password: " << oi.m_password.c_str() 
			<< L"; Forward-Only Cursor: " << (oi.m_cursorType == SOdbcInfo::forwardOnlyCursors);
		std::string s;
		eli::w2mbNoThrow(wos.str(), s);
		return os << s.c_str();
	}
}