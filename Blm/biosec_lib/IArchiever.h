#ifndef __IArchiever_h__
#define __IArchiever_h__

#include <cstdint>
#include <string>
#include <vector>
#include "biosec_lib.h"
#include "biosec_exceptions.h"


namespace blm_utils {


    typedef struct SourcePathInfo {
        std::wstring path;

        SourcePathInfo(const std::wstring &pathInfo)
            : path(pathInfo) {}
    };


    enum class EntityType {
        INVALID,
        FOLDER,
        REGULAR
    };


    struct ArchiveEntryInfo {
        std::wstring fileName;
        std::uintmax_t fileSize;
        time_t mtime;
        EntityType type;
    };

    class IArchiever {
    public:
        enum class Type {
            INVALID_TYPE,
            TAR_ENCRYPTOR,
            TAR_DECRYPTOR
        };

        enum class Result {
            INVALID_RES,
            SUCCESS,
            USER_NOT_ENROLLED,
            BIOMETRIC_ACCEPTED,
            BIOMETRIC_MATCH_NOT_FOUND,
            BIOMETRIC_NOT_BELONGS_TO_USER,
            BIOMETRIC_INVALID_ENCRYPTION_KEY,
            USER_NOT_AN_ARCHIVE_OWNER

        };

        enum class ArchiveEntryQueryRes {
            INVALID_RES,
            SUCCESS,
            OUT_OF_RANGE
        };

        IArchiever() {};
        virtual ~IArchiever() {};

        virtual Result isUserEnrolled() = 0;
		virtual Result getArchiveContent() = 0;
		virtual Result getArchiveContent(const std::vector<std::uint8_t> &data) = 0;
		virtual Result setEncryptionKey(const std::vector<std::uint8_t> &data) = 0;
        virtual std::uint64_t getArchiveEnriesCount() = 0;
        virtual void needOverwrite(bool value) = 0;
        virtual Result setSourcePathlist(const std::vector<SourcePathInfo> &sources) = 0;
        virtual void setTargetDirectory(const std::wstring &targetDirectory) = 0;
        virtual std::wstring getBiosecureFilePath() = 0;
        virtual void setCryptoProvider(const std::wstring &providerName) = 0;
        virtual std::wstring getUserName() = 0;
        virtual Result putBiometric(const std::wstring &biometricType, const std::vector<std::uint8_t> &data) = 0;
        virtual void setItemsToDecompress(const std::vector<std::uint64_t> &items) = 0;
        virtual ArchiveEntryQueryRes getArchiveEntryInfoAt(ArchiveEntryInfo* queryInfo, std::uint32_t index) = 0;
        virtual void setProgressUpdate(const std::function<void(const wchar_t*, const wchar_t*, uint32_t, uint32_t, uint32_t, uint32_t)> &progressUpdate) = 0;
        virtual void setOperationErrorCbc(const std::function<void(BiosecOperationErrorType, const wchar_t*, const wchar_t*)> &operationError) = 0;
        virtual void startEncryption() = 0;
        virtual void startDecryption() = 0;

	protected:
		// TODO this is temporary method for refactoring
		// it should be deleted when all methods will be moved to subclass(an.skornyakov@gmail.com): 
		virtual IArchiever::Type getType() = 0;

    private:
        IArchiever &operator=(const IArchiever &);
        IArchiever(const IArchiever &);

    };


} // namespace

#endif // __IArchiever_h__
