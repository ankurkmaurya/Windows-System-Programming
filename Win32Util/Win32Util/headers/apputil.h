#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>


namespace AppUtil {

	LPWSTR convertUTF8ToLPWSTR(char* utf8chars);

	bool isFileExists(const std::string& filePath);
}