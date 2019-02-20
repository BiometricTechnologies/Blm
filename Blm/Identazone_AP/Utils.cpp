#include <cwchar>
#include "Utils.h"

int findInMultiString(const wchar_t* mString, size_t totalSize, const wchar_t* pattern){
	auto nextStringBegin = 0;
	while(nextStringBegin < totalSize/sizeof(wchar_t)){
		auto res = wcscmp(mString+nextStringBegin, pattern);
		if(0==res){
			return nextStringBegin;
		} else {
			while(mString[nextStringBegin]!=L'\0' && nextStringBegin < totalSize/sizeof(wchar_t)){
				++nextStringBegin;
			}
			++nextStringBegin;
		}
	}
	return nextStringBegin;
}