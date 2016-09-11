#include "mylib.h"
#include "myLibConfig.h"
#include <iostream>

namespace myLib
{
	int sum(int a, int b)
	{
		std::wcout << L"Version: " << myLib_VERSION_MAJOR << L"." << myLib_VERSION_MINOR << L"." << myLib_VERSION_BUILD << std::endl;
		return a + b;
	}
}