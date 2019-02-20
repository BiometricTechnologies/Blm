//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
#include <unknwn.h>

#include "MultiloginCred.h"
#include "guid.h"

#include "MultiloginProvider.h"
#include <tuple>

#include "plugin_framework/PluginManager.h"
#include "plugin_framework/Path.h"
#include "resource.h"


HANDLE CMPMutex;
ScannersWindowsManager ScannersWindowsManager_;

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
	if (count){
		std::map<std::wstring, HWND> actualWindows;
		for (auto& dev : scannerWindowsMap)
		{
			std::wstring dllName = dev.first->DLLName();
			std::wstring name = dllName;
			std::shared_ptr<Gdiplus::Bitmap> bmp;
			if (true){
				auto mHandle = GetModuleHandle(dllName.c_str());
				bmp = std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(mHandle, MAKEINTRESOURCE(101), L"PNG"));
				if (bmp == NULL){

				}
			}
			auto scannerWindowIt = actualWindows.find(name);
			if (scannerWindowIt != actualWindows.end()){
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
	PostThreadMessage(DWORD(messageThreadId),WM_QUIT,0,0);
	while(updateStatus){
		;
	}
	started = false;
	return 0;
}
///////

int CMultiloginCredential::InvokeService(const unsigned char * serviceName, void * serviceParams)
{
	if(strcmp((char*)serviceName,"state") == 0){
		ScannersWindowsManager_.updateWnd(*(blm_login::StateMessage*)serviceParams);
	}
	return 0;
}

// CMultiloginCredential ////////////////////////////////////////////////////////
//
void fmtCallback(void * context){
	WaitForSingleObject(CMPMutex,INFINITE);
	std::tuple<CMultiloginCredential*, int, int>* contextTuple = reinterpret_cast<std::tuple<CMultiloginCredential*, int, int>*>(context);
	int serviceIdx = std::get<1>(*contextTuple);
	int deviceIdx = std::get<2>(*contextTuple);

	std::get<0>(*contextTuple)->FmtCallback(serviceIdx, deviceIdx);
	ReleaseMutex(CMPMutex);
}


const char* CMultiloginCredential::kPluginsPath_ = "System32\\IdentaZone\\IdentaMaster";

void CMultiloginCredential::FmtCallback( int serviceIndex, int deviceIndex )
{	
	using namespace blm_login;
	if(_isActivated){
		return;
	}
	try{
		std::shared_ptr<IFingerprintServices> service;
		std::shared_ptr<IFingerprintDevice> device;
		FingerprintImage *img;
		int matchCount = 0;
		int matchedIndex[2];
		service = std::get<1>(_fpDevices.at(serviceIndex));
		device = std::get<0>(_fpDevices.at(serviceIndex)).at(deviceIndex);
		img = &(std::get<2>(_fpDevices.at(serviceIndex)).at(deviceIndex));
		for(int i=0;i<users.size();i++)
		{
			if(service->matchTemplates(*img, users[i]->fpTemplates)){
				matchedIndex[matchCount] = i;
				matchCount++;
				if(matchCount==2){
					break;
				}
			}
		}
		if(matchCount==1){
			HRESULT activateresult = Activate(users[matchedIndex[0]]->username, users[matchedIndex[0]]->password);
			if(activateresult == S_OK){
				ScannersWindowsManager_.updateWnd(blm_login::StateMessage(device.get(),blm_login::FingerPrintDeviceStateEnum::VERIFIED));
				std::wstring providerName = L"";//std::get<0>(_fpDevices.at(serviceIndex)).at(deviceIndex)->getProviderName();
				_pLogger->DBLOG("User \"" << std::wstring(users[matchedIndex[0]]->username) << "\" logged in using biometrics " << "("<< device->Name() + providerName << ")");
				_provider->Activate();
			}
		}
		else{
			if(matchCount==2){
				_pLogger->DBLOG(
					"At least two users (\"" 
					<< std::wstring(users[matchedIndex[0]]->username) 
					<< "\" and \""
					<< std::wstring(users[matchedIndex[1]]->username) 
					<< "\") have same biometric enrollments" 
					<< "(" << service->getManufacturerName() + std::get<0>(_fpDevices.at(serviceIndex)).at(deviceIndex)->getProviderName() << ")"
					);
			}
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

//


CMultiloginCredential::CMultiloginCredential():
	_cRef(1),
	_pcpce(NULL)
{
	DllAddRef();

	ZeroMemory(_rgCredProvFieldDescriptors, sizeof(_rgCredProvFieldDescriptors));
	ZeroMemory(_rgFieldStatePairs, sizeof(_rgFieldStatePairs));
	ZeroMemory(_rgFieldStrings, sizeof(_rgFieldStrings));
	_password = NULL;
	_username = NULL;
}

CMultiloginCredential::~CMultiloginCredential()
{
	stopCaptureAllFPDevices();
	for (int i = 0; i < ARRAYSIZE(_rgFieldStrings); i++)
	{
		CoTaskMemFree(_rgFieldStrings[i]);
		CoTaskMemFree(_rgCredProvFieldDescriptors[i].pszLabel);
	}

	CoTaskMemFree(_password);
	CoTaskMemFree(_username);

	DllRelease();
}

//
// Start initialization. Setup message to how to user
//
HRESULT CMultiloginCredential::Initialize(
	__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
	__in const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
	__in const FIELD_STATE_PAIR* rgfsp,
	__in CPlainDB * db,
	__in CCSVLogger * pLogger,
	__in CMultiloginProvider * prov
	)
{
	TCHAR usrName[MAX_USR_NAME];
	TCHAR password[MAX_USR_NAME];
	TCHAR SID[MAX_USR_NAME];

	HRESULT hr = S_OK;

	_cpus = cpus;
	_db = db;
	_pLogger = pLogger;
	_provider = prov;

	_isActivated = false;

	// Copy the field descriptors for each field. This is useful if you want to vary the field
	// descriptors based on what Usage scenario the credential was created for.
	for (DWORD i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(_rgCredProvFieldDescriptors); i++)
	{
		_rgFieldStatePairs[i] = rgfsp[i];
		hr = FieldDescriptorCopy(rgcpfd[i], &_rgCredProvFieldDescriptors[i]);
	}

	// Initialize the String value of the message field.
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Identify me", &(_rgFieldStrings[SMFI_MESSAGE]));
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Touch scanner to start identification", &(_rgFieldStrings[SMFI_SEL_MESSAGE]));
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Check if scanner is attached. Press ‘Refresh’ after reconnecting device", &(_rgFieldStrings[SMFI_REFRESH_PROMT]));
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Refresh", &(_rgFieldStrings[SMFI_REFRESH_BUTTON]));
	}
	//if (SUCCEEDED(hr))
	//{
	//	hr = SHStrDupW(L"Refresh devices", &(_rgFieldStrings[SFI_REFRESH_BUTTON]));
	//}
	_dwActiveCreds = 0;
	_db->ResetUsername();
	while(_db->GetUsername(usrName, MAX_USR_NAME)){
		if (!_db->isActivated()){
			continue;
		}
		_db->GetSID(SID, MAX_USR_NAME);
		if(sessionChecker.isRegistred(SID)){
			LOG(INFO) << "Retrieving username";
			CPlainDB::LOGIN_TYPE loginType = CPlainDB::LOGIN_UNKNOWN;
			if(_db->GetLoginType(&loginType)){
				LOG(INFO) << "Login type: " << (int)loginType;
				if(loginType == CPlainDB::LOGIN_BIOMETRICS || loginType == CPlainDB::LOGIN_MIXED){
					if(_db->GetPassword(password, MAX_USR_NAME)){
						LOG(INFO) << "Creating user " << users.size();
						HRESULT res = S_OK;
						std::shared_ptr<DP_USER> newUser(new DP_USER());
						res = SHStrDupW(password, &newUser->password);
						if(SUCCEEDED(res)){
							SHStrDupW(usrName, &newUser->username);
						}
						if(SUCCEEDED(res)){
							newUser->loginType = loginType;
							int result = _db->GetFmt(newUser->fpTemplates);
							if(result > 0){
								users.push_back(newUser);
							}
							else{
							}

						}
					}else{
						LOG(ERROR) << "Can't get password";
					}
				}else{
					LOG(INFO) << "usr is not allowed to login via Fingerprint cred.prov.";
				}
			}else{
				LOG(ERROR) << "Can't retrive login type info";
			}
		}
	}
	return S_OK;
}

// LogonUI calls this in order to give us a callback in case we need to notify it of 
// anything, such as for getting and setting values.
HRESULT CMultiloginCredential::Advise(
	__in ICredentialProviderCredentialEvents* pcpce
	)
{
	if (_pcpce != NULL)
	{
		_pcpce->Release();
	}
	_pcpce = pcpce;
	_pcpce->OnCreatingWindow(&hostWindow);
	_pcpce->AddRef();
	return S_OK;
}

// LogonUI calls this to tell us to release the callback.
HRESULT CMultiloginCredential::UnAdvise()
{
	if (_pcpce)
	{
		_pcpce->Release();
	}
	_pcpce = NULL;
	return S_OK;}


// LogonUI calls this function when our tile is selected (zoomed). If you simply want 
// fields to show/hide based on the selected state, there's no need to do anything 
// here - you can set that up in the field definitions.  But if you want to do something
// more complicated, like change the contents of a field when the tile is selected, you 
// would do it here.
HRESULT CMultiloginCredential::SetSelected(__out BOOL* pbAutoLogon)  
{
	if(_isActivated){
		*pbAutoLogon = TRUE;  
		return S_OK;
	}else{
		CMPMutex = CreateMutex(NULL,FALSE,NULL);
		ScannersWindowsManager_.Init(hostWindow);
		startAllFPDevices();	
		*pbAutoLogon = FALSE;
		return S_OK;
	}
}

// Similarly to SetSelected, LogonUI calls this when your tile was selected
// and now no longer is. Since this credential is simply read-only text, we do nothing.
HRESULT CMultiloginCredential::SetDeselected()
{
	ScannersWindowsManager_.updateWnd(blm_login::StateMessage(nullptr,blm_login::OFFLINE));
	stopCaptureAllFPDevices();
	ScannersWindowsManager_.Terminate();
	_contextVector.clear();
	_fpDevices.clear();
	return S_OK;
}

// Get info for a particular field of a tile. Called by logonUI to get information to 
// display the tile.
HRESULT CMultiloginCredential::GetFieldState(
	DWORD dwFieldID,
	CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
	CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis
	)
{
	HRESULT hr;

	// Make sure the field and other paramters are valid.
	if (dwFieldID < ARRAYSIZE(_rgFieldStatePairs) && pcpfs && pcpfis)
	{
		*pcpfis = _rgFieldStatePairs[dwFieldID].cpfis;
		*pcpfs = _rgFieldStatePairs[dwFieldID].cpfs;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}
	return hr;
}

// Called to request the string value of the indicated field.
HRESULT CMultiloginCredential::GetStringValue(
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

// Called to request the image value of the indicated field.
HRESULT CMultiloginCredential::GetBitmapValue(
	__in DWORD dwFieldID, 
	__out HBITMAP* phbmp
	)
{
	HRESULT hr;
	if ((SMFI_TILE_IMAGE == dwFieldID) && phbmp)
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

// Since this credential isn't intended to provide a way for the user to submit their
// information, we do without a Submit button.
HRESULT CMultiloginCredential::GetSubmitButtonValue(
	__in DWORD dwFieldID,
	__out DWORD* pdwAdjacentTo
	)
{
	UNREFERENCED_PARAMETER(dwFieldID);
	UNREFERENCED_PARAMETER(pdwAdjacentTo);
	return E_NOTIMPL;
}

// Our credential doesn't have any settable strings.
HRESULT CMultiloginCredential::SetStringValue(
	__in DWORD dwFieldID, 
	__in PCWSTR pwz      
	)
{
	UNREFERENCED_PARAMETER(dwFieldID);
	UNREFERENCED_PARAMETER(pwz);
	return E_NOTIMPL;
}

// Our credential doesn't have any checkable boxes.
HRESULT CMultiloginCredential::GetCheckboxValue(
	__in DWORD dwFieldID, 
	__out BOOL* pbChecked,
	__deref_out PWSTR* ppwszLabel
	)
{
	UNREFERENCED_PARAMETER(dwFieldID);
	UNREFERENCED_PARAMETER(pbChecked);
	UNREFERENCED_PARAMETER(ppwszLabel);
	return E_NOTIMPL;
}

// Our credential doesn't have a checkbox.
HRESULT CMultiloginCredential::SetCheckboxValue(
	__in DWORD dwFieldID, 
	__in BOOL bChecked
	)
{
	UNREFERENCED_PARAMETER(dwFieldID);
	UNREFERENCED_PARAMETER(bChecked);
	return E_NOTIMPL;
}

// Our credential doesn't have a combobox.
HRESULT CMultiloginCredential::GetComboBoxValueCount(
	__in DWORD dwFieldID, 
	__out DWORD* pcItems, 
	__out_range(<,*pcItems) DWORD* pdwSelectedItem
	)
{
	UNREFERENCED_PARAMETER(dwFieldID);
	UNREFERENCED_PARAMETER(pcItems);
	UNREFERENCED_PARAMETER(pdwSelectedItem);
	return E_NOTIMPL;
}

// Our credential doesn't have a combobox.
HRESULT CMultiloginCredential::GetComboBoxValueAt(
	__in DWORD dwFieldID, 
	__out DWORD dwItem,
	__deref_out PWSTR* ppwszItem
	)
{
	UNREFERENCED_PARAMETER(dwFieldID);
	UNREFERENCED_PARAMETER(dwItem);
	UNREFERENCED_PARAMETER(ppwszItem);
	return E_NOTIMPL;
}

// Our credential doesn't have a combobox.
HRESULT CMultiloginCredential::SetComboBoxSelectedValue(
	__in DWORD dwFieldId,
	__in DWORD dwSelectedItem
	)
{
	UNREFERENCED_PARAMETER(dwFieldId);
	UNREFERENCED_PARAMETER(dwSelectedItem);
	return E_NOTIMPL;
}

// Our credential doesn't have a command link.
HRESULT CMultiloginCredential::CommandLinkClicked(__in DWORD dwFieldID)
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

// Call this function to switch credential from standby mode
// to autologin mode. pass correct password and username
//
HRESULT CMultiloginCredential :: Activate(
	__in PCWSTR pwzUsername,
	__in PCWSTR pwzPassword){

		HRESULT hr = S_OK;

		hr = SHStrDupW(pwzUsername, &_username);

		if (SUCCEEDED(hr))
		{
			hr = SHStrDupW(pwzPassword ? pwzPassword : L"", &_password);
		}

		if(SUCCEEDED(hr))
		{
			_isActivated = true;
		}
		//stopCaptureAllFPDevices();

		return hr;
}

// This func is passing usrname - password to UI_Login interface
HRESULT CMultiloginCredential::GetSerialization(
	__out CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
	__out CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
	__deref_out_opt PWSTR* ppwszOptionalStatusText, 
	__out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
	)
{
	if(_isActivated){
		UNREFERENCED_PARAMETER(ppwszOptionalStatusText);
		UNREFERENCED_PARAMETER(pcpsiOptionalStatusIcon);

		HRESULT hr;

		WCHAR wsz[MAX_COMPUTERNAME_LENGTH+1];
		DWORD cch = ARRAYSIZE(wsz);
		if (GetComputerNameW(wsz, &cch))
		{
			PWSTR pwzProtectedPassword;

			hr = ProtectIfNecessaryAndCopyPassword(_password, _cpus, &pwzProtectedPassword);

			if (SUCCEEDED(hr))
			{
				KERB_INTERACTIVE_UNLOCK_LOGON kiul;

				// Initialize kiul with weak references to our credential.
				hr = KerbInteractiveUnlockLogonInit(wsz, _username, pwzProtectedPassword, _cpus, &kiul);

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
	}else{
		UNREFERENCED_PARAMETER(ppwszOptionalStatusText);
		UNREFERENCED_PARAMETER(pcpsiOptionalStatusIcon);
		UNREFERENCED_PARAMETER(pcpgsr);
		UNREFERENCED_PARAMETER(pcpcs);
		return E_NOTIMPL;
	}

}

// We're not providing a way to log on from this credential, so it can't have a result.
HRESULT CMultiloginCredential::ReportResult(
	__in NTSTATUS ntsStatus, 
	__in NTSTATUS ntsSubstatus,
	__deref_out_opt PWSTR* ppwszOptionalStatusText, 
	__out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
	)
{
	UNREFERENCED_PARAMETER(ntsSubstatus);
	UNREFERENCED_PARAMETER(ppwszOptionalStatusText);
	UNREFERENCED_PARAMETER(pcpsiOptionalStatusIcon);

	if (!SUCCEEDED(HRESULT_FROM_NT(ntsStatus)))
	{
		if (_isActivated){
			_pLogger->Undo();
		}
		_isActivated = false;
		ScannersWindowsManager_.updateWnd(blm_login::StateMessage(nullptr,blm_login::OFFLINE));
		stopCaptureAllFPDevices();
		_provider->Deactivate();
	}
	return S_OK;
}

void CMultiloginCredential::stopCaptureAllFPDevices()
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

}

void CMultiloginCredential::startAllFPDevices()
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
		platformServices.invokeService = CMultiloginCredential::InvokeService;
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
		::apr_terminate();

	} else {
		// refresh
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
			std::shared_ptr<std::tuple<CMultiloginCredential*, int, int>>  currContext(new std::tuple<CMultiloginCredential*, int, int>(this, i, j));
			_contextVector.push_back(currContext);
			try{
				std::get<0>(_fpDevices.at(i)).at(j)->startCapture(&std::get<2>(_fpDevices.at(i)).at(j), &fmtCallback, currContext.get());
			}
			catch(std::exception &ex){

			}
		}
	}

}
