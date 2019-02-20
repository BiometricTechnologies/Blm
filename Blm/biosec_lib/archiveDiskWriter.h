#ifndef __archiveWriteDisk_h__
#define __archiveWriteDisk_h__

#include "libarchive/archive.h"
#include "archiveEntry.h"

namespace blm_utils{

	/// <summary>
	/// RAII wrapper for archive writing on disk
	/// </summary>
	class ArchiveDiskWriter {
	public:
		ArchiveDiskWriter();
		~ArchiveDiskWriter();
		void writeHeader(const ArchiveEntry& entry);
		void finishEntry();
		void close()/* throw() */;
		struct archive* getArchive();
	private:
		ArchiveDiskWriter(const ArchiveDiskWriter &);
		ArchiveDiskWriter &operator=(const ArchiveDiskWriter &);
		struct archive* arch_;
	};

}

#endif // __archiveWriteDisk_h__
