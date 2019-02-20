//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// CMultiloginProvider implements ICredentialProvider, which is the main
// interface that logonUI uses to decide which tiles to display.
// In this sample, we have decided to show two tiles, one for
// Administrator and one for Guest.  You will need to decide what
// tiles make sense for your situation.  Can you enumerate the
// users who will use your method to log on?  Or is it better
// to provide a tile where they can type in their username?
// Does the user need to interact with something other than the
// keyboard before you can recognize which user it is (such as insert 
// a smartcard)?  We call these "event driven" credential providers.  
// We suggest that such credential providers first provide one basic tile which
// tells the user what to do ("insert your smartcard").  Once the
// user performs the action, then you can callback into LogonUI to
// tell it that you have new tiles, and include a tile that is specific
// to the user that the user can then interact with if necessary.

#include <credentialprovider.h>
#include "MultiloginProvider.h"
#include "guid.h"

#include <windows.h>
#include <Lmcons.h>

#define DEBUG_LOG 1
#define LOG_ROTATE_COUNT 2
#define LOG_ROTATE_SIZE 10000

void loggingInit(CCSVLogger * logger){
	TCHAR sysPath[MAX_PATH];
	TCHAR path[MAX_PATH];
	GetSystemWindowsDirectory(sysPath, MAX_PATH);
	FILE * fp = NULL;

	// LOCK file
	wcscpy_s(path, MAX_PATH, sysPath);
	wcscat_s(path, MAX_PATH, L"\\System32\\IdentaZone\\multilogin.log");
	logger->Init(path, LOG_ROTATE_COUNT, LOG_ROTATE_SIZE);
}
// CMultiloginProvider ////////////////////////////////////////////////////////

CMultiloginProvider::CMultiloginProvider():
	_cRef(1),
	_pcpe(NULL),
	_pkiulSetSerialization(NULL),
	_bAutoSubmitSetSerializationCred(false),
	_dwSetSerializationCred(CREDENTIAL_PROVIDER_NO_DEFAULT)
{
//	Sleep(10000);
	DllAddRef();
	google::InitGoogleLogging("DPLogin");
	google::SetLogDestination(google::GLOG_INFO, "C:\\log\\"); 
	FLAGS_logtostderr = false;
#ifdef DEBUG_LOG
	FLAGS_stderrthreshold=google::GLOG_INFO;
#else
	FLAGS_stderrthreshold=google::GLOG_ERROR;
#endif
	loggingInit(&_logger);

	LOG(INFO) << "Provider started";

	_unlocked = FALSE;
	_pMessageCredential = NULL;
	_credsEnumerated = false;
}

CMultiloginProvider::~CMultiloginProvider()
{
	LOG(INFO) << "Provider stopped";
	if(_pMessageCredential != NULL)
	{
		_pMessageCredential->Release();
	}

	DllRelease();
}

void CMultiloginProvider::_CleanupSetSerialization()
{
	if (_pkiulSetSerialization)
	{
		KERB_INTERACTIVE_LOGON* pkil = &_pkiulSetSerialization->Logon;
		SecureZeroMemory(_pkiulSetSerialization,
			sizeof(*_pkiulSetSerialization) +
			pkil->LogonDomainName.MaximumLength +
			pkil->UserName.MaximumLength +
			pkil->Password.MaximumLength);
		HeapFree(GetProcessHeap(),0, _pkiulSetSerialization);
	}
}

void CMultiloginProvider::Activate()
{
	_unlocked = TRUE;
	if(_pcpe != NULL){
		_pcpe->CredentialsChanged(_upAdviseContext);
	}
}

void CMultiloginProvider::Deactivate()
{
	_unlocked = FALSE;
}


// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.  
//
// This sample only handles the logon and unlock scenarios as those are the most common.
HRESULT CMultiloginProvider::SetUsageScenario(
	__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
	__in DWORD dwFlags
	)
{
	UNREFERENCED_PARAMETER(dwFlags);
	HRESULT hr;


	LOG(INFO) << "Set usage scenario: " << cpus;

	// Decide which scenarios to support here. Returning E_NOTIMPL simply tells the caller
	// that we're not designed for that scenario.
	switch (cpus)
	{
	case CPUS_LOGON:
		// A more advanced credprov might only enumerate tiles for the user whose owns the locked
		// session, since those are the only creds that wil work
		if (!_credsEnumerated)
		{
			LOG(INFO) << "Creds not enumerated";
			db.Init();
			_cpus = cpus;
			hr = this->_EnumerateCredentials();
			_credsEnumerated = true;
		}
		else
		{
			LOG(INFO) << "Creds already enumerated";
			hr = S_OK;
		}
		break;

	case CPUS_UNLOCK_WORKSTATION:       
	case CPUS_CREDUI:
	case CPUS_CHANGE_PASSWORD:
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
// SetSerialization takes the serialization and uses it to create a credential.
//
// SetSerialization is called for two main scenarios.  The first scenario is in the credui case
// where it is prepopulating a tile with credentials that the user chose to store in the OS.
// The second situation is in a remote logon case where the remote client may wish to 
// prepopulate a tile with a username, or in some cases, completely populate the tile and
// use it to logon without showing any UI.
//
// Since this sample doesn't support CPUS_CREDUI, we have not implemented the credui specific
// pieces of this function.  For information on that, please see the credUI sample.
HRESULT CMultiloginProvider::SetSerialization(
	__in const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs
	)
{
	UNREFERENCED_PARAMETER(pcpcs);
	return E_NOTIMPL;
	/*
	HRESULT hr = E_INVALIDARG;

	if ((DLL_GUID == pcpcs->clsidCredentialProvider))
	{
	// Get the current AuthenticationPackageID that we are supporting
	ULONG ulAuthPackage;
	hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);

	if (SUCCEEDED(hr))
	{
	if ((ulAuthPackage == pcpcs->ulAuthenticationPackage) &&
	(0 < pcpcs->cbSerialization && pcpcs->rgbSerialization))
	{
	KERB_INTERACTIVE_UNLOCK_LOGON* pkil = (KERB_INTERACTIVE_UNLOCK_LOGON*) pcpcs->rgbSerialization;
	if (KerbInteractiveLogon == pkil->Logon.MessageType)
	{
	BYTE* rgbSerialization;
	rgbSerialization = (BYTE*)HeapAlloc(GetProcessHeap(), 0, pcpcs->cbSerialization);
	hr = rgbSerialization ? S_OK : E_OUTOFMEMORY;

	if (SUCCEEDED(hr))
	{
	CopyMemory(rgbSerialization, pcpcs->rgbSerialization, pcpcs->cbSerialization);
	KerbInteractiveUnlockLogonUnpackInPlace((KERB_INTERACTIVE_UNLOCK_LOGON*)rgbSerialization, pcpcs->cbSerialization);

	if (_pkiulSetSerialization)
	{
	HeapFree(GetProcessHeap(), 0, _pkiulSetSerialization);

	// For this sample, we know that _dwSetSerializationCred is always in the last slot
	if (_dwSetSerializationCred != CREDENTIAL_PROVIDER_NO_DEFAULT)
	{
	_pMessageCredential->Release();
	_pMessageCredential = NULL;
	_dwSetSerializationCred = CREDENTIAL_PROVIDER_NO_DEFAULT;
	}
	}
	_pkiulSetSerialization = (KERB_INTERACTIVE_UNLOCK_LOGON*)rgbSerialization;
	hr = S_OK;
	}
	}
	}
	}
	else
	{
	hr = E_INVALIDARG;
	}
	}
	return hr;*/
}



// Called by LogonUI to give you a callback.  Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated
HRESULT CMultiloginProvider::Advise(
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
	return  S_OK;
}

// Called by LogonUI when the ICredentialProviderEvents callback is no longer valid.
HRESULT CMultiloginProvider::UnAdvise()
{
	if (_pcpe != NULL)
	{
		_pcpe->Release();
		_pcpe = NULL;
	}
	LOG(INFO) << "Unadvise";
	return  S_OK;
}

// Called by LogonUI to determine the number of fields in your tiles.  This
// does mean that all your tiles must have the same number of fields.
// This number must include both visible and invisible fields. If you want a tile
// to have different fields from the other tiles you enumerate for a given usage
// scenario you must include them all in this count and then hide/show them as desired 
// using the field descriptors.
HRESULT CMultiloginProvider::GetFieldDescriptorCount(
	__out DWORD* pdwCount
	)
{

	*pdwCount = SMFI_NUM_FIELDS;

	return S_OK;
}

// Gets the field descriptor for a particular field
HRESULT CMultiloginProvider::GetFieldDescriptorAt(
	__in DWORD dwIndex, 
	__deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
	)
{    
	HRESULT hr;


	// Verify dwIndex is a valid field.
	if ((dwIndex < SMFI_NUM_FIELDS) && ppcpfd)
	{
		hr = FieldDescriptorCoAllocCopy(s_rgMessageCredProvFieldDescriptors[dwIndex], ppcpfd);
	}
	else
	{ 
		hr = E_INVALIDARG;
	}

	return hr;
}

// Sets pdwCount to the number of tiles that we wish to show at this time.
// Sets pdwDefault to the index of the tile which should be used as the default.
//
// The default tile is the tile which will be shown in the zoomed view by default. If 
// more than one provider specifies a default tile the behavior is the last used cred
// prov gets to specify the default tile to be displayed
//
// If *pbAutoLogonWithDefault is TRUE, LogonUI will immediately call GetSerialization
// on the credential you've specified as the default and will submit that credential
// for authentication without showing any further UI.
HRESULT CMultiloginProvider::GetCredentialCount(
	__out DWORD* pdwCount,
	__out_range(<,*pdwCount) DWORD* pdwDefault,
	__out BOOL* pbAutoLogonWithDefault
	)
{
	HRESULT hr = S_OK;
	/*
	if (_pkiulSetSerialization && _dwSetSerializationCred == CREDENTIAL_PROVIDER_NO_DEFAULT)
	{
	//haven't yet made a cred from the SetSerialization info
	_EnumerateSetSerialization();  //ignore failure, we can still produce our other tiles
	}
	*/

	*pdwCount = 1;
	if(!_unlocked){
		LOG(INFO) << "Get cred count: Autologon off";
		// no tiles, clear out out params
		*pdwDefault = CREDENTIAL_PROVIDER_NO_DEFAULT;
		*pbAutoLogonWithDefault = FALSE;
		return hr;
	}

	LOG(INFO) << "Get cred count: Autologon on";
	*pdwDefault = 0;
	*pbAutoLogonWithDefault = TRUE;
	return hr;

}

// Returns the credential at the index specified by dwIndex. This function is called by logonUI to enumerate
// the tiles.
HRESULT CMultiloginProvider::GetCredentialAt(
	__in DWORD dwIndex, 
	__deref_out ICredentialProviderCredential** ppcpc
	)
{
	HRESULT hr;

	if(!ppcpc){
		hr = E_INVALIDARG;
	}else{

		if(dwIndex < SMFI_NUM_FIELDS){
			LOG(INFO) << "Get credetial at: " << dwIndex;
			hr = _pMessageCredential->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
		}else{
			LOG(INFO) << "Get credential at: bad index";
		}

	}
	return hr;
}

// Sets up all the credentials for this provider. Since we always show the same tiles, 
// we just set it up once.
HRESULT CMultiloginProvider::_EnumerateCredentials()
{
	HRESULT hr;
	_pMessageCredential = new CMultiloginCredential();
	LOG(INFO) << "Enumerating message credential";
	hr = _pMessageCredential->Initialize(_cpus, s_rgMessageCredProvFieldDescriptors, s_rgMessageFieldStatePairs, &db, &_logger, this);
	return hr;
}

// Boilerplate code to create our provider.
HRESULT CSample_CreateInstance(__in REFIID riid, __deref_out void** ppv)
{
	HRESULT hr;

	CMultiloginProvider* pProvider = new CMultiloginProvider();

	if (pProvider)
	{
		hr = pProvider->QueryInterface(riid, ppv);
		pProvider->Release();
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

// This enumerates a tile for the info in _pkiulSetSerialization.  See the SetSerialization function comment for
// more information.
HRESULT CMultiloginProvider::_EnumerateSetSerialization()
{
	KERB_INTERACTIVE_LOGON* pkil = &_pkiulSetSerialization->Logon;

	_bAutoSubmitSetSerializationCred = false;

	// Since this provider only enumerates local users (not domain users) we are ignoring the domain passed in.
	// However, please note that if you receive a serialized cred of just a domain name, that domain name is meant 
	// to be the default domain for the tiles (or for the empty tile if you have one).  Also, depending on your scenario,
	// the presence of a domain other than what you're expecting might be a clue that you shouldn't handle
	// the SetSerialization.  For example, in this sample, we could choose to not accept a serialization for a cred
	// that had something other than the local machine name as the domain.

	// Use a "long" (MAX_PATH is arbitrary) buffer because it's hard to predict what will be
	// in the incoming values.  A DNS-format domain name, for instance, can be longer than DNLEN.
	WCHAR wszUsername[MAX_PATH] = {0};
	WCHAR wszPassword[MAX_PATH] = {0};

	// since this sample assumes local users, we'll ignore domain.  If you wanted to handle the domain
	// case, you'd have to update CSingleloginCred::Initialize to take a domain.
	HRESULT hr = StringCbCopyNW(wszUsername, sizeof(wszUsername), pkil->UserName.Buffer, pkil->UserName.Length);

	if (SUCCEEDED(hr))
	{
		hr = StringCbCopyNW(wszPassword, sizeof(wszPassword), pkil->Password.Buffer, pkil->Password.Length);

		if (SUCCEEDED(hr))
		{
			//CSingleloginCred* pCred = new CSingleloginCred();
			CMultiloginCredential* pCred = new CMultiloginCredential();


			if (pCred)
			{
				hr = pCred->Initialize(_cpus, s_rgCredProvFieldDescriptors, s_rgFieldStatePairs,&db,&_logger,this);

				pCred->Activate(wszUsername, wszPassword);
				_dwSetSerializationCred = 0;
			}
			else
			{
				hr = E_OUTOFMEMORY;
			}

			// If we were passed all the info we need (in this case username & password), we're going to automatically submit this credential.
			if (SUCCEEDED(hr) && (0 < wcslen(wszPassword)))
			{
				_bAutoSubmitSetSerializationCred = true;
			}
		}
	}


	return hr;
}

