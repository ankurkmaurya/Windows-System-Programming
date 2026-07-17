
#include <utility.h>



// Helper function to safely manage memory lifetime
std::string Utility::ConvertPwstrToUtf8(wchar_t* wideStr) {
    if (!wideStr) return "";

    // 1. Calculate the required buffer size (including null terminator)
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);

    if (sizeNeeded <= 0) {
        return "";
    }

    // 2. Allocate space in a std::string
    std::string narrowStr(sizeNeeded - 1, '\0'); // -1 because std::string handles its own null terminator

    // 3. Perform the actual conversion
    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, &narrowStr[0], sizeNeeded, nullptr, nullptr);

    return narrowStr;
}





