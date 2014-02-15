#define TEST_SQL(x, db) \
do { \
	bool ok = x; \
	if(!ok) { \
		std::wcout << L"\nSql-Error in function " << __FUNCTION__ << L"\n" << L"\t " << __TFILE__ << L"(" << __LINE__ << L")" ; \
		std::vector<wxString> errors = db->GetErrorList(); \
		for(size_t i = 0; i < errors.size(); i++) \
		{ \
			std::wcout << errors[i] << L"\n"; \
		} \
	} \
	CPPUNIT_ASSERT(ok); \
} while(0)

