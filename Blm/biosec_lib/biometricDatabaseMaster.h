#ifndef __biometricDatabaseMaster_h__
#define __biometricDatabaseMaster_h__

#include <vector>
#include <memory>
#include <cstdint>
#include "boost/noncopyable.hpp"
#include "DynamicLibrary.h"
#include "IEncryptionKeyManager.h"

namespace blm_utils{
	
	class BiometricDatabaseMaster : boost::noncopyable {
	public:
		BiometricDatabaseMaster();
		
		bool isUserEnrolled(const std::wstring& userSid);
		bool matchAgainstBiometric(const std::wstring& biometricType, const std::vector<uint8_t> biometricData);
		std::vector<uint8_t> getEncryptionKey() /* throw() */;;
		std::vector<wchar_t> getBiometricOwnerName() /* throw() */;
		std::wstring getBiometricOwnerSid();

	private:
		static const char* APDllName() {
			return "IdentaZonAP.dll";
		}

		bool biometricProcessed_;

		std::shared_ptr<DynamicLibrary> encKeyDylib_;
		std::vector<uint8_t> biometricData_;
		std::wstring biometricType_;
		std::vector<uint8_t> encryptionKey_;
		std::vector<wchar_t> biometricOwner_;
		std::wstring biometricOwnerSid_;

		void checkIfBiometricProcessed();
		void readBiometricOwnerSid();

		IEncryptionKeyManager* getEncryptionKeyManager();
	};

}

#endif // __biometricDatabaseMaster_h__
