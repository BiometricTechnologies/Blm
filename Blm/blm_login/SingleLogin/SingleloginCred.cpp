//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#ifndef WIN32_NO_STATUS
#include <ntstatus.h>
#define WIN32_NO_STATUS
#endif
#include <string>
#include <tuple>
#include <unknwn.h>

#include "SingleloginCred.h"
#include "SingleloginProvider.h"
#include "guid.h"
#include "plugin_framework\PluginManager.h"
#include "plugin_framework\Path.h"
#include "resource.h"
#include <atomic>

HANDLE CMPMutex;
ScannersWindowsManager ScannersWindowsManager_;
void tmplog(std::string msg){
	//std::ofstream log("C:\\Logs\\loginlog.txt",std::ios_base::app);
	
	//log << msg << std::endl;
	//log.close();
}
void ScannersWindowsManager::InitBitmaps(){
	int dispH = GetSystemMetrics(SM_CYSCREEN);
	if(true){
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::DEVICEERROR, std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG3),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::INITIALIZED, std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG2),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::OFFLINE,	   std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG2),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::ONLINE,      std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG4),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::SCANNING,    std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG5),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::VERIFIED,    std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG8),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::BADIMAGE,    std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG6),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::BADFINGER,   std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG7),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::CONNECTING,  std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG9),L"JPG"))));
	}
	else{
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::DEVICEERROR, std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG3_128),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::INITIALIZED, std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG2_128),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::OFFLINE,	   std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG2_128),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::ONLINE,      std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG4_128),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::SCANNING,    std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG5_128),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::VERIFIED,    std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG8_128),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::BADIMAGE,    std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG6_128),L"JPG"))));
		scannerStatusIconsMap.insert(
			std::make_pair(blm_login::BADFINGER,   std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(HINST_THISDLL,MAKEINTRESOURCE(IDR_JPG7_128),L"JPG"))));

	}

}
INT_PTR CALLBACK ScannersWindowsManager::ToolDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	bool updateImage=false;
	blm_login::FingerPrintDeviceStateEnum newstatus;
	blm_login::FingerPrintDeviceStateEnum oldstatus;
	BOOL ret = FALSE;
	switch (Message){
	case WM_CTLCOLORDLG:
		ret = (BOOL) GetStockObject(NULL_BRUSH);
		break;
	case WM_INITDIALOG:
		ret = TRUE;
		break;
	case WM_TIMER:
		switch (wParam)
		{
		case blm_login::BADIMAGE:
			KillTimer(hwnd,blm_login::BADIMAGE);
			break;
		case blm_login::BADFINGER:
			KillTimer(hwnd,blm_login::BADFINGER);
			break;
		}
		newstatus = blm_login::ONLINE;
		updateImage = true;
		ret = TRUE;
		break;
	case  WM_USER+100:
		updateImage = true;
		oldstatus = ScannersWindowsManager_.scannerWindowsStatusMap[hwnd];
		newstatus = blm_login::FingerPrintDeviceStateEnum(wParam);
		switch (newstatus)
		{
		case blm_login::ONLINE:
			if((oldstatus==blm_login::BADIMAGE) || (oldstatus==blm_login::BADFINGER)){
				updateImage = false;
			}
			break;
		case blm_login::BADIMAGE:
			if(oldstatus==blm_login::BADFINGER){
				updateImage=false;
			}
			else{
				SetTimer(hwnd,blm_login::BADIMAGE,1000,NULL);
			}
			break;
		case blm_login::BADFINGER:
			SetTimer(hwnd,blm_login::BADFINGER,1000,NULL);
			break;
		case blm_login::OFFLINE:
			if(oldstatus==blm_login::VERIFIED){
				break;
			}
		case blm_login::INITIALIZED:
			if(oldstatus==blm_login::VERIFIED){
				break;
			}
		case blm_login::DEVICEERROR:
		case blm_login::VERIFIED:
			KillTimer(hwnd,blm_login::BADIMAGE);
			KillTimer(hwnd,blm_login::BADFINGER);
			break;
		}
		ret = TRUE;
		break;
	}
	if(updateImage){
		ScannersWindowsManager_.scannerWindowsStatusMap[hwnd] = newstatus;
		Gdiplus::Bitmap *stateBMP = ScannersWindowsManager_.scannerStatusIconsMap[blm_login::FingerPrintDeviceStateEnum(newstatus)].get();
		if(stateBMP){
			HBITMAP hbmp;
			std::shared_ptr<Gdiplus::Bitmap> scannerBmp = ScannersWindowsManager_.scannerWindowsIconsMap[hwnd];
			HDC hDCdst = GetDC(hwnd);
			HDC hDCsrc = CreateCompatibleDC(NULL);
			HDC hDCBuffer = CreateCompatibleDC(hDCdst);
			HBITMAP hBMBuffer = CreateCompatibleBitmap(hDCdst,stateBMP->GetWidth(), stateBMP->GetHeight());
			SelectObject(hDCBuffer,hBMBuffer);
			stateBMP->GetHBITMAP(0,&hbmp);   
			SelectObject(hDCsrc,hbmp);
			BitBlt(hDCBuffer, 0, 0, stateBMP->GetWidth(), stateBMP->GetHeight(),
				hDCsrc, 0, 0, SRCCOPY);
			DeleteObject(hbmp);

			scannerBmp->GetHBITMAP(0,&hbmp);   
			SelectObject(hDCsrc,hbmp);
			int posx = (stateBMP->GetWidth() - scannerBmp->GetWidth())/2;
			int posy = (stateBMP->GetHeight()/2 - scannerBmp->GetHeight())/2;
			BLENDFUNCTION bfn = {0};
			bfn.BlendOp = AC_SRC_OVER;
			bfn.BlendFlags = 0;
			bfn.SourceConstantAlpha = 255;
			bfn.AlphaFormat = AC_SRC_ALPHA;
			GdiAlphaBlend(hDCBuffer, posx, posy, scannerBmp->GetWidth(), scannerBmp->GetHeight(),
				hDCsrc,    0,    0, scannerBmp->GetWidth(), scannerBmp->GetHeight(), bfn);
			DeleteObject(hbmp);

			DeleteDC(hDCsrc);
			BitBlt(hDCdst,0,0,stateBMP->GetWidth(), stateBMP->GetHeight(),hDCBuffer,0,0,SRCCOPY);
			DeleteObject(hBMBuffer);
			DeleteDC(hDCBuffer);
			ReleaseDC(hwnd,hDCdst);
		}
	}

	return ret;
}
HWND ScannersWindowsManager::AddStatusWnd(std::shared_ptr<blm_login::IFingerprintDevice> device){
	HWND scannerWindow=NULL;
	scannerWindowsMap.insert(std::make_pair(device, scannerWindow));
	return S_OK;
}
int ScannersWindowsManager::LayoutStatusWnds(){
	int count = scannerWindowsMap.size();
	if(count){
		std::map<std::wstring, HWND> actualWindows;
		for(auto& dev: scannerWindowsMap)
		{
			std::wstring dllName = dev.first->DLLName();
			std::wstring name = dllName;
			std::shared_ptr<Gdiplus::Bitmap> bmp;
			if(true){
				auto mHandle = GetModuleHandle(dllName.c_str());
				bmp = std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(mHandle, MAKEINTRESOURCE(101), L"PNG"));
				if (bmp == NULL){

				}
			}
			auto scannerWindowIt = actualWindows.find(name);
			if (scannerWindowIt!=actualWindows.end()){
				scannerWindowsMap[dev.first] = scannerWindowIt->second;
			}
			else{
				HWND scannerWindow = CreateDialog(HINST_THISDLL, MAKEINTRESOURCE(IDD_SCANNER), hostWindow_, ToolDlgProc);
				scannerWindowsMap[dev.first] = scannerWindow;
				scannerWindows.push_back(scannerWindow);
				scannerWindowsIconsMap.insert(std::make_pair(scannerWindow, bmp));
				actualWindows.insert(std::make_pair(name, scannerWindow));
			}
		}
		int deltaW = 8;
		int dispW = GetSystemMetrics(SM_CXSCREEN);
		int dispH = GetSystemMetrics(SM_CYSCREEN);
		int winW = scannerStatusIconsMap[blm_login::DEVICEERROR]->GetWidth();
		int winH = scannerStatusIconsMap[blm_login::DEVICEERROR]->GetHeight();
		int posx = (dispW - (scannerWindows.size()*(winW)+(scannerWindows.size() - 1)*deltaW)) / 2;
		for (auto& scannerWindow : scannerWindows)
		{
			SetWindowPos(scannerWindow, HWND_TOP, posx, 20, winW, winH, SWP_SHOWWINDOW);
			SetLayeredWindowAttributes(scannerWindow, RGB(255, 255, 255), 250, LWA_ALPHA);
			posx += winW + deltaW;
		}
	}
	return count;
}


DWORD WINAPI ScannersWindowsManager::MessageThread( LPVOID lpParam )
{
	BOOL bRet =0 ;
	MSG msg;
	bool isDlgMsg;
	ScannersWindowsManager* host = reinterpret_cast<ScannersWindowsManager*>(lpParam);
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	SetEvent(host->ThreadReady);
	host->LayoutStatusWnds();
	host->updateStatus = true;
	SetFocus(host->hostWindow_);
	tmplog("entering thread");
	while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
	{ 
		if (bRet == -1)
		{
			// handle the error and possibly exit
		}
		else
		{

			isDlgMsg = false;
			if(msg.message == WM_USER+100){
				blm_login::StateMessage stateMsg((blm_login::IFingerprintDevice*)msg.wParam, (blm_login::FingerPrintDeviceStateEnum)msg.lParam);
				
				if(stateMsg.device_ == nullptr){
					if(stateMsg.state_ == blm_login::OFFLINE){
						for(auto& dev: host->scannerWindowsMap)
						{
							ShowWindow(dev.second,SW_HIDE);
						}
					}
					else{
						for(auto& dev :host->scannerWindowsMap)
						{
							blm_login::FingerPrintDeviceStateEnum teststate = dev.first->getState();
							LRESULT res = SendMessage(dev.second,WM_USER+100,dev.first->getState(),0);
						}
					}
				}
				else{
					
					blm_login::IFingerprintDevice* device = stateMsg.device_;
					blm_login::FingerPrintDeviceStateEnum state = stateMsg.state_;
					for(auto& dev: host->scannerWindowsMap)
					{
						if(	dev.first.get() == device){
							LRESULT res = SendMessage(dev.second,WM_USER+100,state,0);
						}
						else if(state == blm_login::VERIFIED){
							ShowWindow(dev.second,SW_HIDE);
						}
					}
				}
				isDlgMsg = true;
			}

			if(!isDlgMsg){
				TranslateMessage(&msg); 
				DispatchMessage(&msg); 
			}
		}
	} 
	for(HWND wnd:host->scannerWindows)
	{
		DestroyWindow(wnd);
	}
	host->scannerWindowsMap.clear();
	host->scannerWindowsIconsMap.clear();
	host->scannerWindows.clear();
	
	host->updateStatus=false;
	tmplog("exiting thread");
	return bRet;
}

LRESULT ScannersWindowsManager::setWndState( std::shared_ptr<blm_login::IFingerprintDevice> device,blm_login::FingerPrintDeviceStateEnum state )
{
	LRESULT res = 1;
	if(updateStatus){
		res = SendMessage(scannerWindowsMap[device],WM_USER+100,state,0);
	}
	return res;
}

void ScannersWindowsManager::updateWnd(blm_login::StateMessage message)
{
	PostThreadMessage(messageThreadId, WM_USER+100,(WPARAM)message.device_,(LPARAM)message.state_);
}

int ScannersWindowsManager::Init(HWND hostWindow)
{
	if(!initialized){
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		InitBitmaps();
		hostWindow_ = hostWindow;
		initialized = true;
	}
	return 0;
}
int ScannersWindowsManager::Terminate()
{
	if(!initialized){
		return -1;
	}
	if(started){
		Stop();
	}
	scannerStatusIconsMap.clear();
	Gdiplus::GdiplusShutdown(gdiplusToken);
	initialized = false;
	return 0;
}

int ScannersWindowsManager::Start()
{
	if(!initialized){
		return -1;
	}
	CreateThread(NULL,0,MessageThread,this,0,&messageThreadId);
	WaitForSingleObject(ThreadReady,INFINITE);
	started = true;

	return 0;
}
int ScannersWindowsManager::Stop()
{
	if(!started){
		return -1;
	}
	tmplog("stopping thread");
	PostThreadMessage(DWORD(messageThreadId),WM_QUIT,0,0);
	while(updateStatus){
		tmplog("waiting thread");
	}
	started = false;
	tmplog("scanmaganaged stopped");
	return 0;
}
///////

int CSingleloginCred::InvokeService(const unsigned char * serviceName, void * serviceParams)
{
	if(strcmp((char*)serviceName,"state") == 0){
		ScannersWindowsManager_.updateWnd(*(blm_login::StateMessage*)serviceParams);
	}
	return 0;
}

// CSingleloginCred ////////////////////////////////////////////////////////

const char* CSingleloginCred::kPluginsPath_ = "System32\\IdentaZone\\IdentaMaster";

CSingleloginCred::CSingleloginCred(CCSVLogger * pLogger, CCSVLogger * pDbgLogger):
	_cRef(1),
	_pCredProvCredentialEvents(NULL)
{
	_pLogger = pLogger;
	DllAddRef();

	ZeroMemory(_rgCredProvFieldDescriptors, sizeof(_rgCredProvFieldDescriptors));
	ZeroMemory(_rgFieldStatePairs, sizeof(_rgFieldStatePairs));
	ZeroMemory(_rgFieldStrings, sizeof(_rgFieldStrings));
	_bChecked = FALSE;
	_dwComboIndex = 0;
	_logo = NULL;
	_isActivated = false;
	_isActivationLogged = false;
	hostWindow = NULL;

}

CSingleloginCred::~CSingleloginCred()
{
	stopCaptureAllFPDevices();
	if (_rgFieldStrings[SFI_PASSWORD])
	{
		size_t lenPassword = lstrlen(_rgFieldStrings[SFI_PASSWORD]);
		SecureZeroMemory(_rgFieldStrings[SFI_PASSWORD], lenPassword * sizeof(*_rgFieldStrings[SFI_PASSWORD]));
	}
	for (int i = 0; i < ARRAYSIZE(_rgFieldStrings); i++)
	{
		CoTaskMemFree(_rgFieldStrings[i]);
		CoTaskMemFree(_rgCredProvFieldDescriptors[i].pszLabel);
	}

	if(_user.password)
	{
		size_t lenPassword = lstrlen(_user.password);
		SecureZeroMemory(_user.password, lenPassword * sizeof(*_user.password));
	}

	CoTaskMemFree(_user.username);
	CoTaskMemFree(_user.password);
	
	
	DllRelease();
}


void fmtCallback(void * context){
	
	WaitForSingleObject(CMPMutex,INFINITE);
	std::tuple<CSingleloginCred*, int, int>* contextTuple = reinterpret_cast<std::tuple<CSingleloginCred*, int, int>*>(context);
	int serviceIdx = std::get<1>(*contextTuple);
	int deviceIdx = std::get<2>(*contextTuple);

	std::get<0>(*contextTuple)->FmtCallback(serviceIdx, deviceIdx);
	ReleaseMutex(CMPMutex);
}


void CSingleloginCred::FmtCallback( int serviceIndex, int deviceIndex )
{	
	using  namespace blm_login;
	if(_isActivated){
		return;
	}
	try{
		
		std::shared_ptr<IFingerprintServices> service;
		std::shared_ptr<IFingerprintDevice> device;
		FingerprintImage *img;
		device = std::get<0>(_fpDevices.at(serviceIndex)).at(deviceIndex);
		service = std::get<1>(_fpDevices.at(serviceIndex));
		img = &(std::get<2>(_fpDevices.at(serviceIndex)).at(deviceIndex));
	    if(service->matchTemplates(*img, this->_user.fpTemplates)){
			ScannersWindowsManager_.updateWnd(blm_login::StateMessage(device.get(),blm_login::FingerPrintDeviceStateEnum::VERIFIED));
			std::wstring providerName = L"";//std::get<0>(_fpDevices.at(serviceIndex)).at(deviceIndex)->getProviderName();
			_pLogger->DBLOG("User \"" << std::wstring(_user.username) << "\" logged in using biometrics " << "("<< device->Name() + providerName<< ")");
			_isActivationLogged = true;
    		Activate();
		    _provider->Activate();
		}
		else{
			ScannersWindowsManager_.setWndState(device,blm_login::FingerPrintDeviceStateEnum::BADFINGER);
		}
	}
	catch(const std::exception& ex){
		LOG(INFO) << ex.what();
	}
	catch(...){
		LOG(INFO) << "Fatal error:";

	}
}


DP_USER * CSingleloginCred::GetUser(){
	return &_user;
}

bool CSingleloginCred::isActive()
{
	return _isActivated;
}

void CSingleloginCred::Activate(){
	_isActivated = true;
	//stopCaptureAllFPDevices();

	LOG(INFO) << "Activated";
}


HRESULT CSingleloginCred::Initialize(__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
									 __in const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
									 __in const FIELD_STATE_PAIR* rgfsp,
									 PWSTR usrName,
									 PWSTR fullName,
									 PWSTR password,
									 CPlainDB::LOGIN_TYPE loginType,
									 CSingleloginProvider * provider)
{
	HRESULT hr = S_OK;
	hr = Initialize(cpus, rgcpfd, rgfsp,usrName,password, loginType, provider);
	if(SUCCEEDED(hr)){
		hr = SHStrDupW(fullName,&_user.fullname);
		hr = SHStrDupW(fullName, &_rgFieldStrings[SFI_LARGE_TEXT]);
	}
	return hr;
}


// Initializes one credential with the field information passed in.
// Set the value of the SFI_LARGE_TEXT field to pwzUsername.
void loggingInit(CCSVLogger * logger, LPWSTR logfile);
HRESULT CSingleloginCred::SetPromt( CPlainDB::LOGIN_TYPE loginType, bool isLogged, bool isLocked )
{
	HRESULT hr;
	if(isLocked){
		if(loginType == CPlainDB::LOGIN_BIOMETRICS)
		{
			hr = SHStrDupW(L"Touch the scanner to unlock workstation", &_rgFieldStrings[SFI_SMALL_TEXT]);
		}else if(loginType == CPlainDB::LOGIN_MIXED){
			hr = SHStrDupW(L"Touch the scanner or enter password to unlock workstation", &_rgFieldStrings[SFI_SMALL_TEXT]);
		}else{
			hr = SHStrDupW(L"Enter password to unlock workstation", &_rgFieldStrings[SFI_SMALL_TEXT]);
		}
		if(hr==S_OK){
			hr = SHStrDupW(L"Locked", &_rgFieldStrings[SFI_STATE_TEXT]);
		}
	}
	else{
		if(loginType == CPlainDB::LOGIN_BIOMETRICS)
		{
			hr = SHStrDupW(L"Touch the scanner to login", &_rgFieldStrings[SFI_SMALL_TEXT]);
		}else if(loginType == CPlainDB::LOGIN_MIXED){
			hr = SHStrDupW(L"Touch the scanner or enter password to login", &_rgFieldStrings[SFI_SMALL_TEXT]);
		}else{
			hr = SHStrDupW(L"Enter password to login", &_rgFieldStrings[SFI_SMALL_TEXT]);
		}
		if(hr == S_OK){
			if(isLogged){
				hr = SHStrDupW(L"Logged on", &_rgFieldStrings[SFI_STATE_TEXT]);
			}
			else{
				hr = SHStrDupW(L"", &_rgFieldStrings[SFI_STATE_TEXT]);
			}
		}
	}
	return hr;
}

HRESULT CSingleloginCred::Initialize(
	__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
	__in const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
	__in const FIELD_STATE_PAIR* rgfsp,
	PWSTR usrName,
	PWSTR password,
	CPlainDB::LOGIN_TYPE loginType,
	CSingleloginProvider * provider
	)
{
	HRESULT hr = S_OK;
	_cpus = cpus;
	bool passOnly;
	if(loginType == CPlainDB::LOGIN_PASSWORD)
		passOnly = true;
	else
		passOnly = false;

	// Copy the field descriptors for each field. This is useful if you want to vary the field
	// descriptors based on what Usage scenario the credential was created for.
	for (DWORD i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(_rgCredProvFieldDescriptors); i++)
	{
		_rgFieldStatePairs[i] = rgfsp[i];
		if((loginType != CPlainDB::LOGIN_BIOMETRICS) && (i == SFI_PASSWORD)){
			_rgFieldStatePairs[i].cpfis = CPFIS_FOCUSED;
		}
		if(passOnly &&
			((i==SFI_REFRESH_PROMT) || (i == SFI_COMMAND_LINK)))
		{
			_rgFieldStatePairs[i].cpfs = CPFS_HIDDEN;
		}
		hr = FieldDescriptorCopy(rgcpfd[i], &_rgCredProvFieldDescriptors[i]);
	}


	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(usrName,&_user.username);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(password,&_user.password);
	}
	if (SUCCEEDED(hr))
	{
		_user.loginType = loginType;
	}

	// Initialize the String value of all the fields. 
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(usrName, &_rgFieldStrings[SFI_LARGE_TEXT]);
	}


	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Edit Text", &_rgFieldStrings[SFI_EDIT_TEXT]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"", &_rgFieldStrings[SFI_PASSWORD]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Submit", &_rgFieldStrings[SFI_SUBMIT_BUTTON]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Checkbox", &_rgFieldStrings[SFI_CHECKBOX]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Combobox", &_rgFieldStrings[SFI_COMBOBOX]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Check if scanner is attached. Press ‘Refresh’ after reconnecting device", &(_rgFieldStrings[SFI_REFRESH_PROMT]));
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Refresh", &_rgFieldStrings[SFI_COMMAND_LINK]);
	}

	if(_user.loginType == CPlainDB::LOGIN_BIOMETRICS)
	{
		_rgFieldStatePairs[SFI_PASSWORD].cpfs = CPFS_HIDDEN;
	}
	_provider = provider;
	return S_OK;
}

HRESULT CSingleloginCred::Advise(
	__in ICredentialProviderCredentialEvents* pcpce
	)
{
	if (_pCredProvCredentialEvents != NULL)
	{
		_pCredProvCredentialEvents->Release();
	}
	_pCredProvCredentialEvents = NULL;
//	_pCredProvCredentialEvents = pcpce;
	//_pCredProvCredentialEvents->AddRef();
	pcpce->QueryInterface(IID_PPV_ARGS(&_pCredProvCredentialEvents));
	_pCredProvCredentialEvents->OnCreatingWindow(&hostWindow);
	return S_OK;
}

// LogonUI calls this to tell us to release the callback.
HRESULT CSingleloginCred::UnAdvise()
{
	if (_pCredProvCredentialEvents)
	{
		_pCredProvCredentialEvents->Release();
	}
	_pCredProvCredentialEvents = NULL;
	return S_OK;
}




// LogonUI calls this function when our tile is selected (zoomed)
// If you simply want fields to show/hide based on the selected state,
// there's no need to do anything here - you can set that up in the 
// field definitions. But if you want to do something
// more complicated, like change the contents of a field when the tile is
// selected, you would do it here.
HRESULT CSingleloginCred::SetSelected(__out BOOL* pbAutoLogon)  
{
	last_refresh = 0;
	refresh_length = 0;
	if(_isActivated){
		*pbAutoLogon = TRUE;  
		return S_OK;
	}else{
		//if(_device.Init() == 0){
		CMPMutex = CreateMutex(NULL,FALSE,NULL);
		ScannersWindowsManager_.Init(hostWindow);
		if(_user.loginType == CPlainDB::LOGIN_BIOMETRICS || _user.loginType == CPlainDB::LOGIN_MIXED){
			try{
				startAllFPDevices();
			}
			catch(const std::exception& e){
				LOG(INFO) << "exception during devices initialization:" << e.what();
			}

		}
		*pbAutoLogon = FALSE;
	}

	return S_OK;
}

// Similarly to SetSelected, LogonUI calls this when your tile was selected
// and now no longer is. The most common thing to do here (which we do below)
// is to clear out the password field.
HRESULT CSingleloginCred::SetDeselected()
{
	HRESULT hr = S_OK;
	_provider->updateTiles();
	if (_rgFieldStrings[SFI_PASSWORD])
	{
		size_t lenPassword = lstrlen(_rgFieldStrings[SFI_PASSWORD]);
		SecureZeroMemory(_rgFieldStrings[SFI_PASSWORD], lenPassword * sizeof(*_rgFieldStrings[SFI_PASSWORD]));

		CoTaskMemFree(_rgFieldStrings[SFI_PASSWORD]);
		hr = SHStrDupW(L"", &_rgFieldStrings[SFI_PASSWORD]);

		if (SUCCEEDED(hr) && _pCredProvCredentialEvents)
		{
			_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, _rgFieldStrings[SFI_PASSWORD]);
		}
	}
	if(_user.loginType == CPlainDB::LOGIN_BIOMETRICS || _user.loginType == CPlainDB::LOGIN_MIXED){
		ScannersWindowsManager_.updateWnd(blm_login::StateMessage(nullptr,blm_login::OFFLINE));
		stopCaptureAllFPDevices();
		ScannersWindowsManager_.Terminate();
		_contextVector.clear();
		_fpDevices.clear();
	}
	return hr;
}

// Get info for a particular field of a tile. Called by logonUI to get information 
// to display the tile.
HRESULT CSingleloginCred::GetFieldState(
	__in DWORD dwFieldID,
	__out CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
	__out CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis
	)
{
	HRESULT hr;

	// Validate our parameters.
	if ((dwFieldID < ARRAYSIZE(_rgFieldStatePairs)) && pcpfs && pcpfis)
	{
		*pcpfs = _rgFieldStatePairs[dwFieldID].cpfs;
		*pcpfis = _rgFieldStatePairs[dwFieldID].cpfis;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}
	return hr;
}

// Sets ppwsz to the string value of the field at the index dwFieldID
HRESULT CSingleloginCred::GetStringValue(
	__in DWORD dwFieldID, 
	__deref_out PWSTR* ppwsz
	)
{
	HRESULT hr;

	// Check to make sure dwFieldID is a legitimate index
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && ppwsz) 
	{
		// Make a copy of the string and return that. The caller
		// is responsible for freeing it.
		hr = SHStrDupW(_rgFieldStrings[dwFieldID], ppwsz);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Get the image to show in the user tile
HRESULT CSingleloginCred::GetBitmapValue(
	__in DWORD dwFieldID, 
	__out HBITMAP* phbmp
	)
{
	HRESULT hr;
	if ((SFI_TILEIMAGE == dwFieldID) && phbmp)
	{
		HBITMAP hbmp = LoadBitmap(HINST_THISDLL, MAKEINTRESOURCE(IDB_TILE_IMAGE));
	
		if(hbmp!=NULL){
			*phbmp = hbmp;
			hr=S_OK;
		}
		else{
			UNREFERENCED_PARAMETER(dwFieldID);
			UNREFERENCED_PARAMETER(phbmp);
			hr = E_NOTIMPL;
		}
		
	}
	else
	{
		hr = E_INVALIDARG;
	}
	return hr;
}

// Sets pdwAdjacentTo to the index of the field the submit button should be 
// adjacent to. We recommend that the submit button is placed next to the last
// field which the user is required to enter information in. Optional fields
// should be below the submit button.
HRESULT CSingleloginCred::GetSubmitButtonValue(
	__in DWORD dwFieldID,
	__out DWORD* pdwAdjacentTo
	)
{
	HRESULT hr;

	if (SFI_SUBMIT_BUTTON == dwFieldID && pdwAdjacentTo)
	{
		// pdwAdjacentTo is a pointer to the fieldID you want the submit button to 
		// appear next to.
		*pdwAdjacentTo = SFI_PASSWORD;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}
	return hr;
}

// Sets the value of a field which can accept a string as a value.
// This is called on each keystroke when a user types into an edit field
HRESULT CSingleloginCred::SetStringValue(
	__in DWORD dwFieldID, 
	__in PCWSTR pwz      
	)
{
	HRESULT hr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && 
		(CPFT_EDIT_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft || 
		CPFT_PASSWORD_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft)) 
	{
		PWSTR* ppwszStored = &_rgFieldStrings[dwFieldID];
		CoTaskMemFree(*ppwszStored);
		hr = SHStrDupW(pwz, ppwszStored);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Returns whether a checkbox is checked or not as well as its label.
HRESULT CSingleloginCred::GetCheckboxValue(
	__in DWORD dwFieldID, 
	__in BOOL* pbChecked,
	__deref_out PWSTR* ppwszLabel
	)
{
	HRESULT hr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && 
		(CPFT_CHECKBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		*pbChecked = _bChecked;
		hr = SHStrDupW(_rgFieldStrings[SFI_CHECKBOX], ppwszLabel);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Sets whether the specified checkbox is checked or not.
HRESULT CSingleloginCred::SetCheckboxValue(
	__in DWORD dwFieldID, 
	__in BOOL bChecked
	)
{
	HRESULT hr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && 
		(CPFT_CHECKBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		_bChecked = bChecked;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Returns the number of items to be included in the combobox (pcItems), as well as the 
// currently selected item (pdwSelectedItem).
HRESULT CSingleloginCred::GetComboBoxValueCount(
	__in DWORD dwFieldID, 
	__out DWORD* pcItems, 
	__out_range(<,*pcItems) DWORD* pdwSelectedItem
	)
{
	HRESULT hr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && 
		(CPFT_COMBOBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		switch(_user.loginType){
		case CPlainDB::LOGIN_BIOMETRICS:
		case CPlainDB::LOGIN_PASSWORD:
			*pcItems = 1;
			*pdwSelectedItem = 0;
			break;
		case CPlainDB::LOGIN_MIXED:
			*pcItems = 2;
			*pdwSelectedItem = 0;
			break;
		case CPlainDB::LOGIN_UNKNOWN:
		default:
			*pcItems = 0;
			*pdwSelectedItem = 0;
			break;
		}
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return S_OK;
}

// Called iteratively to fill the combobox with the string (ppwszItem) at index dwItem.
HRESULT CSingleloginCred::GetComboBoxValueAt(
	__in DWORD dwFieldID, 
	__in DWORD dwItem,
	__deref_out PWSTR* ppwszItem
	)
{
	HRESULT hr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && 
		(CPFT_COMBOBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		switch(_user.loginType){
		case CPlainDB::LOGIN_BIOMETRICS:
			hr = SHStrDupW(s_rgComboBoxStrings[1], ppwszItem);
			break;
		case CPlainDB::LOGIN_PASSWORD:
			hr = SHStrDupW(s_rgComboBoxStrings[0], ppwszItem);
			break;
		case CPlainDB::LOGIN_MIXED:
			hr = SHStrDupW(s_rgComboBoxStrings[dwItem], ppwszItem);
			break;
		}

	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Called when the user changes the selected item in the combobox.
HRESULT CSingleloginCred::SetComboBoxSelectedValue(
	__in DWORD dwFieldID,
	__in DWORD dwSelectedItem
	)
{
	HRESULT hr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && 
		(CPFT_COMBOBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		_dwComboIndex = dwSelectedItem;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Called when the user clicks a command link.
HRESULT CSingleloginCred::CommandLinkClicked(__in DWORD dwFieldID)
{
	HRESULT hr;
	// Validate parameter.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && 
		(CPFT_COMMAND_LINK == _rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		const auto before_times(refresh_timer.elapsed());
		boost::timer::nanosecond_type before_refresh = before_times.wall;
		if ((refresh_length == 0) || (before_refresh - last_refresh > 500000000LL)){
			
			stopCaptureAllFPDevices();
			_contextVector.clear();
			_fpDevices.clear();
			startAllFPDevices();
			const auto after_times = refresh_timer.elapsed();
			last_refresh = after_times.wall;
			refresh_length = last_refresh - before_refresh;
		}
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Collect the username and password into a serialized credential for the correct usage scenario 
// (logon/unlock is what's demonstrated in this sample).  LogonUI then passes these credentials 
// back to the system to log on.
HRESULT CSingleloginCred::GetSerialization(
	__out CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
	__out CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
	__deref_out_opt PWSTR* ppwszOptionalStatusText, 
	__in CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
	)
{
	UNREFERENCED_PARAMETER(ppwszOptionalStatusText);
	UNREFERENCED_PARAMETER(pcpsiOptionalStatusIcon);

	HRESULT hr;

	WCHAR wsz[MAX_COMPUTERNAME_LENGTH+1];
	DWORD cch = ARRAYSIZE(wsz);
	if (GetComputerNameW(wsz, &cch))
	{
		PWSTR pwzProtectedPassword;
		if(_isActivated){
			hr = ProtectIfNecessaryAndCopyPassword(_user.password, _cpus, &pwzProtectedPassword);
		}else{
			hr = ProtectIfNecessaryAndCopyPassword(_rgFieldStrings[SFI_PASSWORD], _cpus, &pwzProtectedPassword);
		}


		if (SUCCEEDED(hr))
		{
			KERB_INTERACTIVE_UNLOCK_LOGON kiul;

			hr = KerbInteractiveUnlockLogonInit(wsz, _user.username, pwzProtectedPassword, _cpus, &kiul);

			if (SUCCEEDED(hr))
			{
				// We use KERB_INTERACTIVE_UNLOCK_LOGON in both unlock and logon scenarios.  It contains a
				// KERB_INTERACTIVE_LOGON to hold the creds plus a LUID that is filled in for us by Winlogon
				// as necessary.
				hr = KerbInteractiveUnlockLogonPack(kiul, &pcpcs->rgbSerialization, &pcpcs->cbSerialization);

				if (SUCCEEDED(hr))
				{
					ULONG ulAuthPackage;
					hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);
					if (SUCCEEDED(hr))
					{
						pcpcs->ulAuthenticationPackage = ulAuthPackage;
						pcpcs->clsidCredentialProvider = DLL_GUID;

						// At this point the credential has created the serialized credential used for logon
						// By setting this to CPGSR_RETURN_CREDENTIAL_FINISHED we are letting logonUI know
						// that we have all the information we need and it should attempt to submit the 
						// serialized credential.
						*pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
					}
				}
			}

			CoTaskMemFree(pwzProtectedPassword);
		}
	}
	else
	{
		DWORD dwErr = GetLastError();
		hr = HRESULT_FROM_WIN32(dwErr);
	}

	return hr;
}

struct REPORT_RESULT_STATUS_INFO
{
	NTSTATUS ntsStatus;
	NTSTATUS ntsSubstatus;
	PWSTR     pwzMessage;
	CREDENTIAL_PROVIDER_STATUS_ICON cpsi;
};

static const REPORT_RESULT_STATUS_INFO s_rgLogonStatusInfo[] =
{
	{ STATUS_LOGON_FAILURE, STATUS_SUCCESS, L"Incorrect password or username.", CPSI_ERROR, },
	{ STATUS_ACCOUNT_RESTRICTION, STATUS_ACCOUNT_DISABLED, L"The account is disabled.", CPSI_WARNING },
};

// ReportResult is completely optional.  Its purpose is to allow a credential to customize the string
// and the icon displayed in the case of a logon failure.  For example, we have chosen to 
// customize the error shown in the case of bad username/password and in the case of the account
// being disabled.
HRESULT CSingleloginCred::ReportResult(
	__in NTSTATUS ntsStatus, 
	__in NTSTATUS ntsSubstatus,
	__deref_out_opt PWSTR* ppwszOptionalStatusText, 
	__out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
	)
{
	*ppwszOptionalStatusText = NULL;
	*pcpsiOptionalStatusIcon = CPSI_NONE;


	DWORD dwStatusInfo = (DWORD)-1;

	// Look for a match on status and substatus.
	for (DWORD i = 0; i < ARRAYSIZE(s_rgLogonStatusInfo); i++)
	{
		if (s_rgLogonStatusInfo[i].ntsStatus == ntsStatus && s_rgLogonStatusInfo[i].ntsSubstatus == ntsSubstatus)
		{
			dwStatusInfo = i;
			break;
		}
	}

	if ((DWORD)-1 != dwStatusInfo)
	{
		if (SUCCEEDED(SHStrDupW(s_rgLogonStatusInfo[dwStatusInfo].pwzMessage, ppwszOptionalStatusText)))
		{
			*pcpsiOptionalStatusIcon = s_rgLogonStatusInfo[dwStatusInfo].cpsi;
		}
	}

	// If we failed the logon, try to erase the password field.
	if (!SUCCEEDED(HRESULT_FROM_NT(ntsStatus)))
	{
		if (_isActivationLogged){
			_pLogger->Undo();
			_isActivationLogged = false;
		}
		_isActivated = false;
		ScannersWindowsManager_.updateWnd(blm_login::StateMessage(nullptr,blm_login::OFFLINE));
		stopCaptureAllFPDevices();
		if (_pCredProvCredentialEvents)
		{
			_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, L"");
		}
	}else if(!_isActivationLogged){
		_pLogger->DBLOG("User \"" << std::wstring(_user.username) << "\" logged in using password");
		_isActivationLogged =true;
	}

	// Since NULL is a valid value for *ppwszOptionalStatusText and *pcpsiOptionalStatusIcon
	// this function can't fail.
	return S_OK;
}

void CSingleloginCred::stopCaptureAllFPDevices()
{
	// stop all the devices
	for(unsigned i = 0; i < _fpDevices.size(); ++i){
		for(unsigned j = 0; j < std::get<0>(_fpDevices.at(i)).size(); ++j){
			try{
				std::get<0>(_fpDevices.at(i)).at(j)->stopCapture();
			}
			catch(const std::exception& ex){
				LOG(ERROR) << "Exception occurred trying to stop all fingerprint devices: " << ex.what();
			}
		}
	}
	ScannersWindowsManager_.Stop();
	while(ScannersWindowsManager_.updateStatus){
		;
	}
}

void CSingleloginCred::startAllFPDevices()
{

	using  namespace blm_login;

	
	// Plugin manager initialization, load all plugins from plugins directory 
	// TODO: release apr! (an.skornyakov@gmail.com)

	if(_fpDevices.empty()) {
	    ::apr_initialize();
	    PluginManager &pm = PluginManager::getInstance();
		PF_ObjectParams objectParams;
		CHAR sysdir[MAX_PATH];
		GetSystemWindowsDirectoryA(sysdir,MAX_PATH);
	    auto platformServices = pm.getPlatformServices();
		platformServices.invokeService = CSingleloginCred::InvokeService;
		pm.setPlatformServices(platformServices);
		objectParams.platformServices = &platformServices;
	    pm.loadAll(Path::makeAbsolute(std::string(sysdir) + Path::sep + kPluginsPath_));

	    // get object(fpServices) registration map, create services, get devices from all the services
	    auto regMap = pm.getRegistrationMap();
	    for(std::map<std::string, PF_RegisterParams>::iterator it = regMap.begin(); it != regMap.end(); ++it) {
			try{
				std::shared_ptr<IFingerprintServices> fpService = ((it->second.createFunc(&objectParams)));
				auto fpDeviceVector = fpService->getDevices();
				std::vector<FingerprintImage> fpDeviceImageVector(fpDeviceVector.size());
				this->_fpDevices.push_back(std::make_tuple(fpDeviceVector, fpService, fpDeviceImageVector));
			}
			catch (...){
				continue;
			}
	    }

	    
	} else {
		std::vector<FingerprintImage> imageVector;
		for(unsigned i = 0; i < _fpDevices.size(); ++i){
			std::get<0>(_fpDevices.at(i)).clear();
			std::get<2>(_fpDevices.at(i)).clear();
			std::shared_ptr<IFingerprintServices> service = std::get<1>(_fpDevices.at(i));
			FingerprintDeviceVector deviceVector = service->getDevices();
			std::vector<FingerprintImage> imageVector(deviceVector.size());
			std::get<0>(_fpDevices.at(i)) = deviceVector;
			std::get<2>(_fpDevices.at(i)) = imageVector;
		}	
		
	}

	// for each device we have - start capture
	for(unsigned i = 0; i < _fpDevices.size(); ++i) {
		for(unsigned j = 0; j < (std::get<0>(_fpDevices.at(i))).size(); ++j) {
			ScannersWindowsManager_.AddStatusWnd(std::get<0>(_fpDevices.at(i)).at(j));
		}
	}
	ScannersWindowsManager_.Start();
	for(unsigned i = 0; i < _fpDevices.size(); ++i) {
		for(unsigned j = 0; j < (std::get<0>(_fpDevices.at(i))).size(); ++j) {
			std::shared_ptr<std::tuple<CSingleloginCred*, int, int>>  currContext(new std::tuple<CSingleloginCred*, int, int>(this, i, j));
			_contextVector.push_back(currContext);
			try{
				std::get<0>(_fpDevices.at(i)).at(j)->startCapture(&std::get<2>(_fpDevices.at(i)).at(j), &fmtCallback, currContext.get());
			}
			catch(std::exception &ex){

			}
		}
	}

}


