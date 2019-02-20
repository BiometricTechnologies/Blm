#include <wchar.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include "windows.h"
#include "fileEncryptor.h"
#include <boost/filesystem.hpp>
#include "common.h"


namespace blm_utils {

    const int64_t BlockFileEncryptor::readBufferSize_ = 0x4000;

    static void makeFileHidden(const std::wstring &path) {

        auto attr = GetFileAttributes(path.c_str());
        if(INVALID_FILE_ATTRIBUTES == attr) {
			//logger_.log(SeverityLevel::ERR, "error reading file attributes");
            return;
        }

        attr |= FILE_ATTRIBUTE_HIDDEN;
        if(!SetFileAttributes(path.c_str(), attr)) {

            //if(!SetFileAttributes(path.c_str(), FILE_ATTRIBUTE_HIDDEN)){
            //LOG(ERROR) << "makeFileHidden[Error]:  error setting file flags, error " << GetLastError();
        }
    }

    static void resetFileFlags(const std::wstring &path, DWORD flags) {

        auto attr = GetFileAttributes(path.c_str());
        if(INVALID_FILE_ATTRIBUTES == attr) {
            //LOG(INFO) << "resetFileFlags[error]: error reading file attributes";
            return;
        }

        attr &= ~flags;
        if(!SetFileAttributes(path.c_str(), attr)) {
           // LOG(ERROR) << "resetFileFlags[Error]: error reseting file flags, error " << GetLastError();
        }
    }

    /// <summary>
    /// Encrypts the specified source path.
    /// </summary>
    /// <param name="srcPath">The source path.</param>
    /// <param name="entropy">The entropy.</param>
    /// <param name="destPath">The dest path.</param>
    void blm_utils::BlockFileEncryptor::encrypt(const std::wstring &srcPath, std::vector<std::uint8_t> &entropy, const std::wstring &destPath) { /* throw() */
		checkProgressReceiver();

        int fdRead = 0;
        auto ret = _wsopen_s(&fdRead, srcPath.c_str(),  _O_RDONLY | _O_BINARY, _SH_DENYRW, _S_IREAD);
        if(fdRead < 0 || 0 != ret) {
            // TODO: RAII (an.skornyakov@gmail.com)
            throw std::exception("Exception opening file to encrypt");
        }

        int fdWrite = 0;
        ret = _wsopen_s(&fdWrite, destPath.c_str(),  _O_WRONLY | _O_CREAT | _O_BINARY, _SH_DENYRW, _S_IWRITE);
        if(fdWrite < 0 || 0 != ret) {
            _close(fdRead);
            throw std::exception("Exception creating temp file to write encryption result");
        }
        _close(fdWrite);

        makeFileHidden(destPath);

        ret = _wsopen_s(&fdWrite, destPath.c_str(),  _O_WRONLY | _O_BINARY, _SH_DENYRW, _S_IWRITE);
        if(fdWrite < 0 || 0 != ret) {
            _close(fdRead);
            throw std::exception("Exception opening temp file to write encryption result");
        }

        // write source file size in destination file header
        int64_t fSize = _lseeki64(fdRead, 0, SEEK_END);
        _lseeki64(fdRead, 0, SEEK_SET);
        _write(fdWrite, &fSize, sizeof(fSize));

        auto progressReceiver = getprogressReceiver();
        int64_t bytesDone = 0;
        progressReceiver->reportProgress(this, bytesDone, fSize);

        try {
            // prepare buffers, DATA_BLOBs, then read source file block by block,
            // encrypt and write in dest file
            std::vector<char> readBuff; // use vector as static array
            readBuff.resize(readBufferSize_);
            auto len = _read(fdRead, readBuff.data(), readBuff.size());
            DATA_BLOB entropyBlob;
            entropyBlob.cbData = entropy.size();
            entropyBlob.pbData = reinterpret_cast<BYTE*>(entropy.data());

            while(len > 0) {
                // prepare DATA_BLOBS for encryptor
                DATA_BLOB inputData;
                inputData.cbData = readBuff.size();
                inputData.pbData = reinterpret_cast<BYTE*>(readBuff.data());
                DATA_BLOB outputData;

                try {
                    encryptor_->encrypt(&inputData, &entropyBlob, &outputData);
                } catch(const std::exception  &ex) {
                    //LOG(ERROR) << ex.what();
                    LocalFree(outputData.pbData);
                    throw;
                }
                _write(fdWrite, outputData.pbData, outputData.cbData);
                bytesDone += len;
                //DLOG(INFO) << "Decrypting process: " << bytesDone << " from " << fSize;
                progressReceiver->reportProgress(this, bytesDone, fSize);
                len = _read(fdRead, readBuff.data(), readBuff.size());
                LocalFree(outputData.pbData);
            }
        } catch(const std::exception  &ex) {
            //LOG(ERROR) << ex.what();
            _close(fdRead);
            _close(fdWrite);
            throw;
        }
        _close(fdRead);
        _close(fdWrite);
    }


    /// <summary>
    /// Decrypts the specified source path.
    /// </summary>
    /// <param name="srcPath">The source path.</param>
    /// <param name="entropy">The entropy.</param>
    /// <param name="destPath">The dest path.</param>
    void blm_utils::BlockFileEncryptor::decrypt(const std::wstring &srcPath, std::vector<std::uint8_t> &entropy, const std::wstring &destPath) { /* throw() */
        checkProgressReceiver();

		int fdRead = 0;
		auto ret = _wsopen_s(&fdRead, srcPath.c_str(),  _O_RDONLY | _O_BINARY, _SH_DENYRW, _S_IREAD);
        if(fdRead < 0 || 0 != ret) {
            // TODO: RAII (an.skornyakov@gmail.com)			
            throw std::exception("Exception opening file to decrypt ");
        }
        auto mTime = boost::filesystem::last_write_time(srcPath);
        int fdWrite = 0 ;
		ret = _wsopen_s(&fdWrite, destPath.c_str(),  _O_WRONLY | _O_BINARY | _O_CREAT, _SH_DENYRW, _S_IWRITE);
        if(fdWrite < 0 || 0!=ret) {
            _close(fdRead);
            throw std::exception("Exception opening file to write decryption result");
        }
        //close file to reset its flags
        _close(fdWrite);

        // remove readonly flag
        resetFileFlags(destPath, FILE_ATTRIBUTE_READONLY);
        makeFileHidden(srcPath);

		ret = _wsopen_s(&fdWrite, destPath.c_str(),  _O_WRONLY | _O_BINARY | _O_CREAT, _SH_DENYRW, _S_IWRITE);
        if(fdWrite < 0 || 0 != ret) {
            _close(fdRead);
            throw std::exception("Exception opening file to write decryption result");
        }

        // read size header from source file
        int64_t fSize = 0;
        _read(fdRead, &fSize, sizeof(fSize));
        auto origFileSize = fSize;


        auto progressReceiver = getprogressReceiver();
        int64_t bytesDone = 0;

        // TODO: refactor (an.skornyakov@gmail.com)
        auto MACSizeWin = 0xe6; // DPAPI encryptor MAC size

        try {
            // prepare buffers, DATA_BLOBs, then read source file block by block,
            // decrypt and write in dest file
            std::vector<char> readBuff; // use vector as static array
            readBuff.resize(readBufferSize_ + MACSizeWin);
            auto len = _read(fdRead, readBuff.data(), readBuff.size());

            DATA_BLOB entropyBlob;
            entropyBlob.cbData = entropy.size();
            entropyBlob.pbData = reinterpret_cast<BYTE*>(entropy.data());

            while(len > 0) {
                // prepare DATA_BLOBS for encryptor
                DATA_BLOB inputData;
                inputData.cbData = readBuff.size();
                inputData.pbData = reinterpret_cast<BYTE*>(readBuff.data());
                DATA_BLOB outputData;

                try {
                    encryptor_->decrypt(&inputData, &entropyBlob, &outputData);
                } catch(const std::exception  &ex) {
                    //LOG(ERROR) << ex.what();
                    LocalFree(outputData.pbData);
                    throw;
                }

                auto dataSizeToWrite = fSize;
                if(fSize >= readBufferSize_) {
                    fSize -= (readBufferSize_);
                    dataSizeToWrite = outputData.cbData;
                }

                _write(fdWrite, outputData.pbData, dataSizeToWrite);
                bytesDone += dataSizeToWrite;
                //DLOG(INFO) << "Decrypting process: " << bytesDone << " from " << fSize;
                progressReceiver->reportProgress(this, bytesDone, origFileSize);

                len = _read(fdRead, readBuff.data(), readBuff.size());
                LocalFree(outputData.pbData);

            }
        } catch(const std::exception  &ex) {
            //LOG(ERROR) << ex.what();
            _close(fdRead);
            _close(fdWrite);
            throw;
        }
        _close(fdRead);
        _close(fdWrite);
        boost::filesystem::last_write_time(destPath, mTime);
    }

    blm_utils::BlockFileEncryptor::BlockFileEncryptor(encryptor::AbstractEncryptor* encryptor)
        : encryptor_(encryptor) {

    }

    blm_utils::BlockFileEncryptor::~BlockFileEncryptor() {

    }

    IFileEncryptor::~IFileEncryptor() {

    }


    void BlockFileEncryptor::checkProgressReceiver() { /* throw() */
        if(IProgressReporter::checkProgressReceiver()) {
            throw std::exception("[BlockFileEncryptor::checkProgressReceiver] error progress receiver is expired");
        }
    }

    std::string BlockFileEncryptor::encryptString(const std::string &stringToEncrypt) { /* throw() */
        DATA_BLOB inputData;
        inputData.cbData = stringToEncrypt.size();
        inputData.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(stringToEncrypt.c_str()));
        DATA_BLOB outputData;
        std::unique_ptr<encryptor::AbstractEncryptor> encryptor(new encryptor::DpapiEncryptor);
        encryptor->encrypt(&inputData, nullptr, &outputData);


        try {
            std::string encryptedString(reinterpret_cast<char*>(outputData.pbData), outputData.cbData);
            LocalFree(outputData.pbData);
            return encryptedString;
        } catch(const std::exception &ex) {
            LocalFree(outputData.pbData);
            //LOG(ERROR) << "[BlockFileEncryptor::encryptString] error " << ex.what();
        }


    }

    std::string BlockFileEncryptor::decryptString(const std::string &stringToDecrypt) { /* throw() */
        DATA_BLOB inputData;
        inputData.cbData = stringToDecrypt.size();
        inputData.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(stringToDecrypt.c_str()));
        DATA_BLOB outputData;
        std::unique_ptr<encryptor::AbstractEncryptor> encryptor(new encryptor::DpapiEncryptor);
        encryptor->decrypt(&inputData, nullptr, &outputData);

        try {
            std::string decryptedString(reinterpret_cast<char*>(outputData.pbData), outputData.cbData);
            LocalFree(outputData.pbData);
            return decryptedString;
        } catch(const std::exception &ex) {
            //LOG(ERROR) << "[BlockFileEncryptor::encryptString] error " << ex.what();
            LocalFree(outputData.pbData);
            throw;
        }


    }

    std::string BlockFileEncryptor::encryptStringWithKey(const std::string &originalString, const std::vector<std::uint8_t> &key) { /* throw() */
        /* throw() */

        DATA_BLOB inputData;
        inputData.cbData = originalString.size();
        inputData.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(originalString.c_str()));

        DATA_BLOB entropy;
        entropy.cbData = key.size();
        entropy.pbData = reinterpret_cast<BYTE*>(const_cast<std::uint8_t*>(key.data()));

        DATA_BLOB outputData;
        std::unique_ptr<encryptor::AbstractEncryptor> encryptor(new encryptor::DpapiEncryptor);
        encryptor->encrypt(&inputData, &entropy, &outputData);

        std::string encryptedString;
        try {
            encryptedString.assign(reinterpret_cast<char*>(outputData.pbData), outputData.cbData);
        } catch(const std::exception &ex) {
            LocalFree(outputData.pbData);
            //LOG(ERROR) << "[BlockFileEncryptor::encryptString] error " << ex.what();
        }
        LocalFree(outputData.pbData);

        return encryptedString;
    }

    std::string BlockFileEncryptor::decryptStringWithKey(const std::string &encryptedString, const std::vector<std::uint8_t> &key) { /* throw() */

        DATA_BLOB inputData;
        inputData.cbData = encryptedString.size();
        inputData.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(encryptedString.c_str()));

        DATA_BLOB entropy;
        entropy.cbData = key.size();
        entropy.pbData = reinterpret_cast<BYTE*>(const_cast<std::uint8_t*>(key.data()));

        DATA_BLOB outputData;
        std::unique_ptr<encryptor::AbstractEncryptor> encryptor(new encryptor::DpapiEncryptor);

        encryptor->decrypt(&inputData, &entropy, &outputData);

        std::string decryptedString;
        try {
            decryptedString.assign(reinterpret_cast<char*>(outputData.pbData), outputData.cbData);
        } catch(const std::exception &ex) {
            //LOG(ERROR) << "[BlockFileEncryptor::encryptString] error " << ex.what();
            LocalFree(outputData.pbData);
            throw;
        }

        LocalFree(outputData.pbData);
        return decryptedString;
    }

} // namespace