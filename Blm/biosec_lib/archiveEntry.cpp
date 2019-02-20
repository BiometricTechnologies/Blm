#include "common.h"
#include "archiveEntry.h"

namespace blm_utils {


    ArchiveEntry::ArchiveEntry()
        : entry_(nullptr) {
        entry_ = archive_entry_new();

    }

    ArchiveEntry::~ArchiveEntry() {
        archive_entry_free(entry_);
    }

    struct archive_entry* ArchiveEntry::getEntry() const {
        return entry_;
    }


    std::vector<char> ArchiveEntry::readAttribute(const std::string &name) const {
        archive_entry_xattr_reset(entry_);
        const char* rname;
        const void* rvalue;
        size_t rsize = 0;
        std::vector<char> valueVec;
        while(archive_entry_xattr_next(entry_, &rname, &rvalue, &rsize) != ARCHIVE_WARN) {
            if(!name.compare(rname)) {
                valueVec.assign((char*)rvalue, (char*)rvalue + rsize);
            }
        }
        return valueVec;
    }

    void ArchiveEntry::writeAttribute(const std::string &name, void* val, size_t size) {
        if(nullptr == val) {
            throw std::invalid_argument("[ArchiveEntry::writeAttribute] error nullpt argument val");
        }
        archive_entry_xattr_add_entry(entry_, name.c_str(), val, size);
    }

    void ArchiveEntry::setPathnameWithEncryption(const std::wstring &pathName, std::shared_ptr<blm_utils::IFileEncryptor> encryptor) {
        // encrypt pathname and write in special entry attribute, put random string in regular pathname entry attribute
        std::string pathNameStr(reinterpret_cast<const char*>(pathName.c_str()), pathName.size()*sizeof(std::wstring::value_type)) ;
        //std::copy(pathName.begin(), pathName.end(), std::back_inserter(pathNameStr));
        auto encryptedPathname = encryptor->encryptString(pathNameStr);
        this->writeAttribute(pathnameEncryptedAttribute(), (void*)encryptedPathname.c_str(), encryptedPathname.size());
        archive_entry_copy_pathname(entry_, getRandomString(kRandomStringLength_).c_str());
    }

    std::wstring ArchiveEntry::getPathnameWithEncryption(std::shared_ptr<blm_utils::IFileEncryptor> encryptor) const {
        auto attributeData = this->readAttribute(pathnameEncryptedAttribute());
        std::string attributeString(std::begin(attributeData), std::end(attributeData));
        auto decryptedPathnameStr = encryptor->decryptString(attributeString);
        return std::wstring(reinterpret_cast<wchar_t*>(const_cast<char*>(decryptedPathnameStr.c_str())), decryptedPathnameStr.size() / sizeof(std::wstring::value_type));
    }


    DWORD ArchiveEntry::readFileAttributes() const {
        auto attributesBuffer = this->readAttribute(this->attributesKey());
        if(attributesBuffer.empty()) {
            throw std::logic_error("can't read file attributes from archive entry");
        }
        DWORD attributes = *(reinterpret_cast<DWORD*>(&attributesBuffer[0]));
        return attributes;
    }


    void ArchiveEntry::writeFileAttributes(DWORD attributes) {
        this->writeAttribute(this->attributesKey(), &attributes, sizeof(attributes));
    }


    void ArchiveEntry::setPathnameW(const std::wstring &pathname) {
        archive_entry_copy_pathname_w(entry_, pathname.c_str());
    }


    void ArchiveEntry::setSize(std::int64_t size) {
        archive_entry_set_size(entry_, size);
    }


    void ArchiveEntry::setFileType(EntryFileType filetype) {
        std::uint32_t libraryFileType;
        switch(filetype) {
            case EntryFileType::REGULAR:
                libraryFileType = AE_IFREG;
                break;
            case EntryFileType::DIRECTORY:
                libraryFileType = AE_IFDIR;
                break;
            default:
                throw InvalidArgument("filetype");
                break;
        }

        archive_entry_set_filetype(entry_, libraryFileType);
    }


    void ArchiveEntry::setPermissions(std::int64_t permissons) {
        archive_entry_set_perm(entry_, permissons);
    }


    void ArchiveEntry::setMtime(time_t mTime) {
        archive_entry_set_mtime(entry_, mTime, 0);
    }


    time_t ArchiveEntry::getMtime() {
        return archive_entry_mtime(entry_);
    }


    std::wstring ArchiveEntry::getPathnameW() const {
        std::wstring pathName(archive_entry_pathname_w(entry_));
        return std::move(pathName);
    }


    EntryFileType ArchiveEntry::getFileType() const {
        auto entryFileType = archive_entry_filetype(entry_);
        switch(entryFileType) {
            case AE_IFREG:
				return EntryFileType::REGULAR;
                break;
            case AE_IFDIR:
				return EntryFileType::DIRECTORY;
                break;
            default:
				throw UnknownEnumValue();
                break;
        }
    }


}