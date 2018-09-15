#include "StringConverter.h"

std::wstring StringConverter::StringToWide(std::string str){
	std::wstring wide_str(str.begin(), str.end());
	return wide_str;
}