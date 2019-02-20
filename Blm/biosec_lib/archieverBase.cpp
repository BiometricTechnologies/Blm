#include "fileEncryptor.h"
#include "archieverBase.h"

namespace blm_utils {


    void ArchieverBase::setCryptoProvider(const std::wstring &providerName) {
        std::size_t index = 0;
        for(auto &provider : cryptoProviders_) {
            if(provider == providerName) {
                break;
            } else {
                ++index;
            }
        }

        if(index < cryptoProviders_.size()) {
            cryptoProviderSelected_ = index;
        } else {
            throw std::out_of_range("crypto provider index out of range");
        }
    }

    ArchieverBase::ArchieverBase(IFileEncryptor* fileEncryptor)
        : logger_(std::make_shared<SeverityLogger>()),
	      fileEncryptor_(fileEncryptor){

    }


} // namespace

