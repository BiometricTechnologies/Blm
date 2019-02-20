#ifndef __fileEncryptor22_h__
#define __fileEncryptor22_h__

#include <string>
#include <vector>
#include <memory>
#include "IProgressReporter.h"
#include "common.h"
#include "encryptor.h"


namespace blm_utils{

class IFileEncryptor{
public:
	IFileEncryptor(){};
	virtual void encrypt(const std::wstring& srcPath, std::vector<std::uint8_t>& entropy, const std::wstring& destPath) = 0 /* throw() */;
	virtual void decrypt(const std::wstring& srcPath, std::vector<std::uint8_t>& entropy, const std::wstring& destPath) = 0 /* throw() */;
	virtual std::string encryptString(const std::string& stringToEncrypt) = 0 /* throw() */;
	virtual std::string decryptString(const std::string& stringToDecrypt) = 0/* throw() */;
	virtual std::string encryptStringWithKey(const std::string& originalString, const std::vector<std::uint8_t>& key) = 0 /* throw() */;
	virtual std::string decryptStringWithKey(const std::string& encryptedString, const std::vector<std::uint8_t>& key) = 0 /* throw() */;
	virtual ~IFileEncryptor() = 0;
private:
	IFileEncryptor& operator=(const IFileEncryptor&);
	IFileEncryptor(const IFileEncryptor&);
};

class BlockFileEncryptor : public IFileEncryptor, public IProgressReporter{
public:
	BlockFileEncryptor(encryptor::AbstractEncryptor* encryptor);
	virtual ~BlockFileEncryptor();
	virtual void encrypt(const std::wstring& srcPath, std::vector<std::uint8_t>& entropy, const std::wstring& destPath) /* throw() */;
	virtual void decrypt(const std::wstring& srcPath, std::vector<std::uint8_t>& entropy, const std::wstring& destPath) /* throw() */;
	virtual std::string encryptString(const std::string& stringToEncrypt) /* throw() */;
	virtual std::string decryptString(const std::string& stringToDecrypt) /* throw() */;
	virtual std::string encryptStringWithKey(const std::string& originalString, const std::vector<std::uint8_t>& key) /* throw() */;
	virtual std::string decryptStringWithKey(const std::string& encryptedString, const std::vector<std::uint8_t>& key) /* throw() */;
private:
	static const int64_t readBufferSize_;
	std::shared_ptr<encryptor::AbstractEncryptor> encryptor_;
	SeverityLogger logger_;

	void checkProgressReceiver() /* throw() */;
};

}

#endif // __fileEncryptor_h__