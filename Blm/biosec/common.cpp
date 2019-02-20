#include <boost/filesystem.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include "common.h"

namespace blm_utils{

BstrWrapper::BstrWrapper( const wchar_t* string ){
	if(string){
		managedBstr_ = SysAllocString(string);
	}
}
BstrWrapper::~BstrWrapper(){
	if(managedBstr_){
		SysFreeString(managedBstr_); // Free the created string
	}	
}

BSTR BstrWrapper::bstr(){
	return managedBstr_;
}

BSTR* BstrWrapper::bstrPtr(){
	return &managedBstr_;
}

/// <summary>
/// Checks the path extension with extension specified
/// </summary>
/// <param name="path">The path.</param>
/// <param name="extension">The extension.</param>
/// <returns></returns>
bool checkFileExtension(const std::wstring &path, const std::wstring &extension) {

    boost::filesystem::path p(path);
    if(!p.has_extension()) {
        return false;
    } else {
		auto ext = p.extension().wstring();
        auto compareRes = ext.compare(extension);
		return (0==compareRes);
    }
}

}

