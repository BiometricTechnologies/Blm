/**************************************************************************
    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   (c) Microsoft Corporation. All Rights Reserved.
**************************************************************************/

#include <windows.h>
#include <shlobj.h>
#include "resource.h"
#include "Utils.h"
#include "shlwapi.h"

#include <wchar.h>
#include <io.h>
#include <fcntl.h>

#include <algorithm>
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "common.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
// Log
#pragma warning(push)
#pragma warning(disable : 4251)
#include <glog/logging.h>
#pragma warning(pop)

static const wchar_t* loggingPath = L"C:\\Logs\\IdentaZone\\Biosec";
static const char* biosecApplicationName = "BioSecure";
static const wchar_t* biosecApplicationNameW = L"BioSecure";
static const wchar_t* biosecFileExt = L".izbiosecure";

static const wchar_t* biosecureRegistryKey = L"SOFTWARE\\Classes\\biosecure.izbiosecure";
static const wchar_t* biosecureApplicatioPathRegValName = L"ApplicationPath";
static const wchar_t* biosecureApplicatioPathRegValNameDefault = L"bad";
static const wchar_t* biosecureSecure = L"bad";
static const wchar_t* biosecureCmdSecureKey = L" -s";
static const wchar_t* biosecureCmdUnsecureKey = L" -u";
static const wchar_t* biosecureCmdInputKey = L" -i";
static const wchar_t* biosecureCmdFileInputKey = L" -f";

static const uint32_t maxCmdLength = 32767;


static std::wstring produceResultNameSingleFile(const std::vector<std::wstring> &paths);
static std::wstring produceResultNameGroupFiles(const std::vector<std::wstring> &paths);
static std::vector<std::wstring>* hasBiosecuredFiles(const std::vector<std::wstring> &paths);
static bool CreateDirectoryRecursively(const wchar_t* fullPath);
DWORD WINAPI secureThread(LPVOID lpParam);
DWORD WINAPI unsecureThread(LPVOID lpParam);
bool biosecure(const std::vector<std::wstring> &paths);

class GoogleLogWrapper {
public:
    GoogleLogWrapper(const char* appName, const char* logPath);
    ~GoogleLogWrapper();
private:
    static bool isInited_;
    GoogleLogWrapper(const GoogleLogWrapper &);
    GoogleLogWrapper &operator=(const GoogleLogWrapper &);
};
bool GoogleLogWrapper::isInited_ = false;


GoogleLogWrapper::GoogleLogWrapper(const char* appName, const char* logPath) {
    if(!isInited_) {
        // TODO: thread safety (an.skornyakov@gmail.com)
        // thread safety guaranteed by C++11 standart for initialization and destruction
        isInited_ = true;
        //FLAGS_log_dir = logPath;
        //google::InitGoogleLogging(appName);
    }
}

GoogleLogWrapper::~GoogleLogWrapper() {
    if(isInited_) {
        google::FlushLogFiles(ERROR);
        //google::ShutdownGoogleLogging();
    }
}


// Do not modify order here
static const UINT g_rgIDs[] = {
    IDS_ZERO,
    IDS_ONE,
    IDS_TWO,
    IDS_THREE,
    IDS_FOUR,
    IDS_FIVE,
    IDS_SIX,
    IDS_SEVEN,
    IDS_EIGHT,
    IDS_NINE
};


enum class BiosecureOperation {
    SECURE,
    UNSECURE
};


HRESULT LoadFolderViewImplDisplayString(UINT uIndex, PWSTR psz, UINT cch) {
    HRESULT hr = E_FAIL;

    UINT uString = 0;
    if(uIndex < ARRAYSIZE(g_rgIDs)) {
        uString = g_rgIDs[uIndex];
    }

    if(uString) {
        if(LoadString(g_hInst, uString, psz, cch)) {
            hr = S_OK;
        }
    }
    return hr;
}

HRESULT GetIndexFromDisplayString(PCWSTR psz, UINT* puIndex) {
    HRESULT hr = E_FAIL;

    *puIndex = 0;
    WCHAR szBuff[100] = {};
    for(UINT u = 0; u < ARRAYSIZE(g_rgIDs); u++) {
        if(LoadString(g_hInst, g_rgIDs[u], szBuff, ARRAYSIZE(szBuff))) {
            if(lstrcmpi(szBuff, psz) == 0) {
                *puIndex = u;
                hr = S_OK;
                break;
            }
        }
    }
    return hr;
}

HRESULT LoadFolderViewImplDisplayStrings(PWSTR wszArrStrings[], UINT cArray) {
    HRESULT hr = S_OK;
    for(UINT i = 0; SUCCEEDED(hr) && i < cArray; i++) {
        WCHAR wszTemp[MAX_PATH];
        hr = LoadFolderViewImplDisplayString(i, wszTemp, ARRAYSIZE(wszTemp));
        if(SUCCEEDED(hr)) {
            hr = SHStrDup(wszTemp, &wszArrStrings[i]);
            if(FAILED(hr)) {
                // Free those already allocated.
                for(UINT k = 0; k < i; k++) {
                    CoTaskMemFree(wszArrStrings[k]);
                    wszArrStrings[k] = NULL;
                }
                break;
            }
        } else {
            // Somebody tried increasing the size of the array without
            // adding additional strings.
            wszArrStrings[i] = NULL;
        }
    }
    return hr;
}

STDAPI StringToStrRet(PCWSTR pszName, STRRET* pStrRet) {
    pStrRet->uType = STRRET_WSTR;
    return SHStrDup(pszName, &pStrRet->pOleStr);
}


HRESULT getShellItemsPathsList(IShellItemArray* psia, std::vector<std::wstring> &paths) {
    HRESULT hr;
    DWORD elemCount = 0;
    hr = psia->GetCount(&elemCount);

    if(SUCCEEDED(hr)) {
        for(auto i = 0; i < elemCount; ++i) {
            IShellItem* psi;
            hr = psia->GetItemAt(i, &psi);
            if(SUCCEEDED(hr)) {
                PWSTR pszDisplayName;
                hr = psi->GetDisplayName(SIGDN_NORMALDISPLAY, &pszDisplayName);
                if(SUCCEEDED(hr)) {
                    LPWSTR fileName;
                    hr = (psi->GetDisplayName(SIGDN_FILESYSPATH, &fileName));
                    if(SUCCEEDED(hr)) {
                        paths.emplace_back(std::wstring(fileName));
                    }
                }
                CoTaskMemFree(pszDisplayName);
            }
            psi->Release();
        }
    }
    return hr;
}


HRESULT DisplayItem(IShellItemArray* psia, HWND hwnd) {

    std::unique_ptr<GoogleLogWrapper> glog;
    std::wstring loggingPathWstr(loggingPath);
    std::string loggingPathStr(loggingPathWstr.begin(), loggingPathWstr.end());   // need to init glog
    if(CreateDirectoryRecursively(loggingPath)) {
        glog.reset(new GoogleLogWrapper(biosecApplicationName, loggingPathStr.c_str()));
        LOG(INFO) << "[explorer.exe] start biosecure";
    }

    std::vector<std::wstring> paths;
    auto hr = getShellItemsPathsList(psia, paths);


    if(SUCCEEDED(hr)) {
        biosecure(paths);
    } else {
        LOG(ERROR) << "Fatal error: cant get shell items path list";
    }

    return hr;
}


LONG GetStringRegKey(HKEY hKey, const std::wstring &strValueName, std::wstring &strValue, const std::wstring &strDefaultValue) {
    strValue = strDefaultValue;
    WCHAR szBuffer[512];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError;
    nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
    if(ERROR_SUCCESS == nError) {
        strValue = szBuffer;
    }
    return nError;
}


std::wstring createInputsFile(const std::vector<std::wstring>& inputPaths) {

	DWORD  maxTempPathSize = MAX_PATH + 1; // specified in MSDN
	std::wstring tempPathBuffer(maxTempPathSize, L'\0');

    auto ret = GetTempPathW(maxTempPathSize, &tempPathBuffer[0]); 
    if(ret > MAX_PATH || (ret == 0)) {
        throw std::logic_error("GetTempPath failed");
    }
	
	std::wstring tempFilenameBuffer(MAX_PATH, L'\0');
    if(!GetTempFileNameW(tempPathBuffer.data(), biosecApplicationNameW, 0, &tempFilenameBuffer[0])) {
        throw std::logic_error("Cant create input file");
    }

	LOG(INFO) << "temp file:" << std::string(tempFilenameBuffer.begin(), tempFilenameBuffer.end());
	boost::filesystem::wofstream tempFile(tempFilenameBuffer);

	std::for_each(std::begin(inputPaths), std::end(inputPaths), [&](const std::wstring & thisPath) {
		tempFile << thisPath << std::endl;
	});

	return tempFilenameBuffer;
}


bool biosecurePaths(const std::vector<std::wstring> &paths, BiosecureOperation operation) {

    HKEY hKey;
    auto lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, biosecureRegistryKey, 0, KEY_READ, &hKey);
    if(ERROR_SUCCESS != lRes) {
        MessageBox(NULL, L"fatal error", NULL, MB_ICONERROR);
        return false;
    }

    std::wstring appPath;
    lRes = GetStringRegKey(hKey, biosecureApplicatioPathRegValName, appPath, biosecureApplicatioPathRegValNameDefault);
    if(ERROR_SUCCESS != lRes) {
        MessageBox(NULL, L"fatal error", NULL, MB_ICONERROR);
        return false;
    }

    LPCTSTR lpApplicationName = appPath.data();
    std::wstring lpCommandLine;
    switch(operation) {
        case BiosecureOperation::SECURE:
            lpCommandLine.append(biosecureCmdSecureKey);
            break;
        case BiosecureOperation::UNSECURE:
            lpCommandLine.append(biosecureCmdUnsecureKey);
            break;
        default:
            LOG(ERROR) << "Unknown operation";
            return false;
            break;
    }


	std::wstring inputs;
	std::for_each(std::begin(paths), std::end(paths), [&](const std::wstring & thisPath) {
		inputs +=  L" \"" + thisPath + L"\"";
	});

    auto predictedCmdLen = appPath.size() + (std::max)(wcslen(biosecureCmdSecureKey), wcslen(biosecureCmdUnsecureKey)) +
                           (std::max)(wcslen(biosecureCmdFileInputKey), wcslen(biosecureCmdInputKey)) +
                           inputs.size();
	
    if(predictedCmdLen > maxCmdLength) {
        lpCommandLine.append(biosecureCmdFileInputKey);
        std::wstring inputsFile = createInputsFile(paths);
        lpCommandLine.append(inputsFile);
    } else {
        lpCommandLine.append(biosecureCmdInputKey);
		std::wstring inputs;
		std::for_each(std::begin(paths), std::end(paths), [&](const std::wstring & thisPath) {
			inputs +=  L" \"" + thisPath + L"\"";
		});
        lpCommandLine.append(inputs);
    }

    STARTUPINFO info = {sizeof(info)};
    ZeroMemory(&info, sizeof(STARTUPINFO));
    PROCESS_INFORMATION piForCmd;

    if(CreateProcess(
                appPath.data(),
                const_cast<wchar_t*>(lpCommandLine.data()),
                NULL,
                NULL,
                false,
                0,
                NULL,
                NULL,
                &info,
                &piForCmd
            )) {
        //::WaitForSingleObject(piForCmd.hProcess, INFINITE);
        //CloseHandle(piForCmd.hProcess);
        //CloseHandle(piForCmd.hThread);
    } else {
        MessageBox(NULL, L"Fatal error", NULL, MB_OK);
        LPTSTR errorText = NULL;
        FormatMessage(
            // use system message tables to retrieve error text
            FORMAT_MESSAGE_FROM_SYSTEM
            // allocate buffer on local heap for error text
            | FORMAT_MESSAGE_ALLOCATE_BUFFER
            // Important! will fail otherwise, since we're not
            // (and CANNOT) pass insertion parameters
            | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&errorText,  // output
            0, // minimum size for output buffer
            NULL);   // arguments - see note

        MessageBox(NULL, errorText, NULL, MB_OK);
    }

    return true;
}

bool hasRootPaths(const std::vector<std::wstring> &paths){
	for (auto path = std::begin(paths); path != std::end(paths); ++path){
		boost::filesystem::path pp(*path);
		if (pp.has_root_path() && (pp.root_path() == pp)){
			return true;
		}
	}
	return false;
}

bool biosecure(const std::vector<std::wstring> &paths) {
    try {
        auto size = paths.size();
        if(size < 1) {
            LOG(ERROR) << "[biosecure] error: no paths specified";
            return false;
        }

		if (hasRootPaths(paths)){
			std::wstring message;
			message.append(L"Please select only regular files/folders. It is not possible to encrypt a root folder");
			MessageBox(NULL,
				message.c_str(),
				L"Root folder",
				MB_OK | MB_ICONWARNING);
			LOG(ERROR) << "[biosecure] error: root paths exist";
			return false;
		}
        if(size == 1) { // one file/folder
            boost::filesystem::path p(paths.at(0));
            if(blm_utils::checkFileExtension(p.wstring(), biosecFileExt)) {
                if(!(boost::filesystem::is_directory(p))) {
                    return biosecurePaths(paths, BiosecureOperation::UNSECURE);
                } else { // directory with .izbiosecure suffix
                    std::wstring filename(p.filename().wstring());
                    std::wstring message = L"\"" + filename + L"\" " +
                                           L" is not recognized as valid .izbiosecure archive.";
                    MessageBox(NULL,
                               message.c_str(),
                               NULL,
                               MB_OK | MB_ICONWARNING);
                    return false;
                }
            } else {
                biosecurePaths(paths, BiosecureOperation::SECURE);
            }
        } else { // group of file/folders
            auto biosecured = hasBiosecuredFiles(paths);
            if(biosecured->size() == paths.size()) {
                MessageBox(NULL,
                           L"Please, select only one encrypted archive to enable decryption procedure.",
                           NULL,
                           MB_OK | MB_ICONWARNING);
                delete biosecured;
                return false;
            } else if(!biosecured->empty()) {
                std::wstring message;
                std::wstring filename(boost::filesystem::path(biosecured->front()).filename().c_str());
                filename = L"Encrypted package \"" + filename + L"\"";
                message.append(filename);
                message.append(L" found among files that you selected.\nPlease, select only this file to initialize decryption procedure.\nOtherwise, if you want to encrypt data, please exclude ");
                message.append(filename);
                message.append(L" from your selection.");
                MessageBox(NULL,
                           message.c_str(),
                           L"Ambiguous task",
                           MB_OK | MB_ICONWARNING);
                delete biosecured;
                return false;
            } else {
                biosecurePaths(paths, BiosecureOperation::SECURE);
            }
        }
    } catch(const std::exception &ex) {
        LOG(ERROR) << "[biosecure] Error: " << ex.what();
        return false;
    }
}


/// <summary>
/// Creates the directory recursively.
/// </summary>
/// <param name="fullPath">The full path.</param>
/// <returns></returns>
static bool CreateDirectoryRecursively(const wchar_t* fullPath) {

    auto ret = SHCreateDirectoryEx(NULL, fullPath, NULL);
    if(ERROR_SUCCESS == ret || ERROR_FILE_EXISTS == ret || ERROR_ALREADY_EXISTS == ret) {
        return true;
    }

    return false;
}


bool isBiosecured(const std::wstring &path) {
    return blm_utils::checkFileExtension(path,  L".izbiosecure");
}




std::vector<std::wstring>* hasBiosecuredFiles(const std::vector<std::wstring> &paths) {
    auto found = new std::vector<std::wstring>();
    //return std::find_if(paths.begin(),paths.end(),isBiosecured);

    //int32_t biosecuredPathIndex = 0;
    for(auto path = std::begin(paths); path != std::end(paths); ++path) {
        if(blm_utils::checkFileExtension(*path,  L".izbiosecure")) {
            found->push_back(*path);
        } else if(!found->empty()) {
            break;
            //  }
            //  ++biosecuredPathIndex;
        }
    }
    return found;
    //return biosecuredPathIndex;

}


bool hasNotBiosecuredFiles(const std::vector<std::wstring> &paths) {
    if(std::any_of(std::begin(paths), std::end(paths), [&](const std::wstring & curPath) {
    return (true != blm_utils::checkFileExtension(curPath,  L".izbiosecure"));
    })) {
        return true;
    }
    return false;
}


static std::wstring produceResultNameSingleFile(const std::vector<std::wstring> &paths) {
    boost::filesystem::path p(paths.at(0));
    auto resultPath = p.wstring();
    auto dotPos = resultPath.rfind(L".");
    if(dotPos != std::wstring::npos) {
        resultPath.erase(dotPos);
    }
    resultPath.append(L".izbiosecure");
    return resultPath;
}


static std::wstring produceResultNameGroupFiles(const std::vector<std::wstring> &paths) {
    boost::filesystem::path firstItemPath(paths.at(0));
    auto resultPath = firstItemPath.parent_path().wstring();
    resultPath.append(L"\\");
    resultPath.append(firstItemPath.parent_path().filename().wstring());
    resultPath.append(L".izbiosecure");
    return resultPath;
}




