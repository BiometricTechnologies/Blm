#include "tarDecryptor.h"

namespace blm_utils {


    std::uint64_t TarDecryptor::getArchiveEnriesCount() {
        return getArchiveContentList().size();
    }


    std::wstring TarDecryptor::getBiosecureFilePath() {
        return getSourcePaths().at(0)->wstring();
    }


    void TarDecryptor::produceTargetPath() {
        auto sourcePaths = getSourcePaths();
        if(1 != sourcePaths.size()) {
            throw std::logic_error("multiple decryption sources");
        }
        setTargetPath(sourcePaths[0]->parent_path());
    }


    IArchiever::Result TarDecryptor::setSourcePathlist(const std::vector<SourcePathInfo> &sources) {
        for(auto &thisPath : sources) {
            if(!this->isUserAnArchiveOwner(thisPath.path)) {
                clearSourcePaths();
                return IArchiever::Result::USER_NOT_AN_ARCHIVE_OWNER;
            }

            addSourcePath(thisPath.path);
            this->produceTargetPath();
        }
        return IArchiever::Result::SUCCESS;
    }


    bool TarDecryptor::isUserAnArchiveOwner(const std::wstring &archivePath) {
        ArchiveReader archiveReader; // RAII
        archiveReader.open(archivePath);

        auto nextEntry = archiveReader.getNextEntry();
        while(nextEntry) {
            if(isHeader(*nextEntry)) {
                //getLogger()->log(SeverityLevel::NOTIFICATION, std::string() + __FUNCTION__ +  " header file was found in archive");

                std::string headerContentString;
                std::vector<std::uint8_t> dataBuffer;
                while(archiveReader.readNextDataBlock(dataBuffer)) {
                    headerContentString.append(std::begin(dataBuffer), std::end(dataBuffer));
                }

                try {
                    // if we can decrypt string from header without an exception, then user is correct
                    std::string decryptedHeaderString;
                    decryptedHeaderString = getFileEncryptor()->decryptString(headerContentString);
                    return true;
                } catch(const std::exception &ex) {
                    getLogger()->log(SeverityLevel::ERR, "error deserialize header");
                    return false;
                }
            }
            nextEntry = archiveReader.getNextEntry();
        }

        throw ArchiveInternalError();
    }

}

