#include "stdafx.h"
#include <vector>
#include <olectl.h>
#include "IdentaZoneAP.h"
#include "logging.h"
#include "Utils.h"
#include "EncryptionKeyManager.h"

#define MAX_VALUE_BUFFER_SIZE    0x10000
#define INITIAL_VALUE_BUFFER_SIZE 0x1000

CRITICAL_SECTION DllCriticalSection;    // Serializes access to all globals in module

__declspec(dllexport) void* dllGetEncryptionKeyQueryObject();

STDAPI_(BOOL) DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call) {

	case DLL_PROCESS_ATTACH:
        //InitializeCriticalSection( &DllCriticalSection );
		//open_log();
        break;
    case DLL_PROCESS_DETACH:
        //EnterCriticalSection( &DllCriticalSection );
        //LeaveCriticalSection( &DllCriticalSection );
        //DeleteCriticalSection( &DllCriticalSection );
		//close_log();
        break;

	default:
		break;
    }

    return TRUE;
    UNREFERENCED_PARAMETER( hModule );
    UNREFERENCED_PARAMETER( lpReserved );

}

__declspec(dllexport) void* dllGetEncryptionKeyQueryObject(){
	static BiometricEncryptionKeyManager encKeymanager;
	return &encKeymanager;
}

#pragma region registration

// Structure to hold data for individual keys to register.
typedef struct
{
    HKEY  hRootKey;
    PCWSTR pszSubKey;
    PCWSTR pszClassID;
    PCWSTR pszValueName;
    BYTE *pszData;
    DWORD dwType;
} REGSTRUCT;

const WCHAR apLibraryName[] = L"IdentaZoneAP";


/*
STDAPI DllRegisterServer()
{	
	
	HRESULT hr = S_OK;
	//char buffer[0x100];
	DWORD offset = 0x100;

	Sleep(20000);

	long 
lResult = RegGetValue(
		HKEY_LOCAL_MACHINE,
		L"System\\CurrentControlSet\\Control\\Lsa\\",
		L"Security Packages",
		RRF_RT_ANY,
		NULL,
		buffer,
		&offset);

	if (ERROR_SUCCESS == lResult)
	{
		wcscpy((WCHAR*)(buffer+offset-2), apLibraryName);
		auto libNameLen = wcslen(apLibraryName)*sizeof(WCHAR);
		WCHAR footer[] = {L'\0', L'\0'};

		wcsncpy((WCHAR*)(buffer+offset+libNameLen-2), footer, 2);
		DWORD valueTotalSize = offset + libNameLen + 2;

		lResult = RegSetValueEx(
			HKEY_LOCAL_MACHINE,
			L"System\\CurrentControlSet\\Control\\Lsa\\Security Packages",
			0,
			REG_MULTI_SZ,
			(BYTE*)buffer,
			valueTotalSize);

		if (ERROR_SUCCESS == lResult)
		{
			
		} else {
			hr = SELFREG_E_CLASS;
		}

	} else {
		hr = SELFREG_E_CLASS;
	}


    if (FAILED(hr))
    {
        // Remove the stuff we added.
        DllUnregisterServer();
    }

    return hr;
}*/


STDAPI DllRegisterServer()
{	

	HRESULT hr = S_OK;
	std::vector<char> buffer;
	buffer.resize(INITIAL_VALUE_BUFFER_SIZE);
	DWORD offset = buffer.size();

	Sleep(20000);

	long lResult = RegGetValue(
			HKEY_LOCAL_MACHINE,
			L"System\\CurrentControlSet\\Control\\Lsa\\",
			L"Security Packages",
			RRF_RT_ANY,
			NULL,
			buffer.data(),
			&offset);
		
	if(ERROR_MORE_DATA==lResult){
		buffer.resize(MAX_VALUE_BUFFER_SIZE);
		offset = buffer.size();
		lResult = RegGetValue(
				HKEY_LOCAL_MACHINE,
				L"System\\CurrentControlSet\\Control\\Lsa\\",
				L"Security Packages",
				RRF_RT_ANY,
				NULL,
				buffer.data(),
				&offset);
		if(ERROR_MORE_DATA==lResult){
				return SELFREG_E_CLASS;
			}
	}

	if (ERROR_SUCCESS == lResult)
	{	
		auto libnNameOffset = findInMultiString((wchar_t*)buffer.data(), offset, apLibraryName)*sizeof(wchar_t);
		if(offset!=libnNameOffset){
			return S_OK; // out lib is there already
		}

		wcscpy((WCHAR*)(buffer.data()+offset-2), apLibraryName);
		auto libNameLen = wcslen(apLibraryName)*sizeof(WCHAR);
		WCHAR footer[] = {L'\0', L'\0'};
		wcsncpy((WCHAR*)(buffer.data()+offset+libNameLen-2), footer, 2);
		DWORD valueTotalSize = offset + libNameLen + 2;

		HKEY regKey;
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\Lsa", 0, KEY_ALL_ACCESS, &regKey);
		if (lResult == ERROR_SUCCESS)
		{
			lResult = RegSetValueEx(
				regKey,
				L"Security Packages",
				0,
				REG_MULTI_SZ,
				(BYTE*)buffer.data(),
				valueTotalSize);

			if (ERROR_SUCCESS == lResult)
			{

			} else {
				hr = SELFREG_E_CLASS;
			}
		} else {
			hr = SELFREG_E_CLASS;
		}

		RegCloseKey(regKey);
	} else {
		hr = SELFREG_E_CLASS;
	}


	if (FAILED(hr))
	{
		// Remove the stuff we added.
		DllUnregisterServer();
	}

	return hr;
}


// Registry keys are removed here.
STDAPI DllUnregisterServer()
{
	Sleep(20000);
	HRESULT hr = S_OK;
	std::vector<char> buffer;
	buffer.resize(INITIAL_VALUE_BUFFER_SIZE);
	DWORD offset = buffer.size();

	long lResult = RegGetValue(
		HKEY_LOCAL_MACHINE,
		L"System\\CurrentControlSet\\Control\\Lsa\\",
		L"Security Packages",
		RRF_RT_ANY,
		NULL,
		buffer.data(),
		&offset);

	if(ERROR_MORE_DATA==lResult){
		buffer.resize(MAX_VALUE_BUFFER_SIZE);
		offset = buffer.size();
		lResult = RegGetValue(
			HKEY_LOCAL_MACHINE,
			L"System\\CurrentControlSet\\Control\\Lsa\\",
			L"Security Packages",
			RRF_RT_ANY,
			NULL,
			buffer.data(),
			&offset);
		if(ERROR_MORE_DATA==lResult){
			return SELFREG_E_CLASS;
		}
	}

	if (ERROR_SUCCESS == lResult)
	{		
		auto libnNameOffset = findInMultiString((wchar_t*)buffer.data(), offset, apLibraryName)*sizeof(wchar_t);;
		if(offset==libnNameOffset){
			return S_OK; // there isn't our library
		} else {
			// we need to cut it
			auto libNameLenWithTerm = wcslen(apLibraryName)*sizeof(wchar_t) + 2;
			DWORD valueTotalSize = offset - libNameLenWithTerm;
			wcsncpy((wchar_t*)(buffer.data()+libnNameOffset),
				    (wchar_t*)(buffer.data()+libnNameOffset+libNameLenWithTerm),
					 offset-libNameLenWithTerm-libnNameOffset);

			HKEY regKey;
			lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\Lsa", 0, KEY_ALL_ACCESS, &regKey);
			if (lResult == ERROR_SUCCESS)
			{
				lResult = RegSetValueEx(
					regKey,
					L"Security Packages",
					0,
					REG_MULTI_SZ,
					(BYTE*)buffer.data(),
					valueTotalSize);

				if (ERROR_SUCCESS == lResult)
				{

				} else {
					hr = SELFREG_E_CLASS;
				}
			} else {
				hr = SELFREG_E_CLASS;
			}

			RegCloseKey(regKey);
		}

		
	} else {
		hr = SELFREG_E_CLASS;
	}

	return hr;
}

#pragma endregion registration