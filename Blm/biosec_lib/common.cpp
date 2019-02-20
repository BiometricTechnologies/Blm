#include "Shlwapi.h"
#include "lmcons.h"
#include <sddl.h>

#include "boost/algorithm/string/trim.hpp"

#include "common.h"

namespace blm_utils{


bool hasExtensionEquals(const boost::filesystem::path &path, const std::wstring &extension) {
    return (0 == path.extension().compare(extension));
}

std::string getRandomString(size_t length) {
    auto randchar = []() -> char {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

/// <summary>
/// Removes the last path file extension.
/// </summary>
/// <param name="path">The path.</param>
void removeLastPathFileExtension(std::wstring &path) {
	auto dotPos = path.rfind(L".");
	if(dotPos != std::wstring::npos) {
		path.erase(dotPos);
	}
}


/// <summary>
/// Appends path node to path.
/// </summary>
/// <param name="path">The path.</param>
/// <param name="node">The node.</param>
void winAppendPathNode(std::wstring &path, const std::wstring &node) {
	path.append(L"\\");
	path.append(node);
}


/// <summary>
/// Deletes the file.
/// </summary>
/// <param name="filename">The filename.</param>
/// <returns></returns>
bool DeleteFileNow(const wchar_t* filename) {
	// don't do anything if the file doesn't exist!
	if(!PathFileExistsW(filename)) {
		return true;
	}

	auto name = removeLastSignIfEqual(filename, L'\\');
	// determine the path in which to store the temp filename
	boost::filesystem::path pathToStoreTemp(name);
	pathToStoreTemp = pathToStoreTemp.parent_path();

	// generate a guaranteed to be unique temporary filename to house the pending delete
	wchar_t tempname[MAX_PATH];
	if(!GetTempFileNameW(pathToStoreTemp.c_str(), L".xX", 0, tempname)) {
		return false;
	}

	// move the real file to the dummy filename
	if(!MoveFileExW(name.c_str(), tempname, MOVEFILE_REPLACE_EXISTING)) {
		//LOG(ERROR) << "DeleteFileNow[error] moving file " << name.c_str() << " , error: " << GetLastError();
		// clean up the temp file
		if(!DeleteFileW(tempname)) {
			//LOG(ERROR) << "DeleteFileNow[error] deleting file error: " << GetLastError();
		}
		return false;
	}

	// queue the deletion (the OS will delete it when all handles (ours or other processes) close)
	if(!DeleteFileW(tempname)) {
		//LOG(ERROR) << "DeleteFileNow[error] deleting file  error: " << GetLastError();
		return false;
	}
	return true;
}


/// <summary>
/// Moves the path with suffix.
/// </summary>
/// <param name="path">The path.</param>
/// <param name="suffix">The suffix.</param>
/// <returns></returns>
bool movePathWithSuffix(const std::wstring &path, const wchar_t* suffix) {
	//DLOG(INFO) << "Moving original file";

	boost::filesystem::path src(path);
	auto dest = (src.parent_path().wstring() +  L"\\" + src.stem().wstring());

	if(src.has_extension()) {
		dest += suffix;
		dest += (src.extension().wstring());;
	} else {
		// pop back "\\. from stem"
		dest.pop_back();
		dest.pop_back();
		dest += suffix;
	}

	if(!movePath(src.wstring(), dest)) {
		//LOG(ERROR) << "[movePathWithSuffix] error: Can't move file: " << path.c_str() << " . Error code: " << GetLastError() << ".";
		return false;
	}

	return true;
}

/// <summary>
/// Moves the path.
/// </summary>
/// <param name="oldpath">The old path.</param>
/// <param name="newPath">The new path.</param>
/// <returns></returns>
bool movePath(const std::wstring &oldpath, const std::wstring &newPath) {

	// don't do anything if the file doesn't exist
	if(!PathFileExistsW(oldpath.c_str())) {
		return false;
	}

	// move the real file to the dummy filename
	if(!MoveFileExW(oldpath.c_str(), newPath.c_str(), MOVEFILE_REPLACE_EXISTING)) {
		//LOG(ERROR) << "DeleteFileNow[error] deleting file, error: " << GetLastError();
		// clean up the temp file
		DeleteFileW(newPath.c_str());
		return false;
	}

	return true;
}


/// <summary>
/// Recursively compute path file count
/// </summary>
/// <param name="path">The path.</param>
/// <returns></returns>
int pathFilesCount(const std::wstring &path) {
	int fileCount = 0;

	boost::filesystem::path p(path);
	if(boost::filesystem::is_regular_file(p)) {
		return 1;
	} else if(boost::filesystem::is_directory(p)) {
		boost::filesystem::directory_iterator dir_first(p), dir_last;

		auto pred = [&](const boost::filesystem::directory_entry & entry) {
			fileCount += pathFilesCount(entry.path().wstring());
		};

		std::for_each(dir_first, dir_last, pred);
		return fileCount;
	} else {
		throw std::exception("[pathFilesCount]: error unknown file type");
	}
}

/// <summary>
/// Removes the files recursive.
/// </summary>
/// <param name="path">The path.</param>
/// <returns></returns>
bool removeFilesRecursive(const boost::filesystem::path &path) {
	bool res = true;

	if(!boost::filesystem::exists(path)) {
		return false;
	}

	if(boost::filesystem::is_regular_file(path)) {
		return DeleteFileNow(path.c_str());
	} else if(boost::filesystem::is_directory(path)) {
		boost::filesystem::directory_iterator dir_first(path), dir_last;

		auto pred = [&](const boost::filesystem::directory_entry & entry) {
			res &= removeFilesRecursive(entry.path().wstring());
		};

		std::for_each(dir_first, dir_last, pred);
		res &= RemoveDirectory(path.c_str());
		return res;
	} else {
		throw std::exception("[removeFilesRecursive]: error unknown file type");
	}
}


/// <summary>
/// Get total files count under array of paths.
/// </summary>
/// <param name="srcPaths">The source paths.</param>
/// <returns></returns>
std::int32_t pathsSumFilesCount( const std::vector<std::shared_ptr<boost::filesystem::path>> &srcPaths )
{
	std::int32_t totalFiles = 0;
	auto func = [&](const std::shared_ptr<boost::filesystem::path> & pPath) {
		totalFiles += pathFilesCount((*pPath).wstring());
	};
	std::for_each(std::begin(srcPaths), std::end(srcPaths), func);
	return totalFiles;
}

/// <summary>
/// Determines whether the specified path is writable.
/// </summary>
/// <param name="path">The path.</param>
/// <returns></returns>
bool isWriteAble(const std::wstring &path) {

	auto attr = GetFileAttributes(path.c_str());
	if(INVALID_FILE_ATTRIBUTES == attr) {
		throw std::system_error(GetLastError(), std::system_category(), "[isWriteAble] error reading path attributes");
	}
	wchar_t testFile[MAX_PATH];
	auto testDir = GetTempFileName(path.c_str(), L"IZ", 0, testFile);
	if(testDir) {
		DeleteFile(testFile);
		return true;
	}
	return false;
}


std::wstring getThreadUserName() {
	std::vector<wchar_t> userName;
	userName.resize(UNLEN + 1);
	DWORD size = userName.size();

	if(!GetUserName(userName.data(), &size)) {
		throw std::exception("[getThreadUserName] error cant get user name");
	}

	userName.resize(size);
	std::wstring userNameString(std::begin(userName), std::end(userName));
	return userNameString;
}

void removePathIfNotDirectory(const boost::filesystem::path &path) {
	if(!boost::filesystem::is_directory(path)) {
		if(!DeleteFileNow(path.c_str())) {
			throw std::exception("[removePathIfNotDirectory] error: Can't delete original file");
		}
	}
}


void removeRootPath(boost::filesystem::path* path) {
	// remove root from path with trailing slash
	boost::filesystem::path rootPath = *path;
	while(rootPath.has_parent_path()) {
		rootPath = rootPath.parent_path();
	}
	auto rootLen = rootPath.wstring().size();
	auto pathStrWithoutRoot = (path->wstring());
	pathStrWithoutRoot.erase(0, rootLen + 1);
	*path = pathStrWithoutRoot;

}


std::wstring removeLastSignIfEqual(std::wstring string, wchar_t ch) {
	boost::algorithm::trim_right_if(string, [&](wchar_t thisChar) {
		return (thisChar == ch);
	});
	return string;
}


std::wstring getPathExtensions(const boost::filesystem::path &path) {
	std::stack<std::wstring> extensionsStack;
	auto filename = path.filename();
	while(filename.has_extension()) {
		extensionsStack.push(filename.extension().wstring());
		filename = filename.stem();
	}

	std::wstring estensionsFused;
	while(!extensionsStack.empty()) {
		estensionsFused.append(extensionsStack.top());
		extensionsStack.pop();
	}

	return estensionsFused;
}

boost::filesystem::path getPathStem(const boost::filesystem::path &path) {
	auto stem = path.stem();
	boost::filesystem::path allExtensions;
	while(stem.has_extension()) {
		stem = stem.stem();
	}
	return stem;
}


std::wstring GetCurrentUserSid() {
	HANDLE hTok = NULL;
	LPWSTR sidStr = NULL;
	if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTok)) {
		// get user info size
		LPBYTE buf = NULL;
		DWORD  dwSize = 0;
		GetTokenInformation(hTok, TokenUser, NULL, 0, &dwSize);
		if(dwSize) {
			buf = (LPBYTE)LocalAlloc(LPTR, static_cast<SIZE_T>(dwSize));
		}

		// get user info
		if(GetTokenInformation(hTok, TokenUser, buf, dwSize, &dwSize)) {
			PSID pSid = ((PTOKEN_USER)buf)->User.Sid;
			ConvertSidToStringSid(pSid, &sidStr);
		}
		LocalFree(buf);
		CloseHandle(hTok);
	}
	if(sidStr != NULL) {
		return std::wstring(sidStr);
	} else {
		return nullptr;
	}
}



}