#ifndef __archieverDecryption_h__
#define __archieverDecryption_h__

#include "archiever.h"

namespace blm_utils {

    class TarDecryptor : public TarLibarchiveArchiever {
    public:
        TarDecryptor(IFileEncryptor* fileEncryptor)
            : TarLibarchiveArchiever(fileEncryptor) {

        }

        virtual IArchiever::Type getType() {
            return IArchiever::Type::TAR_DECRYPTOR;
        }

        virtual std::uint64_t getArchiveEnriesCount();
        virtual std::wstring getBiosecureFilePath();
		virtual Result setSourcePathlist(const std::vector<SourcePathInfo> &sources);


    protected:
        

	private:
		 bool isUserAnArchiveOwner(const std::wstring &archivePath);
		 void produceTargetPath();

    };


} // namespace

#endif // __archieverDecryption_h__
