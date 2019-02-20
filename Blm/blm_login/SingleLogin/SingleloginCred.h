//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// CSingleloginCred is our implementation of ICredentialProviderCredential.
// ICredentialProviderCredential is what LogonUI uses to let a credential
// provider specify what a user tile looks like and then tell it what the
// user has entered into the tile.  ICredentialProviderCredential is also
// responsible for packaging up the users credentials into a buffer that
// LogonUI then sends on to LSA.

#pragma once

#include <windows.h>
#include <strsafe.h>
#include <shlguid.h>
#include "common.h"
#include "dll.h"
#include "resource.h"

#include "PlainDB.h"
#include "CSVLogger.h"
#include "fp_device.h"
#include "graphics.h"
#include <map>
#include <boost\timer\timer.hpp>
struct ScannersWindowsManager;
class CSingleloginProvider;

typedef struct DP_USER_
{
	PWSTR username;
	PWSTR fullname;
	PWSTR password;

	std::vector<std::shared_ptr<blm_login::FingerprintTemplate>> fpTemplates;
	
	/*
	unsigned char fmt[10][MAX_FMD_SIZE];
	unsigned int fmtLen[10];
	
	unsigned char fmtSG[10][MAX_FMD_SIZE];
	unsigned int fmtSGLen[10];
	unsigned int fingerCountSG;*/
	std::string formatTypeName;
	CPlainDB::LOGIN_TYPE loginType;
}DP_USER;

class CSingleloginCred : public ICredentialProviderCredential
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
			QITABENT(CSingleloginCred, ICredentialProviderCredential), // IID_ICredentialProviderCredential
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

	DP_USER * GetUser();
	HRESULT Initialize(__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
		__in const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
		__in const FIELD_STATE_PAIR* rgfsp,
		PWSTR usrName,
		PWSTR password,
		CPlainDB::LOGIN_TYPE loginType,
		CSingleloginProvider * provider);
	HRESULT Initialize(__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
		__in const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
		__in const FIELD_STATE_PAIR* rgfsp,
		PWSTR usrName,
		PWSTR fullName,
		PWSTR password,
		CPlainDB::LOGIN_TYPE loginType,
		CSingleloginProvider * provider
		);
	HRESULT SetPromt(CPlainDB::LOGIN_TYPE loginType, bool isLogged, bool isLocked);
	CSingleloginCred(CCSVLogger * pLogger, CCSVLogger * pDbgLogger);

	virtual ~CSingleloginCred();

	void FmtCallback(int serviceIndex, int deviceIndex);

	void Activate();
	bool isActive();
	void startAllFPDevices();
	void stopCaptureAllFPDevices();
	static int InvokeService(const unsigned char * serviceName, void * serviceParams);
private:
	static const char* kPluginsPath_;
	CCSVLogger * _pLogger;

	bool									_isActivated;
	bool									_isActivationLogged;
	DP_USER _user;
	HBITMAP _logo;


	LONG                                    _cRef;

	CREDENTIAL_PROVIDER_USAGE_SCENARIO      _cpus; // The usage scenario for which we were enumerated.

	CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR    _rgCredProvFieldDescriptors[SFI_NUM_FIELDS];    // An array holding the 
	// type and name of each 
	// field in the tile.

	FIELD_STATE_PAIR                        _rgFieldStatePairs[SFI_NUM_FIELDS];             // An array holding the 
	// state of each field in 
	// the tile.

	PWSTR                                    _rgFieldStrings[SFI_NUM_FIELDS];               // An array holding the 
	// string value of each 
	// field. This is different 
	// from the name of the 
	// field held in 
	// _rgCredProvFieldDescriptors.

	ICredentialProviderCredentialEvents*    _pCredProvCredentialEvents;                     // Used to update fields.
	BOOL                                    _bChecked;                                      // Tracks the state of our 
	// checkbox.

	DWORD                                   _dwComboIndex;                                  // Tracks the current index 
	// of our combobox.

	CSingleloginProvider*				_provider;
	
	//CDPDevice								_device;

	std::vector<std::tuple<blm_login::FingerprintDeviceVector,
		                  std::shared_ptr<blm_login::IFingerprintServices>,
						  std::vector<blm_login::FingerprintImage>>> _fpDevices;


	// context
	std::vector<std::shared_ptr<std::tuple<CSingleloginCred*,	       
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
	std::map<HWND, blm_login::FingerPrintDeviceStateEnum> scannerWindowsStatusMap;
	std::map<HWND, std::shared_ptr<Gdiplus::Bitmap>> scannerWindowsIconsMap;
	std::map<blm_login::FingerPrintDeviceStateEnum, std::shared_ptr<Gdiplus::Bitmap>> scannerStatusIconsMap;
	int LayoutStatusWnds();
	HWND hostWindow_;
	bool updateStatus;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	void InitBitmaps();
	HWND AddStatusWnd(std::shared_ptr<blm_login::IFingerprintDevice> device);
	LRESULT setWndState(std::shared_ptr<blm_login::IFingerprintDevice> device,blm_login::FingerPrintDeviceStateEnum state);
	void updateWnd(blm_login::StateMessage);
	
	int Init(HWND hostWindow);
	int Terminate();
	int Start();
	int Stop();


};