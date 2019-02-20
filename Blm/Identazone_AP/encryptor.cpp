#pragma comment(lib, "crypt32.lib")

#include <cstdint>
#include <exception>
#include <stdexcept>
#include <vector>
#include "oaes_lib.h"
#include "encryptor.h"

using namespace std;

namespace encryptor {


    const uint32_t internalKeySize = 128;



    AbstractEncryptor::~AbstractEncryptor(){
    }

    void DpapiEncryptor::encrypt(DATA_BLOB* data, DATA_BLOB* encryptedData, DATA_BLOB* entropy){

        if(!CryptProtectData(
            data,
            NULL, 
            entropy,                               
            NULL, 
            NULL,
            0,
            encryptedData)){
                throw std::exception("Encryption error using CryptProtectData.\n");
        }

    }

    void DpapiEncryptor::decrypt(DATA_BLOB* data, DATA_BLOB* encryptedData, DATA_BLOB* entropy){

        if (!CryptUnprotectData(
            encryptedData,
            NULL,
            entropy,                 
            NULL,                 
            NULL,                 
            0,
            data)){
                throw std::exception("Decryption error using CryptUnprotectData.\n");
        }
    }

#if 0
    OpenAESEncryptor::OpenAESEncryptor( AesKeySize keySize){
        switch(keySize){
        case AES_128:
            keySize_ = 128;
            break;
        case AES_192:
            throw std::invalid_argument("Invalid key size argument.");
            //keySize_ = 192;
            break;
        case AES_256:
            throw std::invalid_argument("Invalid key size argument.");
            //keySize_ = 256;
            break;
        default:
            throw std::invalid_argument("Invalid key size argument.");
        }
    }

    void OpenAESEncryptor::encrypt( DATA_BLOB* data, DATA_BLOB* entropy, DATA_BLOB* encryptedData ){
    }

    /**
    * OpenAES-based decryption 
    * encryptedData IN 
    * entropy IN e.g. password
    * data OUT decryptedData
    */
    void OpenAESEncryptor::decrypt( DATA_BLOB* encryptedData, DATA_BLOB* entropy, DATA_BLOB* data ){
        // Initialization vector for AES
        unsigned char iv[16] = { 0x46, 0xb6, 0x02, 0x6a, 0x99, 0x21, 0x90, 0xde, 0xfd, 0xf4, 0x5b, 0x42, 0x94, 0xde, 0xa6, 0x23 };
        // +Alloc memory
        OAES_CTX * ctx = oaes_alloc();
        // setup encryption options - CBC, iv, key
        oaes_set_option(ctx, OAES_OPTION_CBC, iv);
        oaes_key_import_data( ctx, entropy->pbData, entropy->cbData);

        // OpenAES requires data to have special header, to raw data we need to recreate this header
        // Also we need to create temporary buffers for data
        DATA_BLOB tmpIn;
        DATA_BLOB tmpOut;
        // prepare tmp buffer for input data w/ false header
        tmpIn.cbData= encryptedData->cbData + OAES_BLOCK_SIZE*2;
        tmpIn.pbData = new uint8_t[tmpIn.cbData];
        // move raw data to tmp buffer, size of Header is OAES_BLOCK_SIZE*2 (for current block size)
        memcpy(tmpIn.pbData + OAES_BLOCK_SIZE*2, encryptedData->pbData, encryptedData->cbData);
        // create "false" header
        memcpy(tmpIn.pbData, "OAES", 4);
        tmpIn.pbData[4] = 0x1;
        tmpIn.pbData[5] = 0x2;
        OAES_OPTION options = OAES_OPTION_CBC;
        memcpy(&tmpIn.pbData[6], &options, sizeof(options));
        tmpIn.pbData[8] = 0x1; //Flag pad
        // Initializaion vector is also included in header
        memcpy(&tmpIn.pbData[OAES_BLOCK_SIZE], iv, OAES_BLOCK_SIZE);
        // preapare tmp buffer for output data
        tmpOut.cbData = tmpIn.cbData;
        tmpOut.pbData = new uint8_t[tmpOut.cbData];
        // decrypt data
        oaes_decrypt( ctx, tmpIn.pbData, tmpIn.cbData, tmpOut.pbData,(size_t*) &tmpOut.cbData);
        // move decrypted data from tmp buffer to output buffer
        memcpy(data->pbData,tmpOut.pbData,tmpOut.cbData);
        // set output data length
        data->cbData = tmpOut.cbData;
        // delete tmp buffers
        delete tmpIn.pbData;
        delete tmpOut.pbData;
        // free encryption structure
        oaes_free( &ctx );
    }
#endif

    void WinapiEncryptor::encrypt( DATA_BLOB* plainData, DATA_BLOB* entropy, DATA_BLOB* encryptedData ){

        // Create the crypto provider context. This can be implemented
        HCRYPTPROV hProvider = NULL;
        if (!CryptAcquireContext(&hProvider,
            NULL,  // pszContainer = no named container
            MS_ENH_RSA_AES_PROV,  // pszProvider
            PROV_RSA_AES,
            CRYPT_VERIFYCONTEXT)) {
                throw std::runtime_error("Unable to create crypto provider context.");
        }       

        createAesKey(entropy);
        const unsigned structSize = sizeof(aesKey_) - KEYSIZE_256 + keySize_;

        HCRYPTKEY hKey;
        // Import AES key
        if(!CryptImportKey(hProvider, (CONST BYTE*)&aesKey_, structSize, NULL, 0, &hKey ) )
        {
            throw std::runtime_error("Unable to create key.");
        }

        // set key params
        DWORD dwMode = CRYPT_MODE_CBC;
        if(!CryptSetKeyParam( hKey, KP_MODE, (BYTE*)&dwMode, 0 ))
        {
            throw std::runtime_error("Unable to set key params.");
        }

        if(!CryptSetKeyParam(
            hKey,
            KP_IV,
            iv_.data(),
            0
            )){
                throw std::runtime_error("Unable to set key params.");
        }

        // data prepare 
        std::vector<BYTE>tempBufferHoldingVec(plainData->cbData); // RAII
        DATA_BLOB tmpData;
        tmpData.cbData = plainData->cbData;
        tmpData.pbData = tempBufferHoldingVec.data();
        memcpy(tmpData.pbData, plainData->pbData, plainData->cbData);

        if(!CryptEncrypt(
            hKey,
            NULL,
            false,
            0, // flags
            tmpData.pbData, // plaintext buffer, overwritten
            &tmpData.cbData, // plaintext length, overwritten
            tmpData.cbData  // total size of buffer
            )){
                throw std::runtime_error("Encryption error.");
        }

        if(!CryptDestroyKey(hKey))
            throw std::runtime_error("Unable to destroy crypto key.");

        if (!CryptReleaseContext(hProvider,0)){
            throw std::runtime_error("Unable to release context.");
        }

        encryptedData->cbData = tmpData.cbData;
        memcpy(encryptedData->pbData, tmpData.pbData, tmpData.cbData);
        //delete[] tmpData.pbData;
    }

    // when we decide to support 192 and 256 bit length keys - just uncomment corresponding rows in this block
    WinapiEncryptor::WinapiEncryptor(AesKeySize keySize){
        switch(keySize){
        case AES_128:
            keySize_ = 128/8;
            break;
        case AES_192:
            throw std::invalid_argument("Invalid key size argument.");
            //keySize_ = 192/8;
            break;
        case AES_256:
            throw std::invalid_argument("Invalid key size argument.");
            //keySize_ = 256/8;
            break;
        default:
            throw std::invalid_argument("Invalid key size argument.");
        }

        // default IV
        unsigned char iv[16] = { 0x46, 0xb6, 0x02, 0x6a, 0x99, 0x21, 0x90, 0xde, 0xfd, 0xf4, 0x5b, 0x42, 0x94, 0xde, 0xa6, 0x23 };
        iv_.assign(std::begin(iv), std::end(iv));
    }

    void WinapiEncryptor::decrypt( DATA_BLOB* encryptedData, DATA_BLOB* entropy, DATA_BLOB* plainData ){
        // Create the crypto provider context. This can be implemented
        HCRYPTPROV hProvider = NULL;
        if (!CryptAcquireContext(&hProvider,
            NULL,  // pszContainer = no named container
            MS_ENH_RSA_AES_PROV,  // pszProvider
            PROV_RSA_AES,
            CRYPT_VERIFYCONTEXT)) {
                throw std::runtime_error("Unable to create crypto provider context.");
        }  

        createAesKey(entropy);
        const unsigned structSize = sizeof(aesKey_) - KEYSIZE_256 + keySize_;

        HCRYPTKEY hKey;
        // Import AES key
        if(!CryptImportKey(hProvider, (CONST BYTE*)&aesKey_, structSize, NULL, 0, &hKey ) )
        {
            throw std::runtime_error("Unable to create key.");
        }

        // set key params
        DWORD dwMode = CRYPT_MODE_CBC;
        if(!CryptSetKeyParam( hKey, KP_MODE, (BYTE*)&dwMode, 0 ))
        {
            throw std::runtime_error("Unable to set key params.");
        }

        
        if(!CryptSetKeyParam(
            hKey,
            KP_IV,
            iv_.data(),
            0
            )){
                throw std::runtime_error("Unable to set key params.");
        }

        // data prepare 
        std::vector<BYTE>tempBufferHoldingVec(encryptedData->cbData); // RAII
        DATA_BLOB tmpData;
        tmpData.cbData = encryptedData->cbData;
        tmpData.pbData = tempBufferHoldingVec.data();
        memcpy(tmpData.pbData, encryptedData->pbData, encryptedData->cbData);

        if(!CryptDecrypt(
            hKey,
            NULL,
            false,
            0, // flags
            tmpData.pbData, // plaintext buffer, overwritten
            &tmpData.cbData // plaintext length, overwritten
            )){
                throw std::runtime_error("Encryption error.");
        }

        if(!CryptDestroyKey(hKey))
            throw std::runtime_error("Unable to destroy crypto key.");

        if (!CryptReleaseContext(hProvider,0))        {
            throw std::runtime_error("Unable to release context.");
        }

        plainData->cbData = tmpData.cbData;
        memcpy(plainData->pbData, tmpData.pbData, tmpData.cbData);

    }

    void WinapiEncryptor::createAesKey(DATA_BLOB* entropy){
        // when we decide to support 192 and 256 bit length keys - just uncomment corresponding rows in this block
        switch( keySize_ ){
        case KEYSIZE_128:
            aesKey_.Header.aiKeyAlg = CALG_AES_128;
            aesKey_.dwKeyLength = KEYSIZE_128;
            break;
        case KEYSIZE_192:
            throw std::runtime_error("Invalid key size.");
            //aesKey_.Header.aiKeyAlg = CALG_AES_192;
            //aesKey_.dwKeyLength = KEYSIZE_192;
            break;
        case KEYSIZE_256:
            throw std::runtime_error("Invalid key size.");
            //aesKey_.Header.aiKeyAlg = CALG_AES_256;
            //aesKey_.dwKeyLength = KEYSIZE_256;
            break;
        default:
            throw std::runtime_error("Invalid key size.");
        }
        memcpy_s(aesKey_.cbKey, aesKey_.dwKeyLength, entropy->pbData, keySize_);
    }

    WinapiEncryptor::~WinapiEncryptor(){

    }

}