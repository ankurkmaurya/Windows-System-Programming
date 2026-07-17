#pragma once

#include <windows.h>
#include <string>

class Utility {

public:
	static std::string ConvertPwstrToUtf8(wchar_t* wideStr);


};

