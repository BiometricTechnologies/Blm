#ifndef __archiveWrite_h__
#define __archiveWrite_h__

#include <string>
#include "libarchive/archive.h"


namespace blm_utils{


/// <summary>
/// RAII wrapper for archive writing
/// </summary>
class ArchiveWriter {
public:
	ArchiveWriter();
	~ArchiveWriter();
	void open(const std::wstring &pathname);
	void close() /*throw()*/;
	struct archive* getArchive();
private:
	ArchiveWriter(const ArchiveWriter &);
	ArchiveWriter &operator=(const ArchiveWriter &);
	struct archive* arch_;

};



}

#endif // __archiveWrite_h__
