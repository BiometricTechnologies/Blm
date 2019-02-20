#ifndef __archieverFactory_h__
#define __archieverFactory_h__

#include <memory>
#include "IArchiever.h"
#include "tarEncryptor.h"
#include "tarDecryptor.h"
#include "biosec_exceptions.h"

namespace blm_utils{


class ArchieverFactory {
public:
	static std::unique_ptr<IArchiever> create(IArchiever::Type type) {
		switch(type) {
		case IArchiever::Type::INVALID_TYPE:
			throw InvalidMode("invalid mode");
			break;
		case IArchiever::Type::TAR_ENCRYPTOR:
			return std::unique_ptr<IArchiever>(new TarEncryptor(
				new BlockFileEncryptor(
				new encryptor::DpapiEncryptor)));
			break;
		case IArchiever::Type::TAR_DECRYPTOR:
			return std::unique_ptr<IArchiever>(new TarDecryptor(
				new BlockFileEncryptor(
				new encryptor::DpapiEncryptor)));
			break;
		default:
			throw InvalidMode("invalid mode, forgot to add?");
			break;
		}
	}
};


} // namespace

#endif // __archieverFactory_h__
