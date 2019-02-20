//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include <credentialprovider.h>
#include <windows.h>
#include <strsafe.h>

#include "SingleloginCred.h"
#include "helpers.h"
#include "PlainDB.h"
#include "CSVLogger.h"
#include "SessionChecker.h"

#define MAX_CREDENTIALS 10

class CSingleloginProvider : public ICredentialProvider
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
			QITABENT(CSingleloginProvider, ICredentialProvider), // IID_ICredentialProvider
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

	friend HRESULT CSample_CreateInstance(__in REFIID riid, __deref_out void** ppv);
	void Activate(void);
	void updateTiles();

protected:
	CSingleloginProvider();
	__override ~CSingleloginProvider();

private:


	bool _unlocked;
	CCSVLogger _logger;
	CCSVLogger _dbglogger;
	CPlainDB db;
	LONG                                    _cRef;            // Used for reference counting.


	std::vector<CSingleloginCred*>          _creds;    // Our credential.
	CREDENTIAL_PROVIDER_USAGE_SCENARIO      _cpus;
	ICredentialProviderEvents *				_pcpe;
	UINT_PTR								_upAdviseContext;
	CSessionChecker							sessionChecker;

	HRESULT _EnumerateOne(TCHAR * name, TCHAR * password, CPlainDB::LOGIN_TYPE loginType);
	HRESULT _EnumerateOne(TCHAR * name, TCHAR * fullName, TCHAR * password, CPlainDB::LOGIN_TYPE loginType);
	HRESULT EnumerateLogon();
	HRESULT EnumerateUnlock();
};
