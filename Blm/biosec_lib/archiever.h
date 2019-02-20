#ifndef __ARCHIEVER_H__
#define __ARCHIEVER_H__


#include <cstdlib>
#include <cstdio>
#include <memory>
#include <functional>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <chrono>
#include <unordered_set>
#include <fcntl.h>
#include <wchar.h>

// windows
#include <io.h>
#include <windows.h>
#include <Shlwapi.h>
#include <sys/stat.h>

// boost
#include "boost/filesystem.hpp"
#include "boost/iterator/filter_iterator.hpp"
#include "boost/algorithm/string/trim.hpp"

// 3rd party
#include "libarchive/archive.h"

// application
#include "fileEncryptor.h"
#include "biosec_lib.h"
#include "archieverBase.h"
#include "common.h"
#include "IProgressReporter.h"
#include "progressBar.h"
#include "archiveEntry.h"
#include "archiveReader.h"
#include "archiveWriter.h"
#include "archiveDiskWriter.h"
#include "biosecHeader.h"
#include "biometricDatabaseMaster.h"


namespace blm_utils {



    class TarLibarchiveArchiever : public ArchieverBase, IProgressObserver {
    public:
        TarLibarchiveArchiever(IFileEncryptor* fileEncryptor);
        virtual ~TarLibarchiveArchiever();

        // IArchiever
        virtual Result isUserEnrolled();
        virtual std::uint64_t getArchiveEnriesCount();
        virtual void needOverwrite(bool value);
        virtual void setTargetDirectory(const std::wstring &targetDirectory);
        virtual std::wstring getBiosecureFilePath();
        virtual std::wstring getUserName();
		virtual Result putSecretKey(const std::vector<std::uint8_t> &data);
        virtual Result putBiometric(const std::wstring &biometricType, const std::vector<std::uint8_t> &data);
        virtual void setItemsToDecompress(const std::vector<std::uint64_t> &items);
        virtual ArchiveEntryQueryRes getArchiveEntryInfoAt(ArchiveEntryInfo* queryInfo, std::uint32_t index) ;
        virtual void setProgressUpdate(const std::function<void(const wchar_t*, const wchar_t*, uint32_t, uint32_t, uint32_t, uint32_t)> &progressUpdate);
        virtual void setOperationErrorCbc(const std::function<void(BiosecOperationErrorType, const wchar_t*, const wchar_t*)> &operationError);
        virtual void startEncryption();
        virtual void startDecryption();

        // IProgressObserver
        virtual void reportProgress(void* reporter, std::int64_t done, std::int64_t total);

    protected:
        // TODO this is temporary method for refactoring
        // it should be deleted when all methods will be moved to subclass(an.skornyakov@gmail.com):
        virtual IArchiever::Type getType() {
            return type_;
        }

        

        std::vector<ArchiveEntryInfo> getArchiveContentList() const {
            return archiveContentList_;
        }

		bool isHeader(const ArchiveEntry &entry);

    private:
		typedef enum{
			NONE,
			NORMAL,
			FILE_ONLY
		}ExtractMode;

        // constants
        #pragma region constants

        static const uint32_t kTotalPbIndex_ = 0;
        static const uint32_t kCurrentPbIndex_ = 1;
        static const uint32_t kProgressBarDefaultMaxValue = 100;
        static const uint32_t kCurrentBarEncryptingExtent = 95;

        static const uint32_t kDelayBeforeCloseProgressbarMs = 3000;
        static const std::size_t kReadBufferSize_ =  0x2000;
        static const std::size_t kMaxEncryptionKeySize_ =  256;
		std::string myKey;
        #pragma endregion constants

        //strings
        #pragma region strings


        static const wchar_t* archiveHeaderName() {
            return L"header.bs";
        }
        static const wchar_t* successSecureMessage() {
            return L"Encryption completed";
        }
        static const wchar_t* successUnsecureMessage() {
            return L"Decryption completed";
        }
        static const wchar_t* securingProgressBarMessage() {
            return L"Securing ";
        }
        static const wchar_t* unsecuringProgressBarMessage() {
            return L"Unsecuring ";
        }
        static const wchar_t* wrongUserMessage() {
            return L"Wrong user logged in";
        }
        static const char* fileSizeAttribute() {
            return "oSize";
        }
        static const wchar_t* tempFileExtension() {
            return L".tmp";
        }
        

        #pragma endregion strings

        // validating settings
        // [[ modern ]]

        // [[ modern ]]
       
        // [[ modern ]]
        bool isEncryptionKeyValidForArchive(const std::wstring &archivePath);
        // [[ modern ]]
        IArchiever::Result verifyBiometric();
		IArchiever::Result getArchiveContent();
		IArchiever::Result getArchiveContent(const std::vector<std::uint8_t> &data);
		IArchiever::Result setEncryptionKey(const std::vector<std::uint8_t> &data);
		// [[ modern ]]
        BiosecHeader readArchiveHeader();


        // constructing archive
        // [[ modern ]]
        void addToArchive(const std::wstring &pathName, const std::wstring &archiveRelativePath) /*throw()*/;
        // [[ modern ]]
        void addCatalogEntry(std::wstring &relativePath, int64_t fSize, time_t mTime, DWORD attributes) /* throw()*/;
        // [[ modern ]]
        void addRegularFileEntry(std::wstring &relativePath, const std::wstring &pathName, time_t mTime, DWORD attributes)/*throw()*/;
        // [[ modern ]]
        void writeEncryptedArchiveHeader(const BiosecHeader &header);

        // extracting from archive
        // [[ modern ]]
        // passing argument by value to avoid copying within method
        boost::filesystem::path getPathOriginalFileName(boost::filesystem::path path, const ArchiveEntry &entry);
        // [[ modern ]]
        void readSourceArchivesContent();
        // [[ modern ]]
        void getArchiveContent(const std::wstring &archPath);
        // [[ modern ]]
        void decompress_(const std::wstring &srcPath);
        // [[ modern ]]
        void decompressFromList_(const std::wstring &srcPath, const std::unordered_set<std::wstring> &pathsToDecrypt);
		// [[ modern ]]
		bool needToExtractEntry(const ArchiveEntry &entry, const std::unordered_set<std::wstring> &pathsToDecrypt);
		// [[ modern ]]
		ExtractMode howToExtractEntry(const ArchiveEntry &entry, const std::unordered_set<std::wstring> &pathsToDecrypt);
		// [[ modern ]]
		boost::filesystem::path produceEntryTargetPath(const std::wstring &srcPath, ArchiveEntry &entry, ExtractMode mode, const std::unordered_set<std::wstring> &pathsToDecrypt);
        // [[ modern ]]
        void updateEntryPathname(const boost::filesystem::path &targetPath, ArchiveEntry &entry);
        // [[ modern ]]
		boost::filesystem::path getSubstitudePathname(const boost::filesystem::path &initialPath, const ArchiveEntry &entry);
		// [[ modern ]]
		boost::filesystem::path getSubstitudeRootPath(const boost::filesystem::path &initialPath);
		// [[ modern ]]
		void decryptEntry(const ArchiveEntry &entry);
        // [[ modern ]]
        void extractCatalogEntryIfNotExists(const boost::filesystem::path &catalogPathname, const ArchiveEntry &entry);
        
        // settings
        bool needOverwrite_;
        IArchiever::Type type_;
        






        
        
        std::vector<std::uint64_t> itemsToDecompress_;


        // internal
        bool shouldDeleteAllCreatedFiles_;
        bool hasResultRootPath_;
        struct archive* arch_;
        std::wstring rootPath_;
        bool isRootSet_;
        std::int32_t totalFilesCount_;
		
		template <typename Iter>
		bool WriteBinary(Iter first, Iter last);

		std::vector<uint8_t> encryptionKey1_;
        std::vector<uint8_t> encryptionKey_;
        std::unique_ptr<ProgressBarWindow> progressBar_;
        BiometricDatabaseMaster biometricDbMaster_;
        std::vector<ArchiveEntryInfo> archiveContentList_;

		std::list<boost::filesystem::path> producedPaths_;
		std::map<std::wstring, std::wstring> producedCatalogsPaths_;


        std::shared_ptr<blm_utils::IProgressReporter> progressReporter_;
        
        std::unique_ptr<std::function<void(BiosecOperationErrorType, const wchar_t*, const wchar_t*)>> operationError_;
    };


    


    
}

#endif // __ARCHIEVER_H__