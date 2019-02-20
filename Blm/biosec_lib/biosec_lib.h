#ifndef __biosec_lib_h__
#define __biosec_lib_h__

#include "stdafx.h"
#include <cstdint>

#define BIOSECDLL_EXPORTS
#ifdef BIOSECDLL_EXPORTS
#define BIOSECDLL_API __declspec(dllexport)
#else
#define BIOSECDLL_API __declspec(dllimport)
#endif

#pragma pack(push, 8)

enum EntityType{
	BIOSEC_FOLDER,
	BIOSEC_REGULAR
};


typedef struct archivePathInfo {
    wchar_t* pathName;
    uint64_t size;
    int64_t  mTime;
	uint64_t index;
	EntityType type;
};

typedef struct sourcePathInfo {
    wchar_t* pathName;
};

enum Mode {
	ENCRYPTION,
	DECRYPTION
};

// return codes (via params) for processing biometric callback
enum BiometricProcessingRes {
	BIOMETRIC_ACCEPTED,
	BIOMETRIC_MATCH_NOT_FOUND,
	BIOMETRIC_NOT_BELOGNS_TO_USER,
	BIOMETRIC_INVALID_ENCRYPTION_KEY,
	BIOMETRIC_INTERNAL_ERROR,
	BIOMETRIC_UNKNOWN_ERROR
};


enum RetCode{
	BIOSEC_NO_ERROR,
	BIOSEC_INDEX_OUT_OF_RANGE,
	BIOSEC_INTERNAL_ERROR,
	BIOSEC_LIB_NOT_INITED,
	BIOSEC_INVALID_ARG,
	BIOSEC_BUSY,
	BIOSEC_USER_NOT_ENROLLED,
	BIOSEC_USER_NOT_AN_ARCHIVE_OWNER,
	BIOSEC_UNKNOWN_ERROR
};


enum BiosecOperationErrorType{
	BIOSEC_ERRORTYPE_CRITICAL,
	BIOSEC_ERRORTYPE_ERROR
};


#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

	BIOSECDLL_API uint8_t __stdcall init(uint32_t mode);

	BIOSECDLL_API uint8_t __stdcall release();

    BIOSECDLL_API uint8_t __stdcall setSourcePathList(const sourcePathInfo* pathList, uint64_t size);

    BIOSECDLL_API uint8_t __stdcall setTargetDir(const wchar_t* path);

	BIOSECDLL_API uint8_t __stdcall getCryptoProviders(uint32_t index, wchar_t* buffer, uint32_t bufferLength);

	BIOSECDLL_API uint8_t __stdcall setCryptoProvider(const wchar_t* provider);

	typedef void (__stdcall  *biometricProcesses)(uint8_t);
	BIOSECDLL_API uint8_t __stdcall putBiometric(const wchar_t* biometricType, const uint8_t* data, uint64_t size, biometricProcesses fpBiometricProcessed);
	
	typedef void(__stdcall  *biometricProcesses)(uint8_t);
	BIOSECDLL_API uint8_t __stdcall putSecretKey(uint8_t* data, uint64_t size, biometricProcesses fpBiometricProcessed);

	typedef void(__stdcall  *biometricProcesses)(uint8_t);
	BIOSECDLL_API uint8_t __stdcall putSecretKeyForArchive(uint8_t* data, uint64_t size, biometricProcesses fpBiometricProcessed);

    BIOSECDLL_API uint8_t __stdcall setOverwriteFlagValue(bool needOverwrite);

	BIOSECDLL_API uint8_t __stdcall getUserName(wchar_t* name, uint64_t bufferLength);

	typedef void (__stdcall  *updateCallback)(const wchar_t*, const wchar_t*, uint32_t, uint32_t, uint32_t, uint32_t);
	typedef uint8_t (__stdcall  *finishCallback)(uint8_t, const wchar_t*);
	typedef void (__stdcall  *errorCallback)(BiosecOperationErrorType, const wchar_t*, const wchar_t*);
	
    BIOSECDLL_API uint8_t __stdcall startEncryption(updateCallback fpUpdate, finishCallback fpFinish, errorCallback fpError);
	BIOSECDLL_API uint8_t __stdcall startDecryption(updateCallback fpUpdate, finishCallback fpFinish, errorCallback fpError);

	BIOSECDLL_API uint8_t __stdcall getArchivePath(uint32_t index, archivePathInfo* pathInfo, uint64_t pathnameBufferSize);

    BIOSECDLL_API uint8_t __stdcall setSelectionPaths(const uint64_t* indices, uint64_t size);

	BIOSECDLL_API uint8_t __stdcall getEntriesInArchiveCount(uint64_t* count);


#ifdef __cplusplus
}
#endif



#endif // __biosec_lib_h__


