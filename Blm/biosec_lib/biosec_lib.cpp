// biosec_lib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <algorithm>
#include <memory>
#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "archieverFactory.h"
#include "fileEncryptor.h"
#include "common.h"
#include "biosec_lib.h"

#pragma comment(lib, "shlwapi.lib")


static std::unique_ptr<blm_utils::IArchiever> archiever;
static std::mutex archieverMtx;
static std::unique_ptr<blm_utils::SeverityLogger> logger;

uint8_t __stdcall checkIfActiveUserEnrolled();


BIOSECDLL_API uint8_t __stdcall init(uint32_t mode) {
    try {
        std::unique_lock<std::mutex> lock(archieverMtx, std::try_to_lock);
        logger.reset(new blm_utils::SeverityLogger);
        if(lock.owns_lock()) {

            blm_utils::IArchiever::Type type;
            switch(mode) {
                case Mode::ENCRYPTION:
                    type = blm_utils::IArchiever::Type::TAR_ENCRYPTOR;
                    break;
                case Mode::DECRYPTION:
                    type = blm_utils::IArchiever::Type::TAR_DECRYPTOR;
                    break;
                default:
                    logger->log(blm_utils::SeverityLevel::ERR, "Unknown mode");
                    return RetCode::BIOSEC_INVALID_ARG;
                    break;
            }

            archiever = blm_utils::ArchieverFactory::create(type);
            return checkIfActiveUserEnrolled();

        } else {
            return RetCode::BIOSEC_BUSY;
        }

    } catch(const std::exception &ex) {
        logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
        return RetCode::BIOSEC_INTERNAL_ERROR;
    } catch(...) {
        logger->log(blm_utils::SeverityLevel::CRITICAL);
        return RetCode::BIOSEC_UNKNOWN_ERROR;
    }
    return RetCode::BIOSEC_NO_ERROR;
}


BIOSECDLL_API uint8_t __stdcall release() {
    try {
        //std::unique_lock<std::mutex> lock(archieverMtx, std::try_to_lock);
        std::unique_lock<std::mutex> lock(archieverMtx); //, std::try_to_lock);

        if(lock.owns_lock()) {
            if(archiever) {
                archiever.reset(nullptr);
                return RetCode::BIOSEC_NO_ERROR;
            } else {
                logger->log(blm_utils::SeverityLevel::ERR, "Library not inited");
                return RetCode::BIOSEC_LIB_NOT_INITED;
            }
        } else {
            return RetCode::BIOSEC_BUSY;
        }
    } catch(const std::exception &ex) {
        logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
        return RetCode::BIOSEC_INTERNAL_ERROR;
    } catch(...) {
        logger->log(blm_utils::SeverityLevel::CRITICAL);
        return RetCode::BIOSEC_UNKNOWN_ERROR;
    }
}


BIOSECDLL_API uint8_t __stdcall setSourcePathList(const sourcePathInfo* pathList, uint64_t size) {

    try {
        std::unique_lock<std::mutex> lock(archieverMtx, std::try_to_lock);

        if(lock.owns_lock()) {
            if(archiever) {
                std::vector<blm_utils::SourcePathInfo> sources;
                for(auto i = 0; i < size; ++i) {
                    sources.emplace_back(pathList[i].pathName);
                }
                auto res = archiever->setSourcePathlist(sources);
                switch(res) {
                    case blm_utils::IArchiever::Result::SUCCESS:
                        return RetCode::BIOSEC_NO_ERROR;
                        break;
                    case blm_utils::IArchiever::Result::USER_NOT_AN_ARCHIVE_OWNER:
                        logger->log(blm_utils::SeverityLevel::ERR, "user not an archive owner");
                        return RetCode::BIOSEC_USER_NOT_AN_ARCHIVE_OWNER;
                        break;
                    default:
                        logger->log(blm_utils::SeverityLevel::ERR, "unknown error");
                        return RetCode::BIOSEC_UNKNOWN_ERROR;
                        break;
                }

                return RetCode::BIOSEC_NO_ERROR;
            } else {
                logger->log(blm_utils::SeverityLevel::ERR, "library not inited");
                return RetCode::BIOSEC_LIB_NOT_INITED;
            }
        } else {
            return RetCode::BIOSEC_BUSY;
        }

    } catch(const std::exception &ex) {
        logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
        return RetCode::BIOSEC_INTERNAL_ERROR;
    } catch(...) {
        logger->log(blm_utils::SeverityLevel::CRITICAL);
        return RetCode::BIOSEC_UNKNOWN_ERROR;
    }
}


BIOSECDLL_API uint8_t __stdcall setTargetDir(const wchar_t* path) {
    try {
        std::unique_lock<std::mutex> lock(archieverMtx, std::try_to_lock);

        if(lock.owns_lock()) {
            if(archiever) {
                archiever->setTargetDirectory(path);
                return BIOSEC_NO_ERROR;
            } else {
                logger->log(blm_utils::SeverityLevel::ERR, "library not inited");
                return BIOSEC_LIB_NOT_INITED;
            }
        } else {
            return BIOSEC_BUSY;
        }

    } catch(const std::exception &ex) {
        logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
        return RetCode::BIOSEC_INTERNAL_ERROR;
    } catch(...) {
        logger->log(blm_utils::SeverityLevel::CRITICAL);
        return RetCode::BIOSEC_UNKNOWN_ERROR;
    }
}


// return error code if index out of range
BIOSECDLL_API uint8_t __stdcall getCryptoProviders(uint32_t index, wchar_t* buffer, uint32_t bufferLength) {

    try {
        std::unique_lock<std::mutex> lock(archieverMtx, std::try_to_lock);

        if(!lock.owns_lock()) {
            return BIOSEC_BUSY;
        }

        auto providersAvailable = blm_utils::getAvailableCryptoProviders();
        if(index > providersAvailable.size() - 1) {
            return BIOSEC_INDEX_OUT_OF_RANGE;
        }

        wcsncpy_s(buffer, bufferLength,  providersAvailable[index].c_str(), providersAvailable[index].size());
        return BIOSEC_NO_ERROR;

    } catch(const std::exception &ex) {
        logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
        return RetCode::BIOSEC_INTERNAL_ERROR;
    } catch(...) {
        logger->log(blm_utils::SeverityLevel::CRITICAL);
        return RetCode::BIOSEC_UNKNOWN_ERROR;
    }
}


BIOSECDLL_API uint8_t __stdcall setCryptoProvider(const wchar_t* provider) {
    try {
        std::unique_lock<std::mutex> lock(archieverMtx, std::try_to_lock);

        if(!lock.owns_lock()) {
            return BIOSEC_BUSY;
        }
        if(!archiever) {
            logger->log(blm_utils::SeverityLevel::ERR, "library not inited");
            return BIOSEC_LIB_NOT_INITED;
        }

        archiever->setCryptoProvider(provider) ;
        return BIOSEC_NO_ERROR;

    } catch(const std::exception &ex) {
        logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
        return RetCode::BIOSEC_INTERNAL_ERROR;
    } catch(...) {
        logger->log(blm_utils::SeverityLevel::CRITICAL);
        return RetCode::BIOSEC_UNKNOWN_ERROR;
    }
}

BIOSECDLL_API uint8_t __stdcall putSecretKeyForArchive(uint8_t* data, uint64_t size, biometricProcesses fpBiometricProcessed) {

	std::vector<uint8_t> dataVec(data, data + size);
	auto ret = archiever->getArchiveContent(dataVec);
	switch (ret)
	{
		case blm_utils::IArchiever::Result::BIOMETRIC_ACCEPTED
			:return RetCode::BIOSEC_NO_ERROR;
			break;
	}
	return RetCode::BIOSEC_INTERNAL_ERROR;
}

BIOSECDLL_API uint8_t __stdcall putSecretKey(uint8_t* data, uint64_t size, biometricProcesses fpBiometricProcessed) {

	std::vector<uint8_t> dataVec(data, data + size);
	auto ret = archiever->setEncryptionKey(dataVec);
	switch (ret) 
	{
		case blm_utils::IArchiever::Result::BIOMETRIC_ACCEPTED
			:return RetCode::BIOSEC_NO_ERROR;
		break;
	}
	return RetCode::BIOSEC_INTERNAL_ERROR;
}

BIOSECDLL_API uint8_t __stdcall putBiometric(const wchar_t* biometricType, const uint8_t* data, uint64_t size, biometricProcesses fpBiometricProcessed) {

    try {

        std::mutex mtx;
        std::condition_variable cv;
        bool archieverLocked(false);
        bool archieverInited(true);
        bool lockResultAvailable(false);

        std::vector<uint8_t> biometricDataVec(data, data + size);
        std::wstring biometricTypeStr(biometricType);

        auto task = [ &, biometricDataVec, biometricTypeStr](biometricProcesses fpBiometricProcessed) {
            try {
                std::unique_lock<std::mutex> cvLock(mtx);
                std::unique_lock<std::mutex> lk(archieverMtx, std::try_to_lock);
                if(!lk.owns_lock()) {
                    archieverLocked = false;
                    lockResultAvailable = true;
                    cv.notify_one();
                    return;
                }

                archieverLocked = true;
                if(!archiever) {
                    archieverInited = false;
                    lockResultAvailable = true;
                    cv.notify_one();
                    return;
                }

                lockResultAvailable = true;
                archieverInited = true;
                cvLock.unlock();
                cv.notify_one();



                auto ret = archiever->putBiometric(biometricTypeStr, biometricDataVec);
                lk.unlock();

                switch(ret) {
                    case blm_utils::IArchiever::Result::BIOMETRIC_ACCEPTED:
                        fpBiometricProcessed(BiometricProcessingRes::BIOMETRIC_ACCEPTED);
                        break;
                    case blm_utils::IArchiever::Result::BIOMETRIC_MATCH_NOT_FOUND:
                        fpBiometricProcessed(BiometricProcessingRes::BIOMETRIC_MATCH_NOT_FOUND);
                        break;
                    case blm_utils::IArchiever::Result::BIOMETRIC_NOT_BELONGS_TO_USER:
                        fpBiometricProcessed(BiometricProcessingRes::BIOMETRIC_NOT_BELOGNS_TO_USER);
                        break;
                    case blm_utils::IArchiever::Result::BIOMETRIC_INVALID_ENCRYPTION_KEY:
                        fpBiometricProcessed(BiometricProcessingRes::BIOMETRIC_INVALID_ENCRYPTION_KEY);
                        break;
                    default:
                        logger->log(blm_utils::SeverityLevel::ERR, "internal error");
                        fpBiometricProcessed(BiometricProcessingRes::BIOMETRIC_INTERNAL_ERROR);
                        break;
                }

            } catch(const std::exception &ex) {
                logger->log(blm_utils::SeverityLevel::ERR, "internal error");
                fpBiometricProcessed(BiometricProcessingRes::BIOMETRIC_INTERNAL_ERROR);
            } catch(...) {
                logger->log(blm_utils::SeverityLevel::ERR, "unknown error");
                fpBiometricProcessed(BiometricProcessingRes::BIOMETRIC_UNKNOWN_ERROR);
            }
        };

        std::unique_lock<std::mutex> cvLock(mtx);
        std::thread biometricAnalysis(task, fpBiometricProcessed);
        biometricAnalysis.detach();

        cv.wait(cvLock, [&]() {
            return lockResultAvailable;
        });

        if(!archieverLocked) {
            return RetCode::BIOSEC_BUSY;
        }

        if(!archieverInited) {
            logger->log(blm_utils::SeverityLevel::ERR, "library not inited");
            return RetCode::BIOSEC_LIB_NOT_INITED;
        }

    } catch(const std::exception &ex) {
        logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
        return RetCode::BIOSEC_INTERNAL_ERROR;
    } catch(...) {
        logger->log(blm_utils::SeverityLevel::CRITICAL);
        return RetCode::BIOSEC_UNKNOWN_ERROR;
    }

    return BIOSEC_NO_ERROR;
}


BIOSECDLL_API uint8_t __stdcall setOverwriteFlagValue(bool needOverwrite) {
    try {
        std::unique_lock<std::mutex> lock(archieverMtx, std::try_to_lock);

        if(!lock.owns_lock()) {
            return BIOSEC_BUSY;
        }
        if(!archiever) {
            logger->log(blm_utils::SeverityLevel::ERR, "library not inited");
            return BIOSEC_LIB_NOT_INITED;
        }

        archiever->needOverwrite(needOverwrite) ;
        return BIOSEC_NO_ERROR;

    } catch(const std::exception &ex) {
        logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
        return RetCode::BIOSEC_INTERNAL_ERROR;
    } catch(...) {
        logger->log(blm_utils::SeverityLevel::CRITICAL);
        return RetCode::BIOSEC_UNKNOWN_ERROR;
    }
}


BIOSECDLL_API uint8_t __stdcall getUserName(wchar_t* name, uint64_t bufferLength) {

    try {
        std::unique_lock<std::mutex> lock(archieverMtx, std::try_to_lock);
        if(!lock.owns_lock()) {
            return BIOSEC_BUSY;
        }

        if(!archiever) {
            return BIOSEC_LIB_NOT_INITED;
        }

        auto userName = archiever->getUserName() ;
        wcsncpy_s(name, static_cast<std::size_t>(bufferLength), userName.c_str(), userName.size());
        return BIOSEC_NO_ERROR;

    } catch(const std::exception &ex) {
        logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
        return RetCode::BIOSEC_INTERNAL_ERROR;
    } catch(...) {
        logger->log(blm_utils::SeverityLevel::CRITICAL);
        return RetCode::BIOSEC_UNKNOWN_ERROR;
    }
}


BIOSECDLL_API uint8_t __stdcall startEncryption(updateCallback fpUpdate, finishCallback fpFinish, errorCallback fpError) {
    try {

        std::mutex mtx;
        std::condition_variable cv;
        bool archieverLocked(false);
        bool archieverInited(true);
        bool lockResultAvailable(false);

        auto task = [&](updateCallback fpUpdate, finishCallback fpFinish, errorCallback fpError) {
            try {
                std::unique_lock<std::mutex> cvLock(mtx);
                std::unique_lock<std::mutex> lk(archieverMtx, std::try_to_lock);
                if(!lk.owns_lock()) {
                    archieverLocked = false;
                    lockResultAvailable = true;
                    cv.notify_one();
                    return;
                }

                archieverLocked = true;
                if(!archiever) {
                    archieverInited = false;
                    lockResultAvailable = true;
                    cv.notify_one();
                    return;
                }

                lockResultAvailable = true;
                archieverInited = true;
                cvLock.unlock();
                cv.notify_one();

                archiever->setProgressUpdate(fpUpdate);
                archiever->setOperationErrorCbc(fpError);
                archiever->startEncryption();
                auto targetDir = archiever->getBiosecureFilePath();
                lk.unlock();
                // TODO (an.skornyakov@gmail.com): parse result of encryption and pass result code to fpFinish

                fpFinish(0, targetDir.data());


            } catch(const std::exception &ex) {
                logger->log(blm_utils::SeverityLevel::ERR, "internal error");
                fpFinish(RetCode::BIOSEC_INTERNAL_ERROR, L"");
            } catch(...) {
                logger->log(blm_utils::SeverityLevel::ERR, "unknown error");
                fpFinish(RetCode::BIOSEC_UNKNOWN_ERROR, L"");
            }
        };

        std::unique_lock<std::mutex> lk(mtx);
        std::thread encryptionTask(task, fpUpdate, fpFinish, fpError);
        encryptionTask.detach();

        cv.wait(lk, [&]() {
            return lockResultAvailable;
        });

        if(!archieverLocked) {
            return RetCode::BIOSEC_BUSY;
        }

        if(!archieverInited) {
            logger->log(blm_utils::SeverityLevel::ERR, "library not inited");
            return RetCode::BIOSEC_LIB_NOT_INITED;
        }

    } catch(const std::exception &ex) {
        logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
        return RetCode::BIOSEC_INTERNAL_ERROR;
    } catch(...) {
        logger->log(blm_utils::SeverityLevel::CRITICAL);
        return RetCode::BIOSEC_UNKNOWN_ERROR;
    }

    return BIOSEC_NO_ERROR;

}


BIOSECDLL_API uint8_t __stdcall startDecryption(updateCallback fpUpdate, finishCallback fpFinish, errorCallback fpError) {
    try {

        std::mutex mtx;
        std::condition_variable cv;
        bool archieverLocked(false);
        bool archieverInited(true);
        bool lockResultAvailable(false);

        auto task = [ &, fpUpdate, fpFinish, fpError]() {
            try {
                std::unique_lock<std::mutex> cvLock(mtx);
                std::unique_lock<std::mutex> lk(archieverMtx, std::try_to_lock);
                if(!lk.owns_lock()) {
                    archieverLocked = false;
                    lockResultAvailable = true;
                    cv.notify_one();
                    return;
                }

                archieverLocked = true;
                if(!archiever) {
                    archieverInited = false;
                    lockResultAvailable = true;
                    cv.notify_one();
                    return;
                }

                lockResultAvailable = true;
                archieverInited = true;
                cvLock.unlock();
                cv.notify_one();

                archiever->setProgressUpdate(fpUpdate);
                archiever->setOperationErrorCbc(fpError);
                archiever->startDecryption();
                auto sourceArchive = archiever->getBiosecureFilePath();
                lk.unlock();
                // TODO (an.skornyakov@gmail.com): parse result of encryption and pass result code to fpFinish

                fpFinish(0, sourceArchive.data());


            } catch(const std::exception &ex) {
                logger->log(blm_utils::SeverityLevel::ERR, "internal error");
                fpFinish(RetCode::BIOSEC_INTERNAL_ERROR, L"");
            } catch(...) {
                logger->log(blm_utils::SeverityLevel::ERR, "unknown error");
                fpFinish(RetCode::BIOSEC_UNKNOWN_ERROR, L"");
            }
        };

        std::unique_lock<std::mutex> lk(mtx);
        std::thread decryptionTask(task);
        decryptionTask.detach();


        cv.wait(lk, [&]() {
            return lockResultAvailable;
        });

        if(!archieverLocked) {
            return RetCode::BIOSEC_BUSY;
        }

        if(!archieverInited) {
            logger->log(blm_utils::SeverityLevel::ERR, "library not inited");
            return RetCode::BIOSEC_LIB_NOT_INITED;
        }

    } catch(const std::exception &ex) {
        logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
        return RetCode::BIOSEC_INTERNAL_ERROR;
    } catch(...) {
        logger->log(blm_utils::SeverityLevel::CRITICAL);
        return RetCode::BIOSEC_UNKNOWN_ERROR;
    }

    return BIOSEC_NO_ERROR;
}


BIOSECDLL_API uint8_t __stdcall setSelectionPaths(const uint64_t* indices, uint64_t size) {
    try {
        std::unique_lock<std::mutex> lock(archieverMtx, std::try_to_lock);

        if(!lock.owns_lock()) {
            return BIOSEC_BUSY;
        }
        if(!archiever) {
            logger->log(blm_utils::SeverityLevel::ERR, "library not inited");
            return BIOSEC_LIB_NOT_INITED;
        }

        std::vector<uint64_t> indexVector(indices, indices + size);
        archiever->setItemsToDecompress(indexVector);

        return BIOSEC_NO_ERROR;

    } catch(const std::exception &ex) {
        logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
        return RetCode::BIOSEC_INTERNAL_ERROR;
    } catch(...) {
        logger->log(blm_utils::SeverityLevel::CRITICAL);
        return RetCode::BIOSEC_UNKNOWN_ERROR;
    }


    return 0;
}


BIOSECDLL_API uint8_t __stdcall getArchivePath(uint32_t index, archivePathInfo* pathInfo, uint64_t pathnameBufferSize) {
    try {
        //blm_utils::ScopedProfiler scopeProfiler(__FUNCTION__, *logger);
        //blm_utils::Profiler profiler(*logger);

        //profiler.start("lock");
        std::unique_lock<std::mutex> lock(archieverMtx, std::try_to_lock);
        //profiler.stop();

        //profiler.start("check_lock");
        if(!lock.owns_lock()) {
            return BIOSEC_BUSY;
        }
        //profiler.stop();

        //profiler.start("check_archive");
        if(!archiever) {
            logger->log(blm_utils::SeverityLevel::ERR, "library not inited");
            return BIOSEC_LIB_NOT_INITED;
        }
        //profiler.stop();



        blm_utils::ArchiveEntryInfo archiveEntryInfo;
        //profiler.start("getArchiveEntryInfoAt");
        auto res = archiever->getArchiveEntryInfoAt(&archiveEntryInfo, index);
        //profiler.stop();
        switch(res) {
            case blm_utils::IArchiever::ArchiveEntryQueryRes::OUT_OF_RANGE:
                return BIOSEC_INDEX_OUT_OF_RANGE;
                break;
            case blm_utils::IArchiever::ArchiveEntryQueryRes::SUCCESS:
                //profiler.start("populate struct");
                wcsncpy_s(pathInfo->pathName, pathnameBufferSize, archiveEntryInfo.fileName.c_str(), archiveEntryInfo.fileName.size());
                pathInfo->index = index;
                pathInfo->mTime = archiveEntryInfo.mtime;
                pathInfo->size = archiveEntryInfo.fileSize;
                //profiler.stop();
                switch(archiveEntryInfo.type) {
                    case blm_utils::EntityType::INVALID:
                        return BIOSEC_INTERNAL_ERROR;
                        break;
                    case blm_utils::EntityType::FOLDER:
                        pathInfo->type = BIOSEC_FOLDER;
                        break;
                    case blm_utils::EntityType::REGULAR:
                        pathInfo->type = BIOSEC_REGULAR;
                        break;
                    default:
                        logger->log(blm_utils::SeverityLevel::ERR, "internal error");
                        return BIOSEC_INTERNAL_ERROR;
                        break;
                }
                return BIOSEC_NO_ERROR;
                break;
            case blm_utils::IArchiever::ArchiveEntryQueryRes::INVALID_RES:
                logger->log(blm_utils::SeverityLevel::ERR, "internal error");
                return BIOSEC_INTERNAL_ERROR;
                break;
            default:
                logger->log(blm_utils::SeverityLevel::ERR, "unknown return code");
                return BIOSEC_UNKNOWN_ERROR;
                break;
        }

    } catch(const std::exception &ex) {
        logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
        return RetCode::BIOSEC_INTERNAL_ERROR;
    } catch(...) {
        logger->log(blm_utils::SeverityLevel::CRITICAL);
        return RetCode::BIOSEC_UNKNOWN_ERROR;
    }
}


uint8_t __stdcall checkIfActiveUserEnrolled() {
    auto res = archiever->isUserEnrolled();
    switch(res) {
        case blm_utils::IArchiever::Result::SUCCESS:
            return RetCode::BIOSEC_NO_ERROR;
            break;
        case blm_utils::IArchiever::Result::USER_NOT_ENROLLED:
            logger->log(blm_utils::SeverityLevel::ERR, "User not enrolled");
            return RetCode::BIOSEC_USER_NOT_ENROLLED;
            break;
        default:
            logger->log(blm_utils::SeverityLevel::ERR);
            return RetCode::BIOSEC_UNKNOWN_ERROR;
            break;
    }
}


BIOSECDLL_API uint8_t __stdcall getEntriesInArchiveCount(uint64_t* count) {
	try {
		if(nullptr == count){
			return BIOSEC_INVALID_ARG;
		}

		std::unique_lock<std::mutex> lock(archieverMtx, std::try_to_lock);

		if(!lock.owns_lock()) {
			return BIOSEC_BUSY;
		}
		if(!archiever) {
			logger->log(blm_utils::SeverityLevel::ERR, "library not inited");
			return BIOSEC_LIB_NOT_INITED;
		}

		*count = archiever->getArchiveEnriesCount();
		return BIOSEC_NO_ERROR;

	} catch(const std::exception &ex) {
		logger->log(blm_utils::SeverityLevel::CRITICAL, ex.what());
		return RetCode::BIOSEC_INTERNAL_ERROR;
	} catch(...) {
		logger->log(blm_utils::SeverityLevel::CRITICAL);
		return RetCode::BIOSEC_UNKNOWN_ERROR;
	}


	return 0;
}


