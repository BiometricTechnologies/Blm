#ifndef __archiveRead_h__
#define __archiveRead_h__

#include <memory>
#include <cstdint>
#include "libarchive/archive.h"
#include "archiveEntry.h"

namespace blm_utils {

    /// <summary>
    /// RAII wrapper for archive reading
    /// </summary>
    class ArchiveReader {
    public:
        ArchiveReader();
        ~ArchiveReader();
        void open(const std::wstring &pathname);
		bool readNextDataBlock(std::vector<std::uint8_t>& buffer); 
        void close() /*throw()*/;
		std::unique_ptr<ArchiveEntry> getNextEntry();
        struct archive* getArchive();
    private:
		static const std::size_t kReadBlockSize_ = 10240;

        ArchiveReader(const ArchiveEntry &);
        ArchiveReader &operator=(const ArchiveReader &);
        struct archive* arch_;
		int64_t entryReadOffset;
    };

}

#endif // __archiveRead_h__
