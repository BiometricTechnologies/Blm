#include <windows.h>
#include <Lmcons.h>
#include "PluginManager.h"
#include "Path.h"
#include "fp_device.h"
#include "base64encoder.h"
#include "EncryptionKeyManager.h"
#include <sddl.h>
#pragma region RAII wrappers

class aprLibWrapper {
public:
    aprLibWrapper();
    ~aprLibWrapper();
private:
    aprLibWrapper(const aprLibWrapper &);
    aprLibWrapper &operator=(const aprLibWrapper &);
};

aprLibWrapper::aprLibWrapper() {
    ::apr_initialize();
}

aprLibWrapper::~aprLibWrapper() {
    // ::apr_terminate();
}

#pragma endregion RAII wrappers


apr_int32_t DummyInvokeService(const apr_byte_t* serviceName, void* serviceParams) {
    return 0;
}


static inline int base64SizePredicted(int originalSize) {
    return (originalSize / 3 + 1) * 4;
}

int SimpleEncryptionKeyManager::requestEncryptionKey(const BSTR biometricData, const BSTR biometricType, std::int32_t usernameBufferSize, __inout wchar_t* userName, __out char* keyData, __out size_t* keySize) {
    std::wstring winUserName = getCurrentWinUserName();

    db_.Init();
    db_.ResetUsername();
    wchar_t dbUSerName[MAX_USR_NAME];

    while(db_.GetUsername(dbUSerName, MAX_USR_NAME)) {
        int ret = wcscmp(dbUSerName, winUserName.c_str()); //.compare(dbUSerName);
        if(0 == ret) {
            std::vector<char> entropyBuf;
            if(!db_.GetBiosecEntropy(entropyBuf)) {
                return ENCRYPTION_KEY_REQUEST_INTERNAL_ERR;
            }

            if(entropyBuf.size() > MAX_ENC_KEY_BYTES) {
                return ENCRYPTION_KEY_REQUEST_INTERNAL_ERR;
            }

            std::copy(entropyBuf.begin(), entropyBuf.end(), keyData);
            *keySize = entropyBuf.size();
            return ENCRYPTION_KEY_REQUEST_SUCCESS;
        }
    }
    return ENCRYPTION_KEY_REQUEST_NO_KEY_FOUND;
}



std::wstring SimpleEncryptionKeyManager::getCurrentWinUserName() {
    wchar_t userName[UNLEN + 1];
    DWORD userNameLen = UNLEN + 1;
    GetUserName(userName, &userNameLen);
    return std::wstring(userName, userNameLen);
}

SimpleEncryptionKeyManager::SimpleEncryptionKeyManager() {

}

int SimpleEncryptionKeyManager::getMaxBiometricOwnerUserNameSize() {
    return UNLEN;
}


int DummyEncryptionKeyManager::requestEncryptionKey(const BSTR biometricData, const BSTR biometricType, std::int32_t usernameBufferSize, __inout wchar_t* userName, __out char* keyData, __out size_t* keySize) {
    /*
    if(!biometricData || !userName || !keyData || !keySize){
        return ENCRYPTION_KEY_REQUEST_NULLPTR_ARG;
    }*/

    std::string keyStr(getKey());
    std::copy(keyStr.begin(), keyStr.end(), keyData);
    *keySize = keyStr.size();

    return ENCRYPTION_KEY_REQUEST_SUCCESS;
}



int DummyEncryptionKeyManager::getMaxBiometricOwnerUserNameSize() {
    return 0;
}




void BiometricEncryptionKeyManager::loadProvidersPlugins() {
    aprLibWrapper aprLib;
    PluginManager &pm = PluginManager::getInstance();
    PF_ObjectParams objectParams;
    CHAR sysdir[MAX_PATH];
    GetSystemWindowsDirectoryA(sysdir, MAX_PATH);
    auto platformServices = pm.getPlatformServices();
    platformServices.invokeService = DummyInvokeService;
    pm.setPlatformServices(platformServices);
    objectParams.platformServices = &platformServices;
    pm.loadAll(Path::makeAbsolute(std::string(sysdir) + Path::sep + pluginsPath()));

    // get object(fpServices) registration map, create all available fingerprint services and collect them
    auto regMap = pm.getRegistrationMap();
    for(std::map<std::string, PF_RegisterParams>::iterator it = regMap.begin(); it != regMap.end(); ++it) {
        fpServicesAvaliable_.push_back(((it->second.createFunc(&objectParams))));
    }
}



int BiometricEncryptionKeyManager::requestEncryptionKey(__in const wchar_t* biometricData, __in std::size_t biometricDataSize, __in const wchar_t* biometricType, __in std::size_t biometricTypeSize, __inout wchar_t* userName, __in std::size_t usernameBufferSize, __out std::uint8_t* keyData, __inout std::size_t* keyDataBufferSize) {

    biometricType_.clear();
    biometricType_.assign(biometricType, biometricTypeSize);
    decodeBiometric(reinterpret_cast<BSTR>(const_cast<wchar_t*>(biometricData)));

    // load plugins
    //loadProvidersPlugins();
    aprLibWrapper aprLib;
    PluginManager &pm = PluginManager::getInstance();
    PF_ObjectParams objectParams;
    CHAR sysdir[MAX_PATH];
    GetSystemWindowsDirectoryA(sysdir, MAX_PATH);
    auto platformServices = pm.getPlatformServices();
    platformServices.invokeService = DummyInvokeService;
    pm.setPlatformServices(platformServices);
    objectParams.platformServices = &platformServices;

    pm.loadAll(Path::makeAbsolute(std::string(sysdir) + Path::sep + pluginsPath()));

    // get object(fpServices) registration map, create all available fingerprint services and collect them
    auto regMap = pm.getRegistrationMap();
    for(std::map<std::string, PF_RegisterParams>::iterator it = regMap.begin(); it != regMap.end(); ++it) {
        PF_ObjectParams objectParams;
        objectParams.platformServices = &platformServices;

        try {
            fpServicesAvaliable_.push_back(((it->second.createFunc(&objectParams))));
        } catch(const std::exception &ex) {
            // TODO: log an error here (an.skornyakov@gmail.com)
        }
    }

    auto res = matchBiometricAndGetKey(keyData, keyDataBufferSize);
    fpServicesAvaliable_.clear();
    pm.shutdown();

    if(ENCRYPTION_KEY_REQUEST_SUCCESS == res) {

        auto ret = wcsncpy_s(userName, usernameBufferSize, biometricOwnerUserName.c_str(), _TRUNCATE);

		/* bad old code
        if(usernameBufferSize >= biometricOwnerUserName.size()) {
            std::memcpy(userName, biometricOwnerUserName.c_str(), sizeof(wchar_t)*biometricOwnerUserName.size());
        } else {
            std::memcpy(userName, biometricOwnerUserName.c_str(), sizeof(wchar_t)*usernameBufferSize);
        }*/
    } else {
        if(usernameBufferSize > 0) {
            *userName = 0;
        }
    }

    return res;
}


void BiometricEncryptionKeyManager::decodeBiometric(const BSTR biometricData) {
    decodedBio_.resize(wcslen(biometricData)*sizeof(biometricData[0]));

    // NOTE: this code is neccessary cuz decodeBase64Str() works with C-string only
    std::wstring temp(biometricData);
    std::string bioDataStr;
    bioDataStr.assign(temp.begin(), temp.end());

	decodedBio_= blm_utils::base64_decode(bioDataStr);

    // NOTE: seems this 3rd party function returns size - 1
}

EncryptionKeyRequestRet BiometricEncryptionKeyManager::matchBiometricAndGetKey(__out std::uint8_t* keyData, __out size_t* keySize) {

    db_.Init();
    db_.ResetUsername();
    wchar_t dbUSerName[MAX_USR_NAME];
    wchar_t dbSID[MAX_USR_NAME];

    while(db_.GetUsername(dbUSerName, MAX_USR_NAME)) {
        db_.GetSID(dbSID, MAX_USR_NAME);
        std::vector<std::shared_ptr<blm_login::FingerprintTemplate>> templateVector;
        int result = db_.GetFmt(templateVector);
        if(result > 0) {
            // match template vector for each user against all available devices plugins
            for(auto it = std::begin(fpServicesAvaliable_); it != std::end(fpServicesAvaliable_); ++it) {
                blm_login::FingerprintImage fpImage;
                if(blm_login::Result::Success == (*it)->deserialize(biometricType_, decodedBio_, &fpImage)) {
                    if((*it)->matchTemplates(fpImage, templateVector)) {
                        // obtain entropy
                        std::vector<char> entropyBuf;
                        if(!db_.GetBiosecEntropy(entropyBuf)) {
                            return ENCRYPTION_KEY_REQUEST_INTERNAL_ERR;
                        }
                        if(entropyBuf.size() > MAX_ENC_KEY_BYTES) {
                            return ENCRYPTION_KEY_REQUEST_INTERNAL_ERR;
                        }
                        std::copy(entropyBuf.begin(), entropyBuf.end(), keyData);
                        *keySize = entropyBuf.size();
                        biometricOwnerUserName = dbUSerName;
                        biometricOwnerSid = dbSID;
                        return ENCRYPTION_KEY_REQUEST_SUCCESS;
                    }
                }
            }
        } else { // no fingerprint for such user
            // TODO: log (an.skornyakov@gmail.com)
        }
    }

    return ENCRYPTION_KEY_REQUEST_NO_KEY_FOUND;
}

int BiometricEncryptionKeyManager::getMaxBiometricOwnerUserNameSize() {
    return MAX_USR_NAME;
}


bool BiometricEncryptionKeyManager::isUserEnrolled(__in const wchar_t* SID, __in std::size_t SIDSize) {
    std::wstring SIDStr(SID, SIDSize);
    db_.Init();
    db_.ResetUsername();
    wchar_t dbUSerName[MAX_USR_NAME];
    wchar_t dbSID[MAX_USR_NAME];

    while(db_.GetUsername(dbUSerName, MAX_USR_NAME)) {
        db_.GetSID(dbSID, MAX_USR_NAME);
        if(!SIDStr.compare(dbSID)) {
            std::vector<std::shared_ptr<blm_login::FingerprintTemplate>> templateVector;
            int result = db_.GetFmt(templateVector);
            if(result > 0) {
                return true;
            } else { // no fingerprint for such user
                // TODO: log (an.skornyakov@gmail.com)
                return false;
            }
        }
    }
    return false;
}

