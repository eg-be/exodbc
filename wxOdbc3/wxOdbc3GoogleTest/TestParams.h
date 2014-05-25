#include "Utils.h"

namespace wxOdbc3Test
{
	struct SOdbcInfo
	{
		enum CursorType
		{
			forwardOnlyCursors,
			notForwardOnlyCursors
		};

		SOdbcInfo()
		{ }

		SOdbcInfo(const std::wstring& dsn, const std::wstring& username, const std::wstring& password, CursorType cursorType) 
			: m_dsn(dsn)
			, m_username(username)
			, m_password(password)
			, m_cursorType(cursorType)
		{};
		std::wstring m_dsn;
		std::wstring m_username;
		std::wstring m_password;
		CursorType m_cursorType;
	};

	::std::ostream& operator<<(::std::ostream& os, const SOdbcInfo& oi);

}