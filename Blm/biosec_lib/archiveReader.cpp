#include <exception>
#include "archiveReader.h"

namespace blm_utils {


    ArchiveReader::ArchiveReader()
        : arch_(nullptr),
	      entryReadOffset(0) {
        arch_ = archive_read_new();
        auto res = archive_read_support_format_all(arch_);
        if(ARCHIVE_OK != res) {
            throw LibArchiveInternalError(archive_error_string(arch_));
        }
        res = archive_read_support_compression_all(arch_);
        if(ARCHIVE_OK != res) {
            throw LibArchiveInternalError(archive_error_string(arch_));
        }
    }


    ArchiveReader::~ArchiveReader() {
        archive_read_free(arch_);
    }


    void ArchiveReader::close() {
        if(ARCHIVE_OK != archive_write_close(arch_)) {
            throw std::exception(archive_error_string(arch_));
        }
    }


    struct archive* ArchiveReader::getArchive() {
        return arch_;
    }


    void ArchiveReader::open(const std::wstring &pathname) {
        auto res = archive_read_open_filename_w(arch_, pathname.c_str(), kReadBlockSize_);
        if(ARCHIVE_OK != res) {
            throw LibArchiveInternalError(archive_error_string(arch_));
        }
    }


    std::unique_ptr<ArchiveEntry> ArchiveReader::getNextEntry() {
        std::unique_ptr<ArchiveEntry> nextEntry(new ArchiveEntry);
        auto res = archive_read_next_header2(arch_, nextEntry->getEntry());
        if(res == ARCHIVE_EOF) {
            return std::unique_ptr<ArchiveEntry>();
        }
        if(res != ARCHIVE_OK) {
            throw LibArchiveInternalError(archive_error_string(arch_));
        }

        return std::move(nextEntry);
    }


    bool ArchiveReader::readNextDataBlock(std::vector<std::uint8_t> &buffer) {
		buffer.resize(kReadBlockSize_);
		auto res = archive_read_data(arch_, &buffer[0], buffer.size());
		if(ARCHIVE_FATAL == res || ARCHIVE_WARN == res || ARCHIVE_RETRY == res) {
			throw LibArchiveInternalError(archive_error_string(arch_));
		}

		if(0 == res) {
			return false; // eof
		}

		buffer.resize(res);
		return true;

    }

}

