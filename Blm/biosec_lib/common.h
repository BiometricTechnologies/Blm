#ifndef __common_h__
#define __common_h__

#include <string>
#include <cstdint>
#include <chrono>
#include "boost/filesystem.hpp"
#include "severityLogger.h"
#include "biosec_exceptions.h"

namespace blm_utils{

bool hasExtensionEquals(const boost::filesystem::path& path, const std::wstring& extension);
std::string getRandomString(size_t length);

void removePathIfNotDirectory(const boost::filesystem::path &path);
std::wstring getThreadUserName();
int copy_data(struct archive* ar, struct archive* aw);
void winAppendPathNode(std::wstring &path, const std::wstring &node);
bool DeleteFileNow(const wchar_t* filename);
void removeLastPathFileExtension(std::wstring &path);
int pathFilesCount(const std::wstring &path);
bool removeFilesRecursive(const boost::filesystem::path &path);
bool movePath(const std::wstring &oldpath, const std::wstring &newPath);
bool movePathWithSuffix(const std::wstring &path, const wchar_t* suffix);
std::int32_t pathsSumFilesCount(const std::vector<std::shared_ptr<boost::filesystem::path>> &srcPaths);
bool isWriteAble(const std::wstring &path);
std::wstring getPathExtensions(const boost::filesystem::path &path);
boost::filesystem::path getPathStem(const boost::filesystem::path &path);
std::wstring removeLastSignIfEqual(std::wstring string, wchar_t ch);
void removeRootPath(boost::filesystem::path* path);
std::wstring GetCurrentUserSid();


class Profiler {
public:
	Profiler(const SeverityLogger& logger)
		:logger_(logger) {

	}


	void start(const std::string& intervalname){
		intervalname_ = intervalname;
		profilerStartTimePoint_ = std::chrono::system_clock::now();
	}


	void stop(){
		try{
			std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - profilerStartTimePoint_);
			//logger_.log(SeverityLevel::NOTIFICATION,  std::string() + intervalname_ + " took " + std::to_string(ms.count()) + " ms");
		} catch (const std::exception& ex){
			logger_.log(SeverityLevel::ERR,  std::string() + "scoped profiler destructor raise an exception");
		}
	}


private:
	std::chrono::time_point<std::chrono::system_clock> profilerStartTimePoint_;
	std::string intervalname_;
	SeverityLogger logger_;

};


class ScopedProfiler {
public:
	ScopedProfiler(const std::string& scopeName, const SeverityLogger& logger)
		:profilerStartTimePoint_(std::chrono::system_clock::now()),
		logger_(logger),
		scopeName_(scopeName){

	}


	~ScopedProfiler(){
		try{
			std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - profilerStartTimePoint_);
			//logger_.log(SeverityLevel::NOTIFICATION,  std::string() + scopeName_ + " took " + std::to_string(ms.count()) + " ms");
		} catch (const std::exception& ex){
			logger_.log(SeverityLevel::ERR,  std::string() + "scoped profiler destructor raise an exception");
		}
	}


private:
	std::chrono::time_point<std::chrono::system_clock> profilerStartTimePoint_;
	std::string scopeName_;
	SeverityLogger logger_;

};

}

#endif // __common_h__
