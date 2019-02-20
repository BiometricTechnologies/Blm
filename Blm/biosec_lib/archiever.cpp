#include <exception>
#include "archiever.h"
#include <iomanip>

namespace blm_utils {



    TarLibarchiveArchiever::TarLibarchiveArchiever(blm_utils::IFileEncryptor* fileEncryptor)
        : ArchieverBase(fileEncryptor),
          arch_(nullptr),
          rootPath_(),
          isRootSet_(false),
          needOverwrite_(false),
          shouldDeleteAllCreatedFiles_(false),
          hasResultRootPath_(false),
          totalFilesCount_(0) {

        progressReporter_ = std::dynamic_pointer_cast<IProgressReporter>(getFileEncryptor());
        progressReporter_->setReceiver(this);
    }


    void TarLibarchiveArchiever::needOverwrite(bool val) {
        needOverwrite_ = val;
    }


    TarLibarchiveArchiever::~TarLibarchiveArchiever() {
        if(shouldDeleteAllCreatedFiles_) {
            std::for_each(std::begin(producedPaths_), std::end(producedPaths_), [&](const boost::filesystem::path & p) {
                if(!removeFilesRecursive(p)) {
                    getLogger()->log(SeverityLevel::ERR, "error removing created files");
                }
            });
        }
    }


    std::uint64_t TarLibarchiveArchiever::getArchiveEnriesCount() {
        throw NotImplemented("not implemented in this class");
    }


    void TarLibarchiveArchiever::setOperationErrorCbc(const std::function<void(BiosecOperationErrorType, const wchar_t*, const wchar_t*)> &operationError) {
        operationError_.reset(new std::function<void(BiosecOperationErrorType, const wchar_t*, const wchar_t*)>(operationError));
    }



    bool TarLibarchiveArchiever::isEncryptionKeyValidForArchive(const std::wstring &archivePath) {
        ArchiveReader archiveRead; // RAII
        arch_ = archiveRead.getArchive();
        archiveRead.open(archivePath);

        try {
            this->readArchiveHeader();
        } catch(const std::exception &ex) {
            getLogger()->log(SeverityLevel::ERR, ex.what());
            return false;
        }

        return true;
    }

	void readfile(const std::string &filepath, std::string &buffer){
		std::ifstream fin(filepath.c_str());
		getline(fin, buffer, char(-1));
		fin.close();
	}

    BiosecHeader TarLibarchiveArchiever::readArchiveHeader() {
        ArchiveEntry archiveEntry; // RAII
        struct archive_entry* entry = archiveEntry.getEntry();

        while(1) {
            auto res = archive_read_next_header2(arch_, entry);
            if(ARCHIVE_EOF == res) {
                throw std::exception("[TarLibarchiveArchieverImpl::readArchiveHeader()] secured file is broken");
            }
            if(res != ARCHIVE_OK) {
                throw std::exception("[TarLibarchiveArchieverImpl::readArchiveHeader()] secured file is broken");
            }

            if(isHeader(archiveEntry)) {

                //getLogger()->log(SeverityLevel::NOTIFICATION, std::string() + __FUNCTION__ + " header file was found in archive");

                const void* buff;
                size_t size;
                int64_t offset;
                std::string headerContentString;

                for(;;) {
                    auto r = archive_read_data_block(arch_, &buff, &size, &offset);
                    if(r == ARCHIVE_EOF) {
                        try {
                            // TODO: here we can check data owner (an.skornyakov@gmail.com)
                            std::string decryptedHeaderString;
                            decryptedHeaderString = getFileEncryptor()->decryptString(headerContentString);
							//111
                            std::string decryptedWithKeyHeaderString;
							/*std::copy(std::begin(encryptionKey_), std::end(encryptionKey_), std::back_inserter(myKey));

							std::ofstream file("C:\\Logs\\myKey.txt");

							if (!file) { std::cerr << "Error writing to ..." << std::endl; }
							
							else {
								file << myKey;
							}
						
							file.close();*/

							decryptedWithKeyHeaderString = getFileEncryptor()->decryptStringWithKey(decryptedHeaderString, encryptionKey_);

                            BiosecHeader header;
                            header.deserialize(decryptedHeaderString);
                            return header;
                        } catch(const std::exception &ex) {
                            // LOG(ERROR) << "[TarLibarchiveArchiever::readArchiveHeader()] error deserializing header " << ex.what();
                            std::ostringstream msg;
                            msg << "error deserializing header " << ex.what();
                            getLogger()->log(SeverityLevel::ERR, msg.str());
                            throw;
                        }

                    }
                    if(r != ARCHIVE_OK) {
                        throw std::exception(archive_error_string(arch_));
                    }
                    headerContentString.append(static_cast<char*>(const_cast<void*>(buff)), size);
                }

            }
        }
    }


    void TarLibarchiveArchiever::writeEncryptedArchiveHeader(const BiosecHeader &header) {

        // TODO: here we should preserve original files permissions (an.skornyakov@gmail.com)
        auto headerString = header.serialize();

        std::string encryptedByUserCredentialsHeaderString;
        std::string encryptedWithKeyHeaderString;
        encryptedWithKeyHeaderString = getFileEncryptor()->encryptStringWithKey(headerString, encryptionKey_);
        encryptedByUserCredentialsHeaderString = getFileEncryptor()->encryptString(encryptedWithKeyHeaderString);


        ArchiveEntry archiveEntry; // RAII
        archiveEntry.setPathnameW(archiveHeaderName());
        archiveEntry.setSize(encryptedByUserCredentialsHeaderString.size());
        archiveEntry.setFileType(EntryFileType::REGULAR);
        archiveEntry.setPermissions(0644);
        archive_write_header(arch_, archiveEntry.getEntry());
        auto bytesWritten = archive_write_data(arch_, encryptedByUserCredentialsHeaderString.c_str(), encryptedByUserCredentialsHeaderString.size());


        if(static_cast<size_t>(bytesWritten) != encryptedByUserCredentialsHeaderString.size()) {
            throw std::exception("[TarLibarchiveArchiever::writeArchiveHeader] error writing archive header");
        }
    }


    boost::filesystem::path TarLibarchiveArchiever::getPathOriginalFileName(boost::filesystem::path path, const ArchiveEntry &entry) {
        if(EntryFileType::REGULAR == entry.getFileType()) { // file entries have form like xxxx.yyy.tmp
            if(path.has_extension() && hasExtensionEquals(path, this->tempFileExtension())) {
                path.replace_extension();
            }
        }
        return path;
    }


    /// <summary>
    /// Adds the regular file entry into archive.
    /// </summary>
    /// <param name="relativePath">The relative path within archive.</param>
    /// <param name="pathName">Path of the object to add.</param>
    void TarLibarchiveArchiever::addRegularFileEntry(std::wstring &relativePath, const std::wstring &pathName, time_t mTime, DWORD attributes) { /*throw()*/
        /*throw()*/
        ArchiveEntry archiveEntry; // RAII

        std::wstring tempRelEncryptedFile = relativePath;
        tempRelEncryptedFile.append(tempFileExtension());

        // determine the path in which to store the temp filename
        wchar_t path[MAX_PATH];
        wcscpy_s(path, pathName.c_str());
        PathRemoveFileSpecW(path);

        // generate a guaranteed to be unique temporary filename to house the pending delete
        wchar_t tempName[MAX_PATH];
        if(!GetTempFileNameW(path, L".xX", 0, tempName)) {
            throw std::exception("Error creating temp file");
        }
        std::wstring tempFile(tempName);

        getFileEncryptor()->encrypt(pathName, encryptionKey_, tempFile);

        // NOTE: don't forget to close this file descriptor, we use C-style file IO to achieve performance on read/write
        // operations
        // pack file in archive

        // populate archive entry
        boost::filesystem::path tempPath(tempFile);
        auto tempFileSize = boost::filesystem::file_size(tempPath);
        auto origFileSize = boost::filesystem::file_size(pathName);
        archiveEntry.setSize(tempFileSize);
        archiveEntry.writeAttribute(fileSizeAttribute(), (void*)&origFileSize, sizeof(origFileSize));
        archiveEntry.writeFileAttributes(attributes);
        archiveEntry.setPathnameWithEncryption(tempRelEncryptedFile, getFileEncryptor());

        archiveEntry.setMtime(mTime);
        archiveEntry.setFileType(EntryFileType::REGULAR);
        archiveEntry.setPermissions(0664);
        archive_write_header(arch_, archiveEntry.getEntry());

        int fd = 0;
        auto ret = _wsopen_s(&fd, tempFile.c_str(),  _O_RDONLY | _O_BINARY, _SH_DENYRW, _S_IREAD);
        if(fd < 0 || 0 != ret) {
            // TODO: RAII (an.skornyakov@gmail.com)
            throw std::exception("Exception opening source file");
        }

        try {
            // buffer
            std::vector<char> buff; // use vector as static array
            buff.resize(kReadBufferSize_);

            auto len = _read(fd, buff.data(), buff.size());
            while(len > 0) {
                archive_write_data(arch_, buff.data(), static_cast<size_t>(len));
                len = _read(fd, buff.data(), buff.size());
            }
        } catch(const std::exception  &ex) {
            getLogger()->log(SeverityLevel::ERR, ex.what());
            _close(fd);
            throw;
        }
        _close(fd);

        if(!DeleteFileNow(tempFile.c_str())) {
            throw std::exception("Error deleting temp file");
        }
    }



    /// <summary>
    /// Adds the catalog entry in archive.
    /// </summary>
    /// <param name="relativePath">The relative path.</param>
    /// <param name="fSize">Size of the catalog.</param>
    void TarLibarchiveArchiever::addCatalogEntry(std::wstring &relativePath, int64_t fSize, time_t mTime, DWORD attributes) { /* throw()*/
        ArchiveEntry archiveEntry; // RAII
        archiveEntry.setPathnameWithEncryption(relativePath, getFileEncryptor());
        archiveEntry.writeFileAttributes(attributes);
        archiveEntry.setSize(fSize);
        archiveEntry.setMtime(mTime);
        archiveEntry.setPermissions(0664);
        archiveEntry.setFileType(EntryFileType::DIRECTORY);
        archive_write_header(arch_, archiveEntry.getEntry());
    }


    /// <summary>
    /// Adds item at specified path to archive recursively.
    /// </summary>
    /// <param name="pathName">Path.</param>
    /// <param name="archiveRelativePath">The archive relative path.</param>
    void TarLibarchiveArchiever::addToArchive(const std::wstring &pathName, const std::wstring &archiveRelativePath) {
        WIN32_FIND_DATA ffd;
        auto hFind = FindFirstFile(pathName.c_str(), &ffd);
        DWORD dwError = 0;
        if(INVALID_HANDLE_VALUE == hFind) {
            throw std::exception("Exception reading folder content");
        }
        FindClose(hFind);

        // set relative path
        std::wstring relativePath;
        if(!isRootSet_) {
            rootPath_.assign(ffd.cFileName);
            relativePath = rootPath_;
            isRootSet_ = true;
        } else {
            relativePath = archiveRelativePath;
        }

        int64_t fSize = ffd.nFileSizeHigh | (ffd.nFileSizeLow << 32);
        time_t mTime(boost::filesystem::last_write_time(pathName));
        if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) { // regular file
            progressBar_->setBarText(kCurrentPbIndex_, pathName);
            progressBar_->resetBarParams(kCurrentPbIndex_, kProgressBarDefaultMaxValue);
            this->addRegularFileEntry(relativePath, pathName , mTime, ffd.dwFileAttributes);
            progressBar_->setBarCurrentValue(kCurrentPbIndex_, kProgressBarDefaultMaxValue);
            if(totalFilesCount_ > 1) {
                progressBar_->addBarCurrentValue(kTotalPbIndex_, 1);
            } else {
                progressBar_->setBarCurrentValue(kTotalPbIndex_, kProgressBarDefaultMaxValue);
            }

            if(needOverwrite_) {
                DeleteFileNow(pathName.c_str());
            }
        } else if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { // dir
            addCatalogEntry(relativePath, fSize, mTime, ffd.dwFileAttributes);

            // run recursively for all objects in catalog
            std::wstring tmpStr(pathName);
            winAppendPathNode(tmpStr, L"*");
            hFind = FindFirstFile(tmpStr.c_str(), &ffd);

            if(INVALID_HANDLE_VALUE == hFind) {
                throw std::exception("Exception reading folder content");
            }

            do {
                // add all subs which are not "." or ".."
                if(wcsncmp(L".", ffd.cFileName, wcslen(ffd.cFileName)) != 0 && wcsncmp(L"..", ffd.cFileName, wcslen(ffd.cFileName)) != 0) {
                    std::wstring nextPath(pathName);
                    winAppendPathNode(nextPath, ffd.cFileName);
                    std::wstring nextArchiveRelativePath(relativePath);
                    winAppendPathNode(nextArchiveRelativePath, ffd.cFileName);

                    this->addToArchive(nextPath, nextArchiveRelativePath);
                }
            } while(FindNextFile(hFind, &ffd) != 0);

            dwError = GetLastError();
            if(dwError != ERROR_NO_MORE_FILES) {
                FindClose(hFind);
                throw std::exception("Exception reading folder content");
            }

            // TODO: implement RAII (an.skornyakov@gmail.com)
            FindClose(hFind);

            if(needOverwrite_) {
                if(!RemoveDirectory(pathName.c_str())) {
                    // LOG(ERROR) << "Error deleting original file: " << pathName.c_str();
                    std::ostringstream msg;
                    msg << "error deleting original file: " << pathName.c_str();
                    getLogger()->log(SeverityLevel::ERR, msg.str());
                }
            }
        }
    }


    /// <summary>
    /// Compresses the specified source path.
    /// </summary>
    /// <param name="srcPath">The source path.</param>
    /// <param name="destPath">The dest path.</param>
    void TarLibarchiveArchiever::startEncryption() {
        if(getSourcePaths().empty()) {
            throw std::logic_error("[TarLibarchiveArchiever::compress] source paths param array is empty");
        }
        if(getTargetPath().empty()) {
            throw std::logic_error("[TarLibarchiveArchiever::compress] destination path param string is empty");
        }

        if(!isWriteAble(getTargetPath().parent_path().wstring())) {
            MessageBox(NULL, L"Unable to start encryption procedure. Permission denied", NULL, MB_ICONERROR | MB_OK);
            throw std::logic_error("Access denied!");
        }

        totalFilesCount_ = pathsSumFilesCount(getSourcePaths());
        if(totalFilesCount_ > 1) {
            progressBar_->resetBarParams(kTotalPbIndex_, totalFilesCount_);
        } else {
            progressBar_->resetBarParams(kTotalPbIndex_, kProgressBarDefaultMaxValue);
        }

        progressBar_->setCaption(securingProgressBarMessage() + getTargetPath().wstring());
        progressBar_->setBarText(kTotalPbIndex_, getTargetPath().wstring());

        ArchiveWriter archiveWriter; // RAII
        arch_ = archiveWriter.getArchive();
        archiveWriter.open(getTargetPath().wstring());

        BiosecHeader header;
        header.totalFiles(totalFilesCount_);
        this->writeEncryptedArchiveHeader(header);

        auto sourcePaths = getSourcePaths();
        std::for_each(std::begin(sourcePaths), std::end(sourcePaths), [&](const std::shared_ptr<boost::filesystem::path> &pPath) {
            isRootSet_ = false;
            this->addToArchive((*pPath).wstring(), (*pPath).wstring());
        });

        archiveWriter.close();
        getLogger()->log(SeverityLevel::NORMAL, "Compressing done ");
    }


    /// <summary>
    /// Decompresses archive at specified source path.
    /// </summary>
    /// <param name="srcPath">The source path.</param>
    void TarLibarchiveArchiever::startDecryption() {
        auto sourcePaths = getSourcePaths();
        std::for_each(std::begin(sourcePaths), std::end(sourcePaths), [&](const std::shared_ptr<boost::filesystem::path> &thisPathPtr) {
            this->decompress_((*thisPathPtr).wstring());
        });

        return;
    }

    void TarLibarchiveArchiever::updateEntryPathname(const boost::filesystem::path &targetPath, ArchiveEntry &entry) {
        boost::filesystem::path newEntryPath;

        boost::filesystem::path entryPathInArchive(entry.getPathnameWithEncryption(getFileEncryptor()));
        entryPathInArchive = removeLastSignIfEqual(entryPathInArchive.c_str(), L'/');
        bool needUpdateRoot = !entryPathInArchive.has_parent_path();
        //if(!entryPathInArchive.has_parent_path()) {
        //    isRootSet_ = false;
        //}
        //if(isRootSet_) {
        //    newEntryPath = rootPath_;
        //    removeRootPath(&entryPathInArchive);
        //} else {
        //    newEntryPath = boost::filesystem::path(targetPath).parent_path();

        //}
		//if (entryPathInArchive.has_parent_path()) {
		//	auto rootDir = entryPathInArchive.begin()->wstring();
		//	if (PathFileExistsW((targetPath / rootDir).c_str())) {
		//		getSubstitudeRootPath()
		//	}

		//}
		entryPathInArchive = entryPathInArchive.filename();
		newEntryPath = targetPath.parent_path()/entryPathInArchive;


        if(entryPathInArchive.empty()) {
            throw std::exception("[TarLibarchiveArchieverImpl::updateEntryPathname] error reading entry pathname");
        }

        if(PathFileExistsW(getPathOriginalFileName(newEntryPath, entry).c_str())) {
            newEntryPath = this->getSubstitudePathname(newEntryPath, entry);
        }
		if (EntryFileType::DIRECTORY == entry.getFileType()) {
			producedCatalogsPaths_.insert(std::pair<std::wstring, std::wstring>(entry.getPathnameWithEncryption(getFileEncryptor()), newEntryPath.filename().wstring()));
		}

        // update current root path if initial path is root element in archive
        if(needUpdateRoot) {
            rootPath_ = newEntryPath.wstring();
            isRootSet_ = true;
        }

        entry.setPathnameW(newEntryPath.wstring());
    }


	boost::filesystem::path TarLibarchiveArchiever::produceEntryTargetPath(const std::wstring &srcPath, ArchiveEntry &entry, ExtractMode mode, const std::unordered_set<std::wstring> &pathsToDecrypt) {
        boost::filesystem::path thisPathInArchive(entry.getPathnameWithEncryption(getFileEncryptor()));
		auto finalPath = thisPathInArchive.filename();
		boost::filesystem::path prevPath;
		thisPathInArchive = thisPathInArchive.parent_path();
		boost::filesystem::path root;
		while (!thisPathInArchive.empty() && pathsToDecrypt.count(thisPathInArchive.wstring())){
			root = thisPathInArchive;
			prevPath = finalPath;
			finalPath = thisPathInArchive.filename() / finalPath;
			thisPathInArchive = thisPathInArchive.parent_path();
		}
		if (!needOverwrite_ && !root.empty()){
			auto testPath = boost::filesystem::path(srcPath).parent_path() / root;
			if (PathFileExistsW(testPath.c_str())) {
				if (this->producedCatalogsPaths_.count(root.wstring()) == 0){
					auto substPath = getSubstitudeRootPath(testPath);
					finalPath = substPath.filename() / prevPath;
					producedCatalogsPaths_.insert(std::pair<std::wstring,std::wstring>(root.wstring(),substPath.filename().wstring()));
				}
				else{
					finalPath = producedCatalogsPaths_[root.wstring()]/prevPath;
				}
			}
		}
		//if (mode == FILE_ONLY){
		//	thisPathInArchive = thisPathInArchive.filename();
		//}
		//if (PathFileExistsW(getPathOriginalFileName(newEntryPath, entry).c_str())) {
		//	newEntryPath = this->getSubstitudePathname(newEntryPath, entry);
		//}
		auto targetPath = this->getPathOriginalFileName(boost::filesystem::path(srcPath).parent_path() /= finalPath, entry);
        targetPath = removeLastSignIfEqual(targetPath.wstring(), L'/');
        return targetPath;
    }


    void TarLibarchiveArchiever::decryptEntry(const ArchiveEntry &entry) {
        if(EntryFileType::REGULAR == entry.getFileType()) {
            auto internalFilePath = entry.getPathnameW();
            boost::filesystem::path realFilePath(internalFilePath);
            realFilePath.replace_extension();
            realFilePath.make_preferred();

            producedPaths_.push_back(realFilePath);

            progressBar_->setBarText(kCurrentPbIndex_, realFilePath.wstring());
            progressBar_->resetBarParams(kCurrentPbIndex_, kProgressBarDefaultMaxValue);
            getFileEncryptor()->decrypt(internalFilePath, encryptionKey_, realFilePath.wstring());
            auto attrWasSet = SetFileAttributes(realFilePath.c_str(), entry.readFileAttributes());
            if(!attrWasSet) {
                getLogger()->log(SeverityLevel::ERR, std::string() + "can't set attributes for entry " + realFilePath.string());
            }
            if(!DeleteFileNow(internalFilePath.c_str())) {
                throw std::exception("Error deleting temp file");
            }

            if(totalFilesCount_ > 1) {
                progressBar_->addBarCurrentValue(kTotalPbIndex_, 1);
            } else {
                progressBar_->setBarCurrentValue(kTotalPbIndex_, kProgressBarDefaultMaxValue);
            }

        } else if(EntryFileType::DIRECTORY == entry.getFileType()) {
            boost::filesystem::path realFilePath(entry.getPathnameW());
			//producedCatalogsPaths_.emplace(realFilePath.wstring());
			getLogger()->log(blm_utils::SeverityLevel::NOTIFICATION, std::to_string(producedCatalogsPaths_.size()) + " " + realFilePath.string());
			realFilePath.make_preferred();
            auto attrWasSet = SetFileAttributes(realFilePath.c_str(), entry.readFileAttributes());
        }
    }


    void TarLibarchiveArchiever::decompressFromList_(const std::wstring &srcPath, const std::unordered_set<std::wstring> &pathsToDecrypt) {
        // archive to decompress from
        ArchiveReader archiveReader; // RAII
        arch_ = archiveReader.getArchive();

        // archive(file) to decompress to
        ArchiveDiskWriter archiveDiskWriter; // RAII
        struct archive* ext = archiveDiskWriter.getArchive();

        archiveReader.open(srcPath);


        auto nextEntry = archiveReader.getNextEntry();
        while(nextEntry) {
			auto howToExtract = this->howToExtractEntry(*nextEntry, pathsToDecrypt);
			if (howToExtract != NONE) {
//				howToExtract = NORMAL;
				auto targetPath = this->produceEntryTargetPath(srcPath, *nextEntry, howToExtract, pathsToDecrypt);
				if (needOverwrite_) {
                    removePathIfNotDirectory(targetPath);

                    if(EntryFileType::REGULAR == nextEntry->getFileType()) {
                        boost::filesystem::path tempEncryptedFilePath(srcPath);
						boost::filesystem::path entryPath(nextEntry->getPathnameWithEncryption(getFileEncryptor()));
						//if (howToExtract == FILE_ONLY){
						//	entryPath = entryPath.filename();
						//}
                        tempEncryptedFilePath = tempEncryptedFilePath.parent_path() / entryPath;
                        nextEntry->setPathnameW(tempEncryptedFilePath.wstring());

                        archiveDiskWriter.writeHeader(*nextEntry);
                        copy_data(arch_, ext);
                        archiveDiskWriter.finishEntry();

                        if(!hasResultRootPath_) {
                            producedPaths_.push_back(boost::filesystem::path(nextEntry->getPathnameW()));
                            hasResultRootPath_ = true;
                        }

                        this->decryptEntry(*nextEntry);

                    } else if(EntryFileType::DIRECTORY == nextEntry->getFileType()) {
                        try {
                            this->extractCatalogEntryIfNotExists(targetPath, *nextEntry);
                        } catch(const std::exception &ex) {
                            getLogger()->log(SeverityLevel::ERR, ex.what());
                        }
                    }

                }  else {
					
                    this->updateEntryPathname(targetPath, *nextEntry);

                    archiveDiskWriter.writeHeader(*nextEntry);
                    copy_data(arch_, ext);
                    archiveDiskWriter.finishEntry();

                    if(!hasResultRootPath_) {
                        producedPaths_.push_back(boost::filesystem::path(nextEntry->getPathnameW()));
                        hasResultRootPath_ = true;
                    }

                    this->decryptEntry(*nextEntry);

                }


            }
            nextEntry = archiveReader.getNextEntry();
        }
    }


    bool TarLibarchiveArchiever::needToExtractEntry(const ArchiveEntry &entry, const std::unordered_set<std::wstring> &pathsToDecrypt) {
        bool res = false;
        if(!isHeader(entry)) {

            boost::filesystem::path entryDecryptedPathname(entry.getPathnameWithEncryption(getFileEncryptor()));
            std::wstring enrtyOriginalPathnameStr(getPathOriginalFileName(entryDecryptedPathname, entry).wstring());

            boost::algorithm::trim_right_if(enrtyOriginalPathnameStr, [&](wchar_t endChar) {
                return endChar == L'/';
            });

            boost::filesystem::path enrtyOriginalPathname(enrtyOriginalPathnameStr);

            auto found  = pathsToDecrypt.find(enrtyOriginalPathname.wstring());
            return (found != pathsToDecrypt.end());

        } else {
            res = false;
        }

        return res;
    }

	TarLibarchiveArchiever::ExtractMode TarLibarchiveArchiever::howToExtractEntry(const ArchiveEntry &entry, const std::unordered_set<std::wstring> &pathsToDecrypt) {
		TarLibarchiveArchiever::ExtractMode res = NONE;
		if (!isHeader(entry)) {

			boost::filesystem::path entryDecryptedPathname(entry.getPathnameWithEncryption(getFileEncryptor()));
			std::wstring enrtyOriginalPathnameStr(getPathOriginalFileName(entryDecryptedPathname, entry).wstring());

			boost::algorithm::trim_right_if(enrtyOriginalPathnameStr, [&](wchar_t endChar) {
				return endChar == L'/';
			});

			boost::filesystem::path enrtyOriginalPathname(enrtyOriginalPathnameStr);
			auto found = pathsToDecrypt.find(enrtyOriginalPathname.wstring());
			if (found != pathsToDecrypt.end()){
				std::wstring ppath = enrtyOriginalPathname.parent_path().wstring();
				auto parentFound = pathsToDecrypt.find(ppath);

				res = NORMAL;
				if (parentFound != pathsToDecrypt.end()){
					res = NORMAL;
				}
				else{
					res = FILE_ONLY;
				}
			}
		}

		return res;
	}


    void TarLibarchiveArchiever::decompress_(const std::wstring &srcPath) {
        try {

            // init progress bar, which will be updated during decompression operation, run decompression then close progress bar
            // if decompression fails set flag determines if need to delete created files
            progressBar_->setCaption(unsecuringProgressBarMessage() + srcPath);
            progressBar_->setBarText(kTotalPbIndex_, srcPath);

            std::unordered_set<std::wstring> filesToDecrypt;
            totalFilesCount_ = 0;
            std::for_each(std::begin(itemsToDecompress_), std::end(itemsToDecompress_), [&](std::uint64_t thisItemIndex) {
                if(thisItemIndex < archiveContentList_.size()) {
                    // insert all paths and subpaths
                    auto path = boost::filesystem::path(archiveContentList_[thisItemIndex].fileName);

					filesToDecrypt.insert(path.wstring());
					//while (path.has_filename()) {
     //                   filesToDecrypt.insert(path.wstring());
     //                   if(path.has_parent_path()) {
     //                       path = path.parent_path();
     //                   } else {
     //                       break;
     //                   }
     //               }

                    if(EntityType::REGULAR == archiveContentList_[thisItemIndex].type) {
                        ++totalFilesCount_;
                    }
                }
            });

            if(totalFilesCount_ > 1) {
                progressBar_->resetBarParams(kTotalPbIndex_, totalFilesCount_);
            } else {
                progressBar_->resetBarParams(kTotalPbIndex_, kProgressBarDefaultMaxValue);
            }

            this->decompressFromList_(srcPath, filesToDecrypt);
        } catch(const std::exception &ex) {
            shouldDeleteAllCreatedFiles_ = true;
            throw;
        }
    }


    void TarLibarchiveArchiever::getArchiveContent(const std::wstring &archPath) {
        ArchiveReader archiveReader; // RAII
        archiveReader.open(archPath);

        auto nextEntry = archiveReader.getNextEntry();
        while(nextEntry) {
            if(!isHeader(*nextEntry)) {

                // TODO (an.skornyakov@gmail.com): extract method (start)
                std::wstring decryptedItemName(nextEntry->getPathnameWithEncryption(getFileEncryptor()));
                ArchiveEntryInfo fileInfo;
                fileInfo.fileName = getPathOriginalFileName(decryptedItemName, *nextEntry).wstring();
                fileInfo.fileSize = 0;
                fileInfo.mtime = nextEntry->getMtime();
                if(EntryFileType::REGULAR == nextEntry->getFileType()) {
                    auto sAttr = nextEntry->readAttribute(fileSizeAttribute());
                    if(!sAttr.empty()) {
                        fileInfo.fileSize = *(std::uintmax_t*)sAttr.data();
                    }
                    fileInfo.type = EntityType::REGULAR;
                } else {
                    fileInfo.type = EntityType::FOLDER;
                }
                // TODO (an.skornyakov@gmail.com): extract method (end)

                archiveContentList_.push_back(fileInfo);
            }

            nextEntry = archiveReader.getNextEntry();
        }
    }


    bool TarLibarchiveArchiever::isHeader(const ArchiveEntry &entry) {
        return (0 == entry.getPathnameW().compare(archiveHeaderName()));
    }



    void TarLibarchiveArchiever::setTargetDirectory(const std::wstring &targetDirectory) {
        setTargetPath(targetDirectory);
    }


    std::wstring TarLibarchiveArchiever::getBiosecureFilePath() {
        throw NotImplemented();
    }


    void TarLibarchiveArchiever::reportProgress(void* reporter, std::int64_t done, std::int64_t total) {
        if(done <= total) {
            if(0 == total) {
                total = 1;
            }

            float floatVal = static_cast<float>(done);
            floatVal = floatVal  * kCurrentBarEncryptingExtent / total;
            int32_t pbVal = static_cast<int32_t>(floatVal);
            progressBar_->setBarCurrentValue(kCurrentPbIndex_, pbVal);
            if(totalFilesCount_ == 1) {
                progressBar_->setBarCurrentValue(kTotalPbIndex_, pbVal);
            }
        } else {
            std::ostringstream msg;
            msg << "error reported progress: done " <<  done << " from " << total;
            getLogger()->log(SeverityLevel::ERR, msg.str());
        }
    }


    boost::filesystem::path TarLibarchiveArchiever::getSubstitudePathname(const boost::filesystem::path &initialPath, const ArchiveEntry &entry) {
        // make several decompositions with entry pathname using boost::filesystem
        auto parentPath = initialPath.parent_path();
        auto filename = initialPath.filename();
        boost::filesystem::path stem;
        std::wstring fusedExtensions;
        if(EntryFileType::DIRECTORY == entry.getFileType()) {
            stem = filename.stem();
            fusedExtensions = filename.extension().wstring();
        } else if(EntryFileType::REGULAR == entry.getFileType()) {
            stem = getPathStem(filename);
            if(initialPath.has_extension()) {
                fusedExtensions = getPathExtensions(filename);
            }
        }
        boost::filesystem::path prefixPath = parentPath;


        // produce new path name with form  "original_name(i)" which not exists and update entry with it
        std::wostringstream newEntryPathnameStrStream;
        int i = 1;
        do {
            std::wostringstream clear;
            newEntryPathnameStrStream.swap(clear);
            newEntryPathnameStrStream << prefixPath.wstring();
            newEntryPathnameStrStream << L"\\" << stem.wstring();
            newEntryPathnameStrStream << "(" << i << ")";
            if(initialPath.has_extension()) {
                newEntryPathnameStrStream << fusedExtensions;
            }
            i++;
        } while(PathFileExistsW(getPathOriginalFileName(newEntryPathnameStrStream.str(), entry).c_str()));
        boost::filesystem::path substitudePathname(newEntryPathnameStrStream.str());
        return substitudePathname;
    }

	boost::filesystem::path TarLibarchiveArchiever::getSubstitudeRootPath(const boost::filesystem::path &initialPath) {
		// make several decompositions with entry pathname using boost::filesystem
		auto parentPath = initialPath.parent_path();
		auto filename = initialPath.filename();
		boost::filesystem::path stem;
		std::wstring fusedExtensions;
		stem = filename.stem();
		fusedExtensions = filename.extension().wstring();

		boost::filesystem::path prefixPath = parentPath;


		// produce new path name with form  "original_name(i)" which not exists and update entry with it
		std::wostringstream newEntryPathnameStrStream;
		int i = 1;
		do {
			std::wostringstream clear;
			newEntryPathnameStrStream.swap(clear);
			newEntryPathnameStrStream << prefixPath.wstring();
			newEntryPathnameStrStream << L"\\" << stem.wstring();
			newEntryPathnameStrStream << "(" << i << ")";
			if (initialPath.has_extension()) {
				newEntryPathnameStrStream << fusedExtensions;
			}
			i++;
		} while (PathFileExistsW(newEntryPathnameStrStream.str().c_str()));
		boost::filesystem::path substitudePathname(newEntryPathnameStrStream.str());
		return substitudePathname;
	}

	IArchiever::Result TarLibarchiveArchiever::putSecretKey(const std::vector<std::uint8_t> &data) {
		return getArchiveContent(data);
	}

    IArchiever::Result TarLibarchiveArchiever::putBiometric(const std::wstring &biometricType, const std::vector<std::uint8_t> &data) {
        if(biometricDbMaster_.matchAgainstBiometric(biometricType, data)) {
            return verifyBiometric();
			//return getArchiveContent();
        } else {
            return Result::BIOMETRIC_MATCH_NOT_FOUND;
        }
    }


    std::wstring TarLibarchiveArchiever::getUserName() {
        auto biometricOwner = biometricDbMaster_.getBiometricOwnerName();
        return std::wstring(biometricOwner.data(), biometricOwner.size());
    }


    void TarLibarchiveArchiever::setItemsToDecompress(const std::vector<std::uint64_t> &items) {
        itemsToDecompress_ = items;
    }


    IArchiever::ArchiveEntryQueryRes TarLibarchiveArchiever::getArchiveEntryInfoAt(ArchiveEntryInfo* queryInfo, std::uint32_t index) {
        if(nullptr == queryInfo) {
            throw std::logic_error("null pointer argument queryInfo");
        }

        if(index >= archiveContentList_.size()) {
            return IArchiever::ArchiveEntryQueryRes::OUT_OF_RANGE;
        }


        *queryInfo = archiveContentList_[index];

        return IArchiever::ArchiveEntryQueryRes::SUCCESS;
    }


    void TarLibarchiveArchiever::setProgressUpdate(const std::function<void(const wchar_t*, const wchar_t*, uint32_t, uint32_t, uint32_t, uint32_t)> &progressUpdate) {
        if(!progressBar_) {
            progressBar_.reset(new ProgressBarWindow(progressUpdate));
        } else {
            progressBar_->updateProgressFn(progressUpdate);
        }
    }


    void TarLibarchiveArchiever::extractCatalogEntryIfNotExists(const boost::filesystem::path &catalogPathname, const ArchiveEntry &entry) {
        if(!boost::filesystem::exists(catalogPathname)) {
            auto catalogCreated = boost::filesystem::create_directory(catalogPathname);
            if(!catalogCreated) {
                getLogger()->log(SeverityLevel::ERR, std::string() + "can't create directory " + catalogPathname.string());
                throw std::logic_error("error creating catalog");
                return;
            }

            auto attributesWasSet = SetFileAttributes(catalogPathname.c_str(), entry.readFileAttributes());
            if(!attributesWasSet) {
                getLogger()->log(SeverityLevel::ERR, std::string() + "can't set attributes for catalog " + catalogPathname.string());
                throw std::logic_error("error setting catalog attributes");
            }
        }
    }

	IArchiever::Result TarLibarchiveArchiever::setEncryptionKey(const std::vector<std::uint8_t> &data) {
		encryptionKey_.reserve(data.size());
		std::copy(std::begin(data), std::end(data), std::back_inserter(encryptionKey_));
		return Result::BIOMETRIC_ACCEPTED;
	}

	IArchiever::Result TarLibarchiveArchiever::getArchiveContent(const std::vector<std::uint8_t> &data) {
		

		encryptionKey_.reserve(data.size());
		std::copy(std::begin(data), std::end(data), std::back_inserter(encryptionKey_));

		auto isKeyNotSuitable = false;
		auto sourcePaths = getSourcePaths();
		switch (getType()) {
		case Type::INVALID_TYPE:
			throw std::logic_error("Invalid mode, not initialized?");
			break;
		case Type::TAR_ENCRYPTOR:
			encryptionKey_ = biometricDbMaster_.getEncryptionKey();
			return Result::BIOMETRIC_ACCEPTED;
			break;
		case Type::TAR_DECRYPTOR:
			///111

			isKeyNotSuitable = std::any_of(std::begin(sourcePaths),
				std::end(sourcePaths),
				[&](const std::shared_ptr<boost::filesystem::path> &thisPathPtr) {
				return !(this->isEncryptionKeyValidForArchive((*thisPathPtr).wstring()));
			});

			if (isKeyNotSuitable) {
				return Result::BIOMETRIC_INVALID_ENCRYPTION_KEY;
			}

			this->readSourceArchivesContent();

			return Result::BIOMETRIC_ACCEPTED;
			break;
		default:
			throw std::logic_error("Unknown mode, forgot to add");
			break;
		}

	}

	IArchiever::Result TarLibarchiveArchiever::getArchiveContent() {
		//encryptionKey_ = data;
		std::vector<uint8_t> biometricOwnerEncryptionKey;
		auto isKeyNotSuitable = false;
		auto sourcePaths = getSourcePaths();
		switch (getType()) {
		case Type::INVALID_TYPE:
			throw std::logic_error("Invalid mode, not initialized?");
			break;
		case Type::TAR_ENCRYPTOR:
			encryptionKey_ = biometricDbMaster_.getEncryptionKey();
			return Result::BIOMETRIC_ACCEPTED;
			break;
		case Type::TAR_DECRYPTOR:
			///111
			
			encryptionKey_ = biometricDbMaster_.getEncryptionKey();
			//std::copy(std::begin(encryptionKey1_), std::end(encryptionKey1_), std::back_inserter(myKey));
		/*	readfile("C:\\Logs\\myKey.txt", myKey);


			encryptionKey_.reserve(myKey.length());
			std::copy(myKey.begin(), myKey.end(),
				std::back_inserter(encryptionKey_));*/

			isKeyNotSuitable = std::any_of(std::begin(sourcePaths),
				std::end(sourcePaths),
				[&](const std::shared_ptr<boost::filesystem::path> &thisPathPtr) {
				return !(this->isEncryptionKeyValidForArchive((*thisPathPtr).wstring()));
			});

			if (isKeyNotSuitable) {
				return Result::BIOMETRIC_INVALID_ENCRYPTION_KEY;
			}

			this->readSourceArchivesContent();

			return Result::BIOMETRIC_ACCEPTED;
			break;
		default:
			throw std::logic_error("Unknown mode, forgot to add");
			break;
		}

	}

    IArchiever::Result TarLibarchiveArchiever::verifyBiometric() {
        auto biometricOwnerSid = biometricDbMaster_.getBiometricOwnerSid();
        if(0 != biometricOwnerSid.compare(GetCurrentUserSid())) {

            return Result::BIOMETRIC_NOT_BELONGS_TO_USER;
        }

        std::vector<uint8_t> biometricOwnerEncryptionKey;
        auto isKeyNotSuitable = false;
        auto sourcePaths = getSourcePaths();
        switch(getType()) {
            case Type::INVALID_TYPE:
                throw std::logic_error("Invalid mode, not initialized?");
                break;
            case Type::TAR_ENCRYPTOR:
                encryptionKey_ = biometricDbMaster_.getEncryptionKey();
                return Result::BIOMETRIC_ACCEPTED;
                break;
            case Type::TAR_DECRYPTOR:
                encryptionKey_ = biometricDbMaster_.getEncryptionKey();

                isKeyNotSuitable = std::any_of(std::begin(sourcePaths),
                                               std::end(sourcePaths),
                [&](const std::shared_ptr<boost::filesystem::path> &thisPathPtr) {
                    return !(this->isEncryptionKeyValidForArchive((*thisPathPtr).wstring()));
                });

                if(isKeyNotSuitable) {
                    return Result::BIOMETRIC_INVALID_ENCRYPTION_KEY;
                }

                this->readSourceArchivesContent();

                return Result::BIOMETRIC_ACCEPTED;
                break;
            default:
                throw std::logic_error("Unknown mode, forgot to add");
                break;
        }

    }


    IArchiever::Result TarLibarchiveArchiever::isUserEnrolled() {
        if(!biometricDbMaster_.isUserEnrolled(GetCurrentUserSid())) {
            return IArchiever::Result::USER_NOT_ENROLLED;
        }
        return IArchiever::Result::SUCCESS;
    }



    void TarLibarchiveArchiever::readSourceArchivesContent() {
        archiveContentList_.clear();
        auto sourcePaths = getSourcePaths();
        std::for_each(std::begin(sourcePaths), std::end(sourcePaths), [&](const std::shared_ptr<boost::filesystem::path> &thisPathPtr) {
            this->getArchiveContent((*thisPathPtr).wstring());
        });
    }



    /// <summary>
    /// Copy_data from one archive to another
    /// </summary>
    /// <param name="ar">archive to read.</param>
    /// <param name="aw">archive to write</param>
    /// <returns></returns>
    static int copy_data(struct archive* ar, struct archive* aw) {
        const void* buff;
        size_t size;
        int64_t offset;

        for(;;) {
            int r = archive_read_data_block(ar, &buff, &size, &offset);
            if(r == ARCHIVE_EOF) {
                return (ARCHIVE_OK);
            }
            if(r != ARCHIVE_OK) {
                throw std::exception(archive_error_string(ar));
            }

            r = archive_write_data_block(aw, buff, size, offset);
            if(r != ARCHIVE_OK) {
                throw std::exception(archive_error_string(aw));
            }
        }
    }





} // namespace


















