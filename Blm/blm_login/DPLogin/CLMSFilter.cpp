
#include "CLMSFilter.h"



CLMSFilter::CLMSFilter():
	_cRef(1)
{
	DllAddRef();
}

CLMSFilter::~CLMSFilter()
{

	DllRelease();
}

#define FILTER_ENABLED 1


IFACEMETHODIMP CLMSFilter:: Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags, GUID* rgclsidProviders, BOOL* rgbAllow, DWORD cProviders){
	UNREFERENCED_PARAMETER(dwFlags);
	switch (cpus)
	{
	case CPUS_CREDUI:
		
			return S_OK;
		
	case CPUS_LOGON:
	case CPUS_UNLOCK_WORKSTATION:
	case CPUS_CHANGE_PASSWORD:
#if FILTER_ENABLED
		//Filters out the default Windows provider (only for Logon and Unlock scenarios)
		for (DWORD i = 0; i < cProviders; i++)
		{
			if (!IsEqualGUID(rgclsidProviders[i], DLL_GUID) && !IsEqualGUID(rgclsidProviders[i],DLL_GUID2)) {
				rgbAllow[i] = FALSE;
			}
		}
#endif
		return S_OK;
	default:
		return E_INVALIDARG;
	}
}

HRESULT CLMSFilter::UpdateRemoteCredential(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsIn, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsOut)

{
	UNREFERENCED_PARAMETER(pcpcsOut);
	UNREFERENCED_PARAMETER(pcpcsIn);
    return E_NOTIMPL;
}


// Boilerplate code to create our provider.
HRESULT CLMSFilter_CreateInstance(__in REFIID riid, __deref_out void** ppv)
{
	HRESULT hr;

	CLMSFilter * pFilter = new CLMSFilter();

	if (pFilter)
	{
		hr = pFilter->QueryInterface(riid, ppv);
		pFilter->Release();
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}
