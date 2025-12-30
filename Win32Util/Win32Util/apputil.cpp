
#include "apputil.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

#include <windows.h>


namespace AppUtil {

	LPWSTR convertUTF8ToLPWSTR(char* utf8chars) {
		if (!utf8chars) return nullptr;

		// Get required buffer size
		int sizeNeeded = MultiByteToWideChar(
			CP_UTF8,            // or CP_ACP for ANSI
			0,
			utf8chars,
			-1,
			nullptr,
			0
		);
		if (sizeNeeded == 0) return nullptr;

		// Allocate buffer (caller must free this later using delete[])
		LPWSTR result = new WCHAR[sizeNeeded];

		// Perform conversion
		MultiByteToWideChar(
			CP_UTF8,
			0,
			utf8chars,
			-1,
			result,
			sizeNeeded
		);
		return result;
	}

	bool isFileExists(const std::string& filePath) {
		return std::filesystem::exists(filePath);
	}

}