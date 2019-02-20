#ifndef __EncryptionKeyRequest_h__
#define __EncryptionKeyRequest_h__

#include <windows.h>
#include <wtypes.h>
#include <cstdint>

#define MAX_ENC_KEY_BYTES 256

typedef enum BiometricDataType{
	FP_DP_IMG,
	FP_SG_IMG,
	FP_IB_IMG
};

typedef struct {
	BiometricDataType dataType;
	size_t size;
	char* data;
} UserBiometricData;

typedef enum EncryptionKeyRequestRet{
	ENCRYPTION_KEY_REQUEST_SUCCESS,
	ENCRYPTION_KEY_REQUEST_NO_KEY_FOUND,
	ENCRYPTION_KEY_REQUEST_NULLPTR_ARG,
	ENCRYPTION_KEY_REQUEST_NOT_IMPL_YET,
	ENCRYPTION_KEY_REQUEST_INTERNAL_ERR,
	ENCRYPTION_KEY_INSUFFICIAL_USERNAME_BUFFERSIZE
};


/// <summary>
/// Encryption key manager interface
/// </summary>
class IEncryptionKeyManager {
public:
	IEncryptionKeyManager(){};
	virtual ~IEncryptionKeyManager()=0{};
	virtual int requestEncryptionKey(__in const wchar_t* biometricData, __in std::size_t biometricDataSize,
		                             __in const wchar_t* biometricType, __in std::size_t biometricTypeSize,
									 __inout wchar_t* userName, __in std::size_t usernameBufferSize,
									 __out std::uint8_t* keyData, __inout std::size_t* keyDataBufferSize) = 0;
    
	virtual int getMaxBiometricOwnerUserNameSize() = 0;
	virtual int getMaxSIDSize() = 0;
	virtual void getBiometricOwnerSid(__out wchar_t* SIDBuffer, __out std::size_t SIDBufferSize) = 0;
	virtual bool isUserEnrolled(__in const wchar_t* SID, __in std::size_t SIDSize) = 0;
private:
	IEncryptionKeyManager(const IEncryptionKeyManager&);
	IEncryptionKeyManager& operator=(const IEncryptionKeyManager&);
};

#endif // EncryptionKeyRequest_h__
