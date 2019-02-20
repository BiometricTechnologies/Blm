//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// CSingleloginProvider implements ICredentialProvider, which is the main
// interface that logonUI uses to decide which tiles to display.
// In this sample, we will display one tile that uses each of the nine
// available UI controls.

#include <credentialprovider.h>
#include "SingleloginProvider.h"
#include "SingleloginCred.h"
#include "SessionChecker.h"
#include "guid.h"
#include <wincred.h>
#include <string.h>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

namespace logging = boost::log;
namespace src = logging::sources;
#define LOG_ROTATE_COUNT 2
#define LOG_ROTATE_SIZE 10000

void loggingInit(CCSVLogger * logger, LPWSTR logfile){
	TCHAR sysPath[MAX_PATH];
	TCHAR path[MAX_PATH];
	GetSystemWindowsDirectory(sysPath, MAX_PATH);
	FILE * fp = NULL;
	// LOCK file
	wcscpy_s(path, MAX_PATH, sysPath);
	wcscat_s(path, MAX_PATH, L"\\System32\\IdentaZone\\");
	wcscat_s(path, MAX_PATH, logfile);
	logger->Init(path,LOG_ROTATE_COUNT,LOG_ROTATE_SIZE);
}
// CSingleloginProvider ////////////////////////////////////////////////////////

CSingleloginProvider::CSingleloginProvider():
	_cRef(1)
{
	//Sleep(10000);
//	MessageBox(NULL,L"kkk",L"lll",MB_ICONERROR);
	loggingInit(&_logger,L"singlelogin.log");
	DllAddRef();

	_unlocked = FALSE;
}

CSingleloginProvider::~CSingleloginProvider()
{
	_dbglogger.DBLOG("entered ~CSingleloginProvider()");
	for each (CSingleloginCred* credential in _creds)
	{
		if (credential != NULL)
		{
			_dbglogger.DBLOG("releasing credo");
			credential->Release();
		}
	}
	_dbglogger.DBLOG("left ~CSingleloginProvider()");
	_dbglogger.DBLOG("                            ");
	DllRelease();
}

HRESULT CSingleloginProvider::_EnumerateOne(TCHAR * name, TCHAR * password, CPlainDB::LOGIN_TYPE loginType)
{
	HRESULT hr = S_OK;
	CSingleloginCred* pCredential = NULL;
	try{
		pCredential = new CSingleloginCred(&_logger,&_dbglogger);
		hr = pCredential->Initialize(_cpus, s_rgCredProvFieldDescriptors, s_rgFieldStatePairs,
			name, password, loginType, this);
		if (FAILED(hr))
		{
			pCredential->Release();
			pCredential = NULL;
		}else{
			_creds.push_back(pCredential);
		}
	}
	catch(std::exception &e)
	{
		hr = E_OUTOFMEMORY;
	}
	return hr;
}

HRESULT CSingleloginProvider::_EnumerateOne(TCHAR * name, TCHAR * fullName, TCHAR * password, CPlainDB::LOGIN_TYPE loginType)
{
	HRESULT hr = S_OK;
	CSingleloginCred* pCredential = NULL;
	try{
		pCredential = new CSingleloginCred(&_logger,&_dbglogger);
		hr = pCredential->Initialize(_cpus, s_rgCredProvFieldDescriptors, s_rgFieldStatePairs,
			name, fullName, password, loginType, this);
		if (FAILED(hr))
		{
			pCredential->Release();
			pCredential = NULL;
		}else{
			_creds.push_back(pCredential);
		}
	}
	catch(std::exception &e)
	{
		hr = E_OUTOFMEMORY;
	}
	return hr;
}

void CSingleloginProvider::Activate()
{
	_unlocked = TRUE;
	if(_pcpe != NULL){
		_pcpe->CredentialsChanged(_upAdviseContext);
	}
}
void CSingleloginProvider::updateTiles()
{
	if(_pcpe != NULL){
		_pcpe->CredentialsChanged(_upAdviseContext);
	}
}

HRESULT CSingleloginProvider::EnumerateUnlock()
{
	if(_creds.size()>0){
		return S_OK;
	}
	/////////
	logging::add_file_log("unlock.log");
	src::logger lg;
	BOOST_LOG(lg)<<"unlock initialization";
	/////////
	HRESULT hr = S_OK;
	CSingleloginCred* pCredential = NULL;	
	
	db.Init();
	TCHAR usrName[MAX_USR_NAME];
	TCHAR password[MAX_USR_NAME];
	TCHAR fullName[MAX_USR_NAME] = L"";
	db.ResetUsername();

	sessionChecker.Init();
	
	// find last logged user in database and read all the fingerprint templates from database inside user structure
	while(db.GetUsername(usrName, MAX_USR_NAME)){
		std::wstring uname(usrName);
		//BOOST_LOG(lg)<<uname;
		CPlainDB::LOGIN_TYPE loginType = CPlainDB::LOGIN_UNKNOWN;
		if(sessionChecker.isLastLogged(usrName)){
			//BOOST_LOG(lg)<<"is last logged in";
			if(db.GetLoginType(&loginType)){
				if(db.GetPassword(password, MAX_USR_NAME)){
					if(db.GetFullName(fullName, MAX_USR_NAME)){
						if(_EnumerateOne(usrName,fullName,password,loginType) == S_OK){
							DP_USER * pUser = _creds[0]->GetUser();
							//std::shared_ptr<blm_login::FingerPrintTemplate> fpTemplate(new blm_login::FingerPrintTemplate);
							int result = db.GetFmt(pUser->fpTemplates);
							// TODO: refactor this block (an.skornyakov@gmail.com)
							if(result > 0){
							}else{		
								pUser->fpTemplates.clear();
								LOG(ERROR) << "Can't get FMT, error code: " << result;
							}							
							_creds[0]->SetPromt(loginType, false, true);
							//BOOST_LOG(lg)<<"enumerated with fullname";
						}
					}else{
						if(_EnumerateOne(usrName,password,loginType) == S_OK){
							DP_USER * pUser = _creds[0]->GetUser();
							//std::shared_ptr<blm_login::FingerPrintTemplate> fpTemplate(new blm_login::FingerPrintTemplate);
							int result = db.GetFmt(pUser->fpTemplates);
							// TODO: refactor this block (an.skornyakov@gmail.com)
							if(result > 0){
								//pUser->fpTemplates.push_back(fpTemplate);
							}else{
								pUser->fpTemplates.clear();
								LOG(ERROR) << "Can't get FMT, error code: " << result;
							}							
							_creds[0]->SetPromt(loginType, false, true);
							//BOOST_LOG(lg)<<"enumerated without fullname";
						}
					}
				}
			}
		}
	}
	return hr;
}

HRESULT CSingleloginProvider::EnumerateLogon()
{

	if(_creds.size()>0){
		return S_OK;
	}
	HRESULT hr = S_OK;
	CSingleloginCred* pCredential = NULL;
	db.Init();
	TCHAR usrName[MAX_USR_NAME];
	TCHAR password[MAX_USR_NAME];
	TCHAR fullName[MAX_USR_NAME] = L"";
	TCHAR SID[MAX_USR_NAME];
	db.ResetUsername();

	sessionChecker.Init();
	_logger.Log((L"Enumerate; current count="+std::to_wstring(_creds.size())).c_str());
	while(db.GetUsername(usrName, MAX_USR_NAME)){
		if (!db.isActivated()){
			continue;
		}
		CPlainDB::LOGIN_TYPE loginType = CPlainDB::LOGIN_UNKNOWN;
		db.GetSID(SID, MAX_USR_NAME);
		if(sessionChecker.isRegistred(SID)){
			if(db.GetLoginType(&loginType)){
				if(db.GetPassword(password, MAX_USR_NAME)){
					if(db.GetFullName(fullName, MAX_USR_NAME)){
						if(_EnumerateOne(usrName,fullName,password,loginType) == S_OK){
							DP_USER * pUser = _creds.back()->GetUser();
							int result = db.GetFmt(pUser->fpTemplates);
							//std::shared_ptr<blm_login::FingerPrintTemplate> fpTemplate(new blm_login::FingerPrintTemplate);
							if(result > 0){
								//pUser->fpTemplates.push_back(fpTemplates);
							}else{
								pUser->fpTemplates.clear();
								LOG(ERROR) << "Can't get FMT, error code: " << result;
							}	
							_creds.back()->SetPromt(loginType, sessionChecker.isLogged(usrName), false);
						}
					}else{
						if(_EnumerateOne(usrName,password,loginType) == S_OK){
							DP_USER * pUser = _creds.back()->GetUser();
							//std::shared_ptr<blm_login::FingerPrintTemplate> fpTemplate(new blm_login::FingerPrintTemplate);
							int result = db.GetFmt(pUser->fpTemplates);
							//std::shared_ptr<blm_login::FingerPrintTemplate> fpTemplate(new blm_login::FingerPrintTemplate);

							if(result > 0){							
								//pUser->fpTemplates.push_back(fpTemplates);
							}else{	
								pUser->fpTemplates.clear();
								LOG(ERROR) << "Can't get FMT, error code: " << result;
							}	
							_creds.back()->SetPromt(loginType, sessionChecker.isLogged(usrName), false);
						}
					}
				}
			}
		}
	}
	return hr;
}

// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.
HRESULT CSingleloginProvider::SetUsageScenario(
	__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
	__in DWORD dwFlags
	)
{
	//std::wstring Flags;	
	
	//if(dwFlags&	CREDUIWIN_GENERIC)				Flags+=L"CREDUIWIN_GENERIC                ";
	//if(dwFlags&	CREDUIWIN_CHECKBOX)				Flags+=L"CREDUIWIN_CHECKBOX               ";
	//if(dwFlags&	CREDUIWIN_AUTHPACKAGE_ONLY)		Flags+=L"CREDUIWIN_AUTHPACKAGE_ONLY       ";
	//if(dwFlags&	CREDUIWIN_IN_CRED_ONLY)			Flags+=L"CREDUIWIN_IN_CRED_ONLY           ";
	//if(dwFlags&CREDUIWIN_ENUMERATE_ADMINS)		Flags+=L"CREDUIWIN_ENUMERATE_ADMINS       ";
	//if(dwFlags&CREDUIWIN_ENUMERATE_CURRENT_USER)Flags+=L"CREDUIWIN_ENUMERATE_CURRENT_USER ";
	//if(dwFlags&CREDUIWIN_SECURE_PROMPT)			Flags+=L"CREDUIWIN_SECURE_PROMPT          ";
	//if(dwFlags&CREDUIWIN_PREPROMPTING)			Flags+=L"CREDUIWIN_PREPROMPTING           ";
	//if(dwFlags&CREDUIWIN_PACK_32_WOW)			Flags+=L"CREDUIWIN_PACK_32_WOW            ";
	//_logger.Log(L"SetUsageScenario "+Flags+L" "+std::to_wstring(dwFlags));
	UNREFERENCED_PARAMETER(dwFlags);
	HRESULT hr = S_OK;
	// Decide which scenarios to support here. Returning E_NOTIMPL simply tells the caller
	// that we're not designed for that scenario.
	switch (cpus)
	{
	case CPUS_LOGON:
		_cpus = cpus;
		EnumerateLogon();
		break;
	case CPUS_UNLOCK_WORKSTATION:       
		_cpus = cpus;

		// Create and initialize our credential.
		// A more advanced credprov might only enumerate tiles for the user whose owns the locked
		// session, since those are the only creds that wil work
		EnumerateUnlock();
		break;
	case CPUS_CHANGE_PASSWORD:
		hr = E_NOTIMPL;
		break;
	case CPUS_CREDUI:
		hr = E_NOTIMPL;
		break;
	default:
		hr = E_INVALIDARG;
		break;
	}

	return hr;
}

// SetSerialization takes the kind of buffer that you would normally return to LogonUI for
// an authentication attempt.  It's the opposite of ICredentialProviderCredential::GetSerialization.
// GetSerialization is implement by a credential and serializes that credential.  Instead,
// SetSerialization takes the serialization and uses it to create a tile.
//
// SetSerialization is called for two main scenarios.  The first scenario is in the credui case
// where it is prepopulating a tile with credentials that the user chose to store in the OS.
// The second situation is in a remote logon case where the remote client may wish to 
// prepopulate a tile with a username, or in some cases, completely populate the tile and
// use it to logon without showing any UI.
//
// If you wish to see an example of SetSerialization, please see either the SampleCredentialProvider
// sample or the SampleCredUICredentialProvider sample.  [The logonUI team says, "The original sample that
// this was built on top of didn't have SetSerialization.  And when we decided SetSerialization was
// important enough to have in the sample, it ended up being a non-trivial amount of work to integrate
// it into the main sample.  We felt it was more important to get these samples out to you quickly than to
// hold them in order to do the work to integrate the SetSerialization changes from SampleCredentialProvider 
// into this sample.]
HRESULT CSingleloginProvider::SetSerialization(
	__in const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs
	)
{
	UNREFERENCED_PARAMETER(pcpcs);
	return E_NOTIMPL;
}

// Called by LogonUI to give you a callback.  Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated.
HRESULT CSingleloginProvider::Advise(
	__in ICredentialProviderEvents* pcpe,
	__in UINT_PTR upAdviseContext
	)
{
	if (_pcpe != NULL)
	{
		_pcpe->Release();
	}
	_pcpe = pcpe;
	_pcpe->AddRef();
	_upAdviseContext = upAdviseContext;
	LOG(INFO) << "Advise";
	return S_OK;
}

// Called by LogonUI when the ICredentialProviderEvents callback is no longer valid.
HRESULT CSingleloginProvider::UnAdvise()
{
	if(!_creds.empty() && (_cpus == CPUS_UNLOCK_WORKSTATION)){
		_creds[0]->stopCaptureAllFPDevices();
	}
	if (_pcpe != NULL)
	{
		_pcpe->Release();
		_pcpe = NULL;
	}
	_unlocked = FALSE;
	LOG(INFO) << "Unadvise";
	return S_OK;
}

// Called by LogonUI to determine the number of fields in your tiles.  This
// does mean that all your tiles must have the same number of fields.
// This number must include both visible and invisible fields. If you want a tile
// to have different fields from the other tiles you enumerate for a given usage
// scenario you must include them all in this count and then hide/show them as desired 
// using the field descriptors.
HRESULT CSingleloginProvider::GetFieldDescriptorCount(
	__out DWORD* pdwCount
	)
{
	*pdwCount = SFI_NUM_FIELDS;
	return S_OK;
}

// Gets the field descriptor for a particular field.
HRESULT CSingleloginProvider::GetFieldDescriptorAt(
	__in DWORD dwIndex, 
	__deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
	)
{    
	HRESULT hr;

	// Verify dwIndex is a valid field.
	if ((dwIndex < SFI_NUM_FIELDS) && ppcpfd)
	{
		hr = FieldDescriptorCoAllocCopy(s_rgCredProvFieldDescriptors[dwIndex], ppcpfd);
	}
	else
	{ 
		hr = E_INVALIDARG;
	}

	return hr;
}

// Sets pdwCount to the number of tiles that we wish to show at this time.
// Sets pdwDefault to the index of the tile which should be used as the default.
// The default tile is the tile which will be shown in the zoomed view by default. If 
// more than one provider specifies a default the last used cred prov gets to pick 
// the default. If *pbAutoLogonWithDefault is TRUE, LogonUI will immediately call 
// GetSerialization on the credential you've specified as the default and will submit 
// that credential for authentication without showing any further UI.
HRESULT CSingleloginProvider::GetCredentialCount(
	__out DWORD* pdwCount,
	__out_range(<,*pdwCount) DWORD* pdwDefault,
	__out BOOL* pbAutoLogonWithDefault
	)
{
	HRESULT hr = S_OK;
	*pdwCount = _creds.size();
	if(!_unlocked){
		if(_cpus!=CPUS_UNLOCK_WORKSTATION){
			*pdwDefault = CREDENTIAL_PROVIDER_NO_DEFAULT;
		}
		else{
			*pdwDefault = 0;
		}
		*pbAutoLogonWithDefault = FALSE;
		return S_OK;
	}
	LOG(INFO) << "Get cred count: Autologon on";
	*pdwDefault = 0;
	//for (DWORD i = 0; i < *pdwCount; i++){
	//	if(_creds[i]->isActive()){
	//		*pdwDefault = i;
	//		break;
	//	}
	//}
	*pdwCount = 1;
	*pbAutoLogonWithDefault = TRUE;
	return hr;

}

// Returns the credential at the index specified by dwIndex. This function is called by logonUI to enumerate
// the tiles.
HRESULT CSingleloginProvider::GetCredentialAt(
	__in DWORD dwIndex, 
	__deref_out ICredentialProviderCredential** ppcpc
	)
{
	HRESULT hr;
	if(_unlocked){
		for (DWORD i = 0; i < _creds.size(); i++){
			if(_creds[i]->isActive()){
				dwIndex = i;
				break;
			}
		}
	}
	if((dwIndex < _creds.size()) && ppcpc)
	{
		hr = _creds[dwIndex]->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}


// Boilerplate code to create our provider.
HRESULT CSample_CreateInstance(__in REFIID riid, __deref_out void** ppv)
{
	HRESULT hr;
	try{
		CSingleloginProvider* pProvider = new CSingleloginProvider();
		hr = pProvider->QueryInterface(riid, ppv);
		pProvider->Release();
	}
	catch(std::exception &e)
	{
		hr = E_OUTOFMEMORY;
	}
	return hr;
}
