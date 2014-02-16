// Some helper functions
// ---------------------
void PrintErrors(wxDb* pDb, const wxString& file, int line);

// Some helper macros
// ------------------
#define CATCH_LOG_RETHROW \
catch(CPPUNIT_NS::Exception e) \
{ \
	PrintErrors(m_pDbMySql, __TFILE__, __LINE__); \
	throw e; \
}

#define CATCH_LOG_RETHROW_DELETE_TABLE(pTable) \
	catch(CPPUNIT_NS::Exception e) \
{ \
	PrintErrors(m_pDbMySql, __TFILE__, __LINE__); \
	if(pTable) \
		delete pTable; \
	throw e; \
}

//#define TEST_SQL(x, db) \
//do { \
//	bool ok = x; \
//	if(!ok) { \
//		std::wcout << L"\nSql-Error in function " << __FUNCTION__ << L"\n" << L"\t " << __TFILE__ << L"(" << __LINE__ << L")" ; \
//		std::vector<wxString> errors = db->GetErrorList(); \
//		for(size_t i = 0; i < errors.size(); i++) \
//		{ \
//			std::wcout << errors[i] << L"\n"; \
//		} \
//	} \
//	CPPUNIT_ASSERT(ok); \
//} while(0)

