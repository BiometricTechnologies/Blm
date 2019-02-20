//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#pragma once

#include <windows.h>
#include <strsafe.h>
#include <shlguid.h>
#include <map>
#include "helpers.h"
#include "dll.h"
#include "common.h"

#include "PlainDB.h"
#include "CSVLogger.h"
#include "fp_device.h"
#include "SessionChecker.h"
#include "graphics.h"
#include <boost\timer\timer.hpp>

struct ScannersWindowsManager;
class CMultiloginProvider;
#define MAX_CREDENTIALS 10

typedef struct DP_USER_
{
	PWSTR username;
	PWSTR password;
	std::vector<std::shared_ptr<blm_login::FingerprintTemplate>> fpTemplates;

	std::string formatTypeName;
	CPlainDB::LOGIN_TYPE loginType;
}DP_USER;

class CMultiloginCredential : public ICredentialProviderCredential
{
public:
	// IUnknown
	IFACEMETHODIMP_(ULONG) AddRef()
	{
		return ++_cRef;
	}

	IFACEMETHODIMP_(ULONG) Release()
	{
		LONG cRef = --_cRef;
		if (!cRef)
		{
			delete this;
		}
		return cRef;
	}

	IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void** ppv)
	{
		static const QITAB qit[] =
		{
			QITABENT(CMultiloginCredential, ICredentialProviderCredential), // IID_ICredentialProviderCredential
			{0},
		};
		return QISearch(this, qit, riid, ppv);
	}
public:
	// ICredentialProviderCredential
	IFACEMETHODIMP Advise(__in ICredentialProviderCredentialEvents* pcpce);
	IFACEMETHODIMP UnAdvise();

	IFACEMETHODIMP SetSelected(__out BOOL* pbAutoLogon);
	IFACEMETHODIMP SetDeselected();

	IFACEMETHODIMP GetFieldState(__in DWORD dwFieldID,
		__out CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
		__out CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis);

	IFACEMETHODIMP GetStringValue(__in DWORD dwFieldID, __deref_out PWSTR* ppwsz);
	IFACEMETHODIMP GetBitmapValue(__in DWORD dwFieldID, __out HBITMAP* phbmp);
	IFACEMETHODIMP GetCheckboxValue(__in DWORD dwFieldID, __out BOOL* pbChecked, __deref_out PWSTR* ppwszLabel);
	IFACEMETHODIMP GetComboBoxValueCount(__in DWORD dwFieldID, __out DWORD* pcItems, __out_range(<,*pcItems) DWORD* pdwSelectedItem);
	IFACEMETHODIMP GetComboBoxValueAt(__in DWORD dwFieldID, __in DWORD dwItem, __deref_out PWSTR* ppwszItem);
	IFACEMETHODIMP GetSubmitButtonValue(__in DWORD dwFieldID, __out DWORD* pdwAdjacentTo);

	IFACEMETHODIMP SetStringValue(__in DWORD dwFieldID, __in PCWSTR pwz);
	IFACEMETHODIMP SetCheckboxValue(__in DWORD dwFieldID, __in BOOL bChecked);
	IFACEMETHODIMP SetComboBoxSelectedValue(__in DWORD dwFieldID, __in DWORD dwSelectedItem);
	IFACEMETHODIMP CommandLinkClicked(__in DWORD dwFieldID);

	IFACEMETHODIMP GetSerialization(__out CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, 
		__out CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
		__deref_out_opt PWSTR* ppwszOptionalStatusText, 
		__out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);
	IFACEMETHODIMP ReportResult(__in NTSTATUS ntsStatus, 
		__in NTSTATUS ntsSubstatus,
		__deref_out_opt PWSTR* ppwszOptionalStatusText, 
		__out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);

	void FmtCallback( int serviceIndex, int deviceIndex );
public:

	HRESULT Activate(__in PCWSTR pwzUsername,
		__in PCWSTR pwzPassword = NULL);

	HRESULT Initialize(
		__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
		__in const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
		__in const FIELD_STATE_PAIR* rgfsp,
		__in CPlainDB * db,
		__in CCSVLogger * pLogger,
		__in CMultiloginProvider * prov);
	CMultiloginCredential();

	void startAllFPDevices();
	void stopCaptureAllFPDevices();
	virtual ~CMultiloginCredential();

	static int InvokeService(const unsigned char * serviceName, void * serviceParams);

private:
	static const char* CMultiloginCredential::kPluginsPath_;
	CSessionChecker sessionChecker;
	CPlainDB * _db;
	// userlist
	std::vector<std::shared_ptr<DP_USER>> users;
	DWORD                                   _dwNumCreds;

	bool _isActivated;

	LONG                                    _cRef;

	CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR    _rgCredProvFieldDescriptors[SMFI_NUM_FIELDS];   // An array holding the 
	// type and name of each 
	// field in the tile.

	FIELD_STATE_PAIR                        _rgFieldStatePairs[SMFI_NUM_FIELDS];            // An array holding the 
	// state of each field in 
	// the tile.

	PWSTR                                 _rgFieldStrings[SMFI_NUM_FIELDS];             // An array holding the string 

	PWSTR                                  _password;
	PWSTR 									_username;
	
	DWORD									_dwActiveCreds;

	CMultiloginProvider	*					_provider;
	ICredentialProviderCredentialEvents*    _pcpce;
	CCSVLogger *							_pLogger;
	CREDENTIAL_PROVIDER_USAGE_SCENARIO    _cpus; // The usage scenario for which we were enumerated.

	std::vector<std::tuple<blm_login::FingerprintDeviceVector,
		std::shared_ptr<blm_login::IFingerprintServices>,
		std::vector<blm_login::FingerprintImage>>> _fpDevices;


	// context
	std::vector<std::shared_ptr<std::tuple<CMultiloginCredential*,	       
		int,
		int>>>  _contextVector;

	HWND hostWindow;
	//
	boost::timer::cpu_timer refresh_timer;
	boost::timer::nanosecond_type last_refresh = 0;
	boost::timer::nanosecond_type refresh_length = 0;

};
struct ScannersWindowsManager{
	HANDLE ThreadReady;
	ScannersWindowsManager(){
		initialized = false;
		started = false;
		ThreadReady = CreateEvent(NULL,TRUE,FALSE,TEXT("ThreadReady"));
	}
	~ScannersWindowsManager(){
		if(initialized){
			Terminate();
		}
	}
	bool initialized;
	bool started;
	static INT_PTR CALLBACK ToolDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI MessageThread( LPVOID lpParam );
	DWORD messageThreadId;
	std::vector<HWND> scannerWindows;
	std::map<std::shared_ptr<blm_login::IFingerprintDevice>, HWND> scannerWindowsMap;
	std::map<HWND,std::shared_ptr<Gdiplus::Bitmap>> scannerWindowsIconsMap;
	std::map<blm_login::FingerPrintDeviceStateEnum, std::shared_ptr<Gdiplus::Bitmap>> scannerStatusIconsMap;
	std::map<HWND, blm_login::FingerPrintDeviceStateEnum> scannerWindowsStatusMap;
	int LayoutStatusWnds();
	HWND hostWindow_;
	bool updateStatus;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	void InitBitmaps();
	HWND AddStatusWnd(std::shared_ptr<blm_login::IFingerprintDevice> device);
	LRESULT setWndState(std::shared_ptr<blm_login::IFingerprintDevice> device,blm_login::FingerPrintDeviceStateEnum state);
	void updateWnd(blm_login::StateMessage message);

	int Init(HWND hostWindow);
	int Terminate();
	int Start();
	int Stop();
};