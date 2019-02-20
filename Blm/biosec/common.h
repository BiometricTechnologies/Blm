#ifndef __common_h__
#define __common_h__

#include <windows.h>
#include "atlbase.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
// Log
#pragma warning(push)
#pragma warning(disable : 4251)
#include <glog/logging.h>
#pragma warning(pop)

namespace blm_utils{

class COInitializer{
public:
	COInitializer(){
		if (CoInitializeEx(NULL,COINIT_APARTMENTTHREADED) != S_OK)  //Needed to use any COM call in your application
			throw std::exception("Error in CoInitializeEx");
	}

	~COInitializer(){
		CoUninitialize(); //Uninitialized COM
	}
private:
	COInitializer(const COInitializer&);
	COInitializer& operator=(const COInitializer&);
};

class BstrWrapper{
public:
	BstrWrapper(const wchar_t* string = nullptr);
	~BstrWrapper();
	BSTR bstr();
	BSTR* bstrPtr();
private:
	BSTR managedBstr_;
};




bool checkFileExtension(const std::wstring &path, const std::wstring &extension);
}

#endif // __common_h__
