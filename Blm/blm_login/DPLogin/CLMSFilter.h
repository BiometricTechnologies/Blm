#pragma once 
#include "credentialprovider.h" 
#include "guid.h"
#include "dll.h" 
#include "helpers.h"

//This class implements ICredentialProviderFilter, which is responsible for 
//filtering out other credential providers. 
//The LMS Credential Provider uses this to mask out the default Windows 
//provider. 
class CLMSFilter : public ICredentialProviderFilter 
{ 
public: 
	//This section contains some COM boilerplate code 

	// IUnknown 
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
			QITABENT(CLMSFilter, ICredentialProviderFilter), // IID_ICredentialProvider
			{0},
		};
		return QISearch(this, qit, riid, ppv);
	}


public: 
	friend HRESULT CLMSFilter_CreateInstance(REFIID riid, __deref_out void** 
		ppv); 

	//Implementation of ICredentialProviderFilter 
	IFACEMETHODIMP Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags, GUID* rgclsidProviders, BOOL* rgbAllow, DWORD cProviders); 
	IFACEMETHODIMP UpdateRemoteCredential(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsIn, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsOut); 

protected: 
	CLMSFilter(); 
	__override ~CLMSFilter(); 

private: 
	LONG _cRef; 
}; 