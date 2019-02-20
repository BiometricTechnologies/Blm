#ifndef __archiverEncryption_h__
#define __archiverEncryption_h__

#include "archiever.h"

namespace blm_utils{

	class TarEncryptor : public TarLibarchiveArchiever{
	public:
		TarEncryptor(IFileEncryptor* fileEncryptor)
			:TarLibarchiveArchiever(fileEncryptor){

		}

		virtual IArchiever::Type getType() {
			return IArchiever::Type::TAR_ENCRYPTOR;
		}

		virtual std::wstring getBiosecureFilePath();
		virtual Result setSourcePathlist(const std::vector<SourcePathInfo> &sources);

	protected:
		

	private:
		static const wchar_t* biosecFileExtension() {
			return L".izbiosecure";
		}

		void produceTargetPath();

		boost::filesystem::path produceTargetPathForMultipleSources(const std::vector<std::shared_ptr<boost::filesystem::path>>& sources);
		boost::filesystem::path produceTargetPathForSingleSource(const boost::filesystem::path& source);

	};

} // namespace

#endif // __archiverEncryption_h__
