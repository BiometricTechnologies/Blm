#include "biometricDatabaseMaster.h"

__declspec(dllimport) void* dllGetEncryptionKeyQueryObject();

namespace blm_utils {


    IEncryptionKeyManager* BiometricDatabaseMaster::getEncryptionKeyManager() {
        if(!encKeyDylib_) {
			std::string err;
            encKeyDylib_ = std::shared_ptr<DynamicLibrary>(DynamicLibrary::load(this->APDllName(), err));
        }

        IEncryptionKeyManager* encryptionKeyManager = (reinterpret_cast<IEncryptionKeyManager*>(dllGetEncryptionKeyQueryObject()));
        return encryptionKeyManager;
    }


    BiometricDatabaseMaster::BiometricDatabaseMaster()
        : biometricProcessed_(false) {}


    bool BiometricDatabaseMaster::matchAgainstBiometric(const std::wstring &biometricType, const std::vector<uint8_t> biometricData) {
        auto encryptionKeyManager = this->getEncryptionKeyManager();
        biometricProcessed_ = false;

        // prepare data and buffers
        std::wstring biometricDataStr(reinterpret_cast<const wchar_t*>(biometricData.data()), biometricData.size() / sizeof(std::wstring::value_type));
        biometricOwner_.resize(encryptionKeyManager->getMaxBiometricOwnerUserNameSize());
        encryptionKey_.resize(MAX_ENC_KEY_BYTES);
        size_t keySize = encryptionKey_.size();

        auto ret = encryptionKeyManager->requestEncryptionKey(biometricDataStr.c_str(), biometricDataStr.size(),
                   biometricType.c_str(), biometricType.size(),
                   biometricOwner_.data(), biometricOwner_.size(),
                   encryptionKey_.data(), &keySize);

        switch(ret) {
            case ENCRYPTION_KEY_REQUEST_SUCCESS:
                encryptionKey_.resize(keySize);
				biometricProcessed_ = true;
				this->readBiometricOwnerSid();
                return true;
                break;
            case ENCRYPTION_KEY_REQUEST_NO_KEY_FOUND:
                return false;
                break;
            default:
                throw std::logic_error("internal error");
                break;
        }
    }


    void BiometricDatabaseMaster::checkIfBiometricProcessed() {
        if(!biometricProcessed_) {
            throw std::logic_error("biometric not processes");
        }
    }


    void BiometricDatabaseMaster::readBiometricOwnerSid() {
        this->checkIfBiometricProcessed();
        auto encryptionKeyManager = this->getEncryptionKeyManager();
        auto desiredBufferSize = encryptionKeyManager->getMaxSIDSize();
        biometricOwnerSid_.resize(desiredBufferSize);
		encryptionKeyManager->getBiometricOwnerSid(const_cast<wchar_t*>(biometricOwnerSid_.data()), biometricOwnerSid_.size());
		biometricOwnerSid_.resize(wcslen(biometricOwnerSid_.data()));
    }


    std::vector<wchar_t> BiometricDatabaseMaster::getBiometricOwnerName() { /* throw() */
        this->checkIfBiometricProcessed();
        return biometricOwner_;
    }


    std::vector<uint8_t> BiometricDatabaseMaster::getEncryptionKey() { /* throw() */
        this->checkIfBiometricProcessed();
        return encryptionKey_;
    }


    bool BiometricDatabaseMaster::isUserEnrolled(const std::wstring& userSid) {
        auto encryptionKeyManager = this->getEncryptionKeyManager();
        return encryptionKeyManager->isUserEnrolled(userSid.c_str(), userSid.size());
    }


    std::wstring BiometricDatabaseMaster::getBiometricOwnerSid() {
        this->checkIfBiometricProcessed();
        return biometricOwnerSid_;
    }

}

