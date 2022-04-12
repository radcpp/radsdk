#ifndef RADCPP_STRING_H
#define RADCPP_STRING_H
#pragma once

#include "radcpp/Common/Common.h"

#include <string>
#include <string_view>
#include <vector>

std::vector<std::string> StrSplit(std::string_view str, std::string_view delimiters, bool skipEmptySubStr = false);

std::string StrFormat(const char* format, ...);
int StrFormatInPlace(std::string& buffer, const char* format, ...);
int StrFormatInPlaceArgList(std::string& buffer, const char* format, va_list args);

bool StrEqual(std::string_view str1, std::string_view str2);
bool StrCaseEqual(std::string_view str1, std::string_view str2);

std::string StrWideToU8(std::wstring_view wstr);
std::wstring StrU8ToWide(std::string_view str);

std::string StrUpper(std::string_view s);
std::string StrLower(std::string_view s);
void StrUpperInplace(std::string& s);
void StrLowerInplace(std::string& s);

void StrReplaceAll(std::string& str, std::string_view substr, std::string_view replaced);
bool StrIsDecInteger(std::string_view str);
bool StrIsHexNumber(std::string_view str);
bool StrIsBinNumber(std::string_view str);
// Check whether this string is a valid numeric string (a base 10 real number)
bool StrIsNumeric(std::string_view str);
bool IsDigit(char c);
bool IsHexDigit(char c);

void StrTrimLeading(std::string& str, std::string_view charlist = " \t\v\n\r\f");
void StrTrimTrailing(std::string& str, std::string_view charlist = " \t\v\n\r\f");
void StrTrim(std::string& str);

#endif // RADCPP_STRING_H