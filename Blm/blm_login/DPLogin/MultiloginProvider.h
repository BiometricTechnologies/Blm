
#include <credentialprovider.h>
#include <windows.h>
#include <strsafe.h>

#include "MultiloginCred.h"

#include "PlainDB.h"
#include "CSVLogger.h"

#include "helpers.h"

#define MAX_DWORD   0xffffffff        // maximum DWORD

class CMultiloginProvider :
	public ICredentialProvider
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
			QITABENT(CMultiloginProvider, ICredentialProvider), // IID_ICredentialProvider
			{0},
		};
		return QISearch(this, qit, riid, ppv);
	}

public:

	IFACEMETHODIMP SetUsageScenario(__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, __in DWORD dwFlags);
	IFACEMETHODIMP SetSerialization(__in const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);

	IFACEMETHODIMP Advise(__in ICredentialProviderEvents* pcpe, __in UINT_PTR upAdviseContext);
	IFACEMETHODIMP UnAdvise();

	IFACEMETHODIMP GetFieldDescriptorCount(__out DWORD* pdwCount);
	IFACEMETHODIMP GetFieldDescriptorAt(__in DWORD dwIndex,  __deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);

	IFACEMETHODIMP GetCredentialCount(__out DWORD* pdwCount,
		__out_range(<,*pdwCount) DWORD* pdwDefault,
		__out BOOL* pbAutoLogonWithDefault);
	IFACEMETHODIMP GetCredentialAt(__in DWORD dwIndex, 
		__deref_out ICredentialProviderCredential** ppcpc);


	void Activate();
	void Deactivate();
	friend HRESULT CSample_CreateInstance(__in REFIID riid, __deref_out void** ppv);
	
protected:
	CMultiloginProvider();
	__override ~CMultiloginProvider();

private:

	CPlainDB db;

	HRESULT _EnumerateOneCredential(__in DWORD dwCredientialIndex,
		__in PCWSTR pwzUsername,
		__in PCWSTR pwzPassword);
	HRESULT _EnumerateSetSerialization();

	// Create/free enumerated credentials.
	HRESULT _EnumerateCredentials();
	void _ReleaseEnumeratedCredentials();
	void _CleanupSetSerialization();

private:
	CCSVLogger _logger;
	LONG              _cRef;
	CMultiloginCredential *_pMessageCredential;
	// this Provider.
	KERB_INTERACTIVE_UNLOCK_LOGON*          _pkiulSetSerialization;
	DWORD                                   _dwSetSerializationCred; //index into rgpCredentials for the SetSerializationCred
	bool                                    _bAutoSubmitSetSerializationCred;
	CREDENTIAL_PROVIDER_USAGE_SCENARIO      _cpus;
	ICredentialProviderEvents *				_pcpe;
	UINT_PTR								_upAdviseContext;
	BOOL									_unlocked;
	bool									_credsEnumerated;

};

