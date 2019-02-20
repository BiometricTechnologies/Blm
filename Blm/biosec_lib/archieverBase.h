#ifndef __archieverBase_h__
#define __archieverBase_h__

#include "boost/filesystem/path.hpp"
#include "severityLogger.h"
#include "IArchiever.h"


namespace blm_utils {

    class IArchiever;

    class ArchieverBase : public IArchiever {
    public:
        ArchieverBase(IFileEncryptor* fileEncryptor);
        virtual void setCryptoProvider(const std::wstring &providerName);

    protected:
        boost::filesystem::path getTargetPath() const {
            return targetPath_;
        }

        void setTargetPath(const boost::filesystem::path &val) {
            targetPath_ = val;
        }

        std::vector<std::shared_ptr<boost::filesystem::path>> getSourcePaths() const {
            return sourcePaths_;
        }

        void setSourcePaths(const std::vector<std::shared_ptr<boost::filesystem::path>> &val) {
            sourcePaths_ = val;
        }

        void clearSourcePaths() {
            sourcePaths_.clear();
        }

        void addSourcePath(const boost::filesystem::path &path) {
            sourcePaths_.push_back(std::make_shared<boost::filesystem::path>(path));
        }

        std::shared_ptr<SeverityLogger> getLogger() const {
            return logger_;
        }

        std::shared_ptr<blm_utils::IFileEncryptor> getFileEncryptor() const {
            return fileEncryptor_;
        }

    private:
        std::vector<std::wstring> cryptoProviders_;
        std::uint8_t cryptoProviderSelected_;

        boost::filesystem::path targetPath_;
        std::vector<std::shared_ptr<boost::filesystem::path>> sourcePaths_;

        std::shared_ptr<SeverityLogger> logger_;
        std::shared_ptr<blm_utils::IFileEncryptor> fileEncryptor_;


    };


    static const wchar_t* cryptoProviders[] = {
        L"Windows DPAPI"
    };


    inline std::vector<std::wstring> getAvailableCryptoProviders() {
        std::vector<std::wstring> providers;
        for(auto provider = 0; provider < sizeof(cryptoProviders) / sizeof(cryptoProviders[0]); ++provider) {
            providers.emplace_back(cryptoProviders[provider]);
        }
        return providers;
    }


} // namespace


#endif // __archieverBase_h__
