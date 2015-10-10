#include "types.h"
#include <algorithm>
#include <cwctype>
#include <memory>
#include "windows.h"

int RoundCast(float rhs) {
	return static_cast<int>(rhs + 0.5f);
}

std::wstring ToUpper(const std::wstring& s) {
	std::wstring b(s);
	std::transform(b.begin(), b.end(), b.begin(), std::towupper);
	return b;
}

std::wstring ToLower(const std::wstring& s) {
	std::wstring b(s);
	std::transform(b.begin(), b.end(), b.begin(), std::towlower);
	return b;
}

std::wstring UTF8ToWString(const char* utf8) {
	int requiredBufferSizeInwchar_ts = MultiByteToWideChar(CP_UTF8, 0, utf8, static_cast<int>(strlen(utf8) + 1), 0, 0);

	std::wstring destinationBuffer(requiredBufferSizeInwchar_ts, 0);

	MultiByteToWideChar(CP_UTF8, 0, utf8, strlen(utf8), &destinationBuffer[0], requiredBufferSizeInwchar_ts);
	return destinationBuffer;
}

std::string WStringToUTF8(const std::wstring& utf16) {
	auto requiredBufferSize = WideCharToMultiByte(CP_UTF8, 0, &utf16[0], utf16.size(), nullptr, 0, nullptr, nullptr);

	std::string destinationBuffer(requiredBufferSize, 0);

	WideCharToMultiByte(CP_UTF8, 0, &utf16[0], utf16.size(), &destinationBuffer[0], requiredBufferSize, nullptr, nullptr);
	return destinationBuffer;
}

std::wstring ToWString( uint32_t i ) {
	std::wstringstream ss;
	ss << i;
	return ss.str();
}

std::wstring ToWString( int i ) {
	std::wstringstream ss;
	ss << i;
	return ss.str();
}

std::wstring ToWString( uint8_t i ) {
	std::wstringstream ss;
	ss << i;
	return ss.str();
}

std::wstring ToWString( const char* i ) {
	std::wstringstream ss;
	ss << i;
	return ss.str();
}

std::string ToAString(const std::wstring& s) {
	return std::string(s.begin(), s.end());
}

std::string ToAString(int i) {
	std::stringstream ss;
	ss << i;
	return ss.str();
}
