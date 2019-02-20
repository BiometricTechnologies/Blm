#ifndef __archiveEntry_h__
#define __archiveEntry_h__


#include <vector>
#include <memory>
#include "libarchive/archive_entry.h"
#include "libarchive/archive.h"
#include "fileEncryptor.h"

namespace blm_utils{

	enum class EntryFileType{
		REGULAR,
		DIRECTORY
	};

	class ArchiveEntry {
	public:		

		ArchiveEntry();
		~ArchiveEntry();
		struct archive_entry* getEntry() const;
		std::vector<char> readAttribute(const std::string& name) const;
		void writeAttribute(const std::string& name, void* val, size_t size);
		void setPathnameWithEncryption(const std::wstring &pathName, std::shared_ptr<blm_utils::IFileEncryptor> encryptor);
		void setPathnameW(const std::wstring& pathname);
		std::wstring getPathnameW() const;
		void setSize(std::int64_t size);
		void setPermissions(std::int64_t permissons);
		void setMtime(time_t mTime);
		time_t getMtime();
		void setFileType(EntryFileType filetype);
		EntryFileType getFileType() const;
		std::wstring getPathnameWithEncryption(std::shared_ptr<blm_utils::IFileEncryptor> encryptor) const;
		DWORD readFileAttributes() const;
		void writeFileAttributes(DWORD attributes);
	private:
		static const std::size_t kRandomStringLength_ =  0x20;
		static const char* attributesKey() {
			return "fAttr";
		}

		ArchiveEntry(const ArchiveEntry &);
		ArchiveEntry &operator=(const ArchiveEntry &);
		struct archive_entry* entry_;

		static const char* pathnameEncryptedAttribute() {
			return "nameEcnrypted";
		}
	};

} // blm_utils
#endif // __archiveEntry_h__
