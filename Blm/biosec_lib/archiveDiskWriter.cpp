#include <exception>
#include "archiveDiskWriter.h"
#include "biosec_exceptions.h"

namespace blm_utils {

    ArchiveDiskWriter::ArchiveDiskWriter()
        : arch_(nullptr) {
        arch_ = archive_write_disk_new();
        int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
        if(ARCHIVE_OK != archive_write_disk_set_options(arch_, flags)) {
            throw LibArchiveInternalError(archive_error_string(arch_));
        }
        if(ARCHIVE_OK != archive_write_disk_set_standard_lookup(arch_)) {
            throw LibArchiveInternalError(archive_error_string(arch_));
        }

    }


    ArchiveDiskWriter::~ArchiveDiskWriter() {
        archive_write_free(arch_);
    }


    struct archive* ArchiveDiskWriter::getArchive() {
        return arch_;
    }


    void ArchiveDiskWriter::close() {
        if(ARCHIVE_OK != archive_write_close(arch_)) {
            throw LibArchiveInternalError(archive_error_string(arch_));
        }
    }


    void ArchiveDiskWriter::writeHeader(const ArchiveEntry &entry) {
        if(ARCHIVE_OK != archive_write_header(arch_, entry.getEntry())) {
            throw LibArchiveInternalError(archive_error_string(arch_));
        }
    }


    void ArchiveDiskWriter::finishEntry() {
        if(ARCHIVE_OK != archive_write_finish_entry(arch_)) {
            throw LibArchiveInternalError(archive_error_string(arch_));
        }
    }

}



