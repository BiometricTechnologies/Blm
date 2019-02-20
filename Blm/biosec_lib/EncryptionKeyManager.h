#ifndef __EncryptionKeyManager_h__
#define __EncryptionKeyManager_h__

#include <string>
#include <cstring>
#include <windows.h>
#include "IEncryptionKeyManager.h"

#if 0
class DummyEncryptionKeyManager : public IEncryptionKeyManager {
public:
	DummyEncryptionKeyManager(){};
	virtual ~DummyEncryptionKeyManager(){};
	virtual int requestEncryptionKey(const BSTR biometricData, const BSTR biometricType, const wchar_t* userName, __out char* keyData, __out size_t* keySize);
	virtual int requestEncryptionKeyAsync(const BSTR biometricData, const BSTR biometricType, const wchar_t* userName, biometricDataAcquiredCb cb);

private:
	static char* getKey(){
		return "Absolutely random encryption key";
	}
};

int DummyEncryptionKeyManager::requestEncryptionKey( const BSTR biometricData, const BSTR biometricType, const wchar_t* userName, __out  char* keyData, __out size_t* keySize)
{
	/*
	if(!biometricData || !userName || !keyData || !keySize){
		return ENCRYPTION_KEY_REQUEST_NULLPTR_ARG;
	}*/
	
	std::string keyStr(getKey());
	std::copy(keyStr.begin(), keyStr.end(), keyData);
	*keySize = keyStr.size();
	
	return ENCRYPTION_KEY_REQUEST_SUCCESS;
}

int DummyEncryptionKeyManager::requestEncryptionKeyAsync( const BSTR biometricData, const BSTR biometricType, const wchar_t* userName, biometricDataAcquiredCb cb )
{
	return ENCRYPTION_KEY_REQUEST_NOT_IMPL_YET;
}

#endif

#endif // EncryptionKeyManager_h__
