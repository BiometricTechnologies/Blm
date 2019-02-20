#ifndef __EncryptionKeyManager_h__
#define __EncryptionKeyManager_h__

#include <string>
#include <cstring>
#include <wchar.h>
#include "PlainDB.h"
#include "IEncryptionKeyManager.h"


class BiometricEncryptionKeyManager : public IEncryptionKeyManager {
public:
	BiometricEncryptionKeyManager(){};
	virtual ~BiometricEncryptionKeyManager(){};

	virtual int requestEncryptionKey(__in const wchar_t* biometricData, __in std::size_t biometricDataSize, __in const wchar_t* biometricType, __in std::size_t biometricTypeSize, __inout wchar_t* userName, __in std::size_t usernameBufferSize, __out std::uint8_t* keyData, __inout std::size_t* keyDataBufferSize);

	virtual int getMaxSIDSize(){
		return MAX_USR_NAME;
	}

	virtual int getMaxBiometricOwnerUserNameSize();
	virtual void getBiometricOwnerSid(__out wchar_t* SIDBuffer, __out std::size_t SIDBufferSize)
	{
		wcsncpy_s(SIDBuffer,SIDBufferSize, biometricOwnerSid.c_str(), biometricOwnerSid.size());
	}

	virtual bool isUserEnrolled(__in const wchar_t* SID, __in std::size_t SIDSize);

private:
	CPlainDB db_;
	std::vector<unsigned char> decodedBio_;
	std::wstring biometricType_;
	void decodeBiometric(const BSTR biometricData);
	EncryptionKeyRequestRet matchBiometricAndGetKey(__out std::uint8_t* keyData, __out size_t* keySize);
	std::vector<std::shared_ptr<blm_login::IFingerprintServices>> fpServicesAvaliable_;
	void loadProvidersPlugins();
	std::wstring biometricOwnerUserName;
	std::wstring biometricOwnerSid;
	// TODO: temp, uncomment when plug-ins become synchronized (an.skornyakov@gmail.com)
	//static const char* pluginsPath(){ return "System32\\IdentaZone\\IdentaMaster";}
	static const char* pluginsPath(){ return "System32\\IdentaZone\\IdentaMaster";}

	

};


class SimpleEncryptionKeyManager : public IEncryptionKeyManager {
public:
	SimpleEncryptionKeyManager();
	virtual ~SimpleEncryptionKeyManager(){};
	virtual int requestEncryptionKey(const BSTR biometricData, const BSTR biometricType, std::int32_t usernameBufferSize, __inout wchar_t* userName, __out char* keyData, __out size_t* keySize);
	virtual int getMaxBiometricOwnerUserNameSize();
private:
	CPlainDB db_;
	std::wstring getCurrentWinUserName();
};



class DummyEncryptionKeyManager : public IEncryptionKeyManager {
public:
	DummyEncryptionKeyManager(){};
	virtual ~DummyEncryptionKeyManager(){};
	virtual int requestEncryptionKey(const BSTR biometricData, const BSTR biometricType, std::int32_t usernameBufferSize, __inout wchar_t* userName, __out char* keyData, __out size_t* keySize);
	virtual int getMaxBiometricOwnerUserNameSize();

private:
	char* getKey(){
		return "Absolutely random encryption key";
	}
};




#endif // EncryptionKeyManager_h__
