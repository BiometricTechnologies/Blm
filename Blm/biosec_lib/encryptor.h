#pragma once

#include "windows.h"
#include "Wincrypt.h"
#include <cstdint>
#include <vector>

namespace encryptor{

    class AbstractEncryptor{
    public:
        virtual void encrypt(DATA_BLOB* data, DATA_BLOB* entropy, DATA_BLOB* encryptedData) = 0;
        virtual void decrypt(DATA_BLOB* encryptedData, DATA_BLOB* entropy, DATA_BLOB* data) = 0;
        virtual ~AbstractEncryptor();
    };

    class DpapiEncryptor : public AbstractEncryptor{
    public:
        virtual void encrypt(DATA_BLOB* data, DATA_BLOB* entropy, DATA_BLOB* encryptedData);
        virtual void decrypt(DATA_BLOB* encryptedData, DATA_BLOB* entropy, DATA_BLOB* data);
    };

    enum AesKeySize{
        AES_128,
        AES_192,
        AES_256
    };


    class OpenAESEncryptor : public AbstractEncryptor{
    private:
        uint32_t keySize_;
    public:
        OpenAESEncryptor(AesKeySize keySize = AES_128);
        virtual void encrypt(DATA_BLOB* data, DATA_BLOB* entropy, DATA_BLOB* encryptedData);
        virtual void decrypt(DATA_BLOB* encryptedData, DATA_BLOB* entropy, DATA_BLOB* data);
    };

#define KEYSIZE_128 (128/8)
#define KEYSIZE_192 (192/8)
#define KEYSIZE_256 (256/8)

    struct AesKey
    {
        BLOBHEADER Header;
        DWORD dwKeyLength;
        // Set to max possible key size
        BYTE cbKey[KEYSIZE_256];

        AesKey() {
            ZeroMemory( this, sizeof(AesKey) );
            Header.bType = PLAINTEXTKEYBLOB;
            Header.bVersion = CUR_BLOB_VERSION;
            Header.reserved = 0;
        }

        ~AesKey() {                
            SecureZeroMemory( this, sizeof(AesKey) );
        }
    };

    // Winapi based encryptor, copying is forbidded for class
    class WinapiEncryptor : public AbstractEncryptor{
    public:
        explicit WinapiEncryptor(AesKeySize keySize);
        virtual ~WinapiEncryptor();
        virtual void encrypt(DATA_BLOB* data, DATA_BLOB* entropy, DATA_BLOB* encryptedData);
        virtual void decrypt(DATA_BLOB* encryptedData, DATA_BLOB* entropy, DATA_BLOB* data);
    private:
        uint32_t keySize_;
        AesKey aesKey_;
        std::vector<unsigned char> iv_;
        WinapiEncryptor(const WinapiEncryptor& orig);
        WinapiEncryptor& operator=(const WinapiEncryptor& rhs);
        void createAesKey(DATA_BLOB* entropy);
    };



}