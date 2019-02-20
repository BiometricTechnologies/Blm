#include <exception>
#include "biosec_exceptions.h"
#include "archiveWriter.h"

namespace blm_utils {

    ArchiveWriter::ArchiveWriter()
        : arch_(nullptr) {
        arch_ = archive_write_new();
        if(ARCHIVE_OK != archive_write_set_format_pax_restricted(arch_)) {
            throw LibArchiveInternalError(archive_error_string(arch_));
        }

    }


    ArchiveWriter::~ArchiveWriter() {
        archive_write_free(arch_);
    }


    struct archive* ArchiveWriter::getArchive() {
        return arch_;
    }


    void ArchiveWriter::close() {
        if(ARCHIVE_OK != archive_write_close(arch_)) {
            throw std::exception(archive_error_string(arch_));
        }
    }


    void ArchiveWriter::open(const std::wstring &pathname) {
        if(ARCHIVE_OK != archive_write_open_filename_w(arch_, pathname.c_str())) {
            throw std::exception(archive_error_string(arch_));
        }
    }

}

