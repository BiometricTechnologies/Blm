/**************************************************************************
    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   (c) Microsoft Corporation. All Rights Reserved.
**************************************************************************/

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "utils.h"
#include "resource.h"
#include <new>  // std::nothrow
#define MENUVERB_DISPLAY     0



const ICIVERBTOIDMAP c_FolderViewImplContextMenuUnsecureIDMap[] =
{
    { L"display",    "display",   MENUVERB_DISPLAY, 0, },
    { NULL,          NULL,        (UINT)-1,         0, }
};


class CFolderViewImplContextMenuUnsecure   : public IContextMenu
                                   , public IShellExtInit
                                   , public IObjectWithSite
{
public:
    CFolderViewImplContextMenuUnsecure() : _cRef(1), _punkSite(NULL), _pdtobj(NULL)
    {
        DllAddRef();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] = {
            QITABENT(CFolderViewImplContextMenuUnsecure, IContextMenu),
            QITABENT(CFolderViewImplContextMenuUnsecure, IShellExtInit),
            QITABENT(CFolderViewImplContextMenuUnsecure, IObjectWithSite),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }


    // IContextMenu
    IFACEMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    IFACEMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);
    IFACEMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pRes, LPSTR pszName, UINT cchMax);

    // IShellExtInit
    IFACEMETHODIMP Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID);

    // IObjectWithSite
    IFACEMETHODIMP SetSite(IUnknown *punkSite);
    IFACEMETHODIMP GetSite(REFIID riid, void **ppvSite);

private:
    long    _cRef;
    IDataObject *_pdtobj;
    IUnknown *_punkSite;

    ~CFolderViewImplContextMenuUnsecure()
    {
        // _punkSite should be NULL due to SetSite(NULL).
        if (_pdtobj)
        {
            _pdtobj->Release();
        }
        DllRelease();
    }
};

HRESULT CFolderViewImplContextMenuUnsecure_CreateInstance(REFIID riid, void **ppv)
{
    *ppv = NULL;
    CFolderViewImplContextMenuUnsecure* pFolderViewImplContextMenu = new (std::nothrow) CFolderViewImplContextMenuUnsecure();
    HRESULT hr = pFolderViewImplContextMenu ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pFolderViewImplContextMenu->QueryInterface(riid, ppv);
        pFolderViewImplContextMenu->Release();
    }
    return hr;
}

HRESULT CFolderViewImplContextMenuUnsecure::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT /* idCmdLast */, UINT /* uFlags */)
{
    WCHAR szMenuItem[80];
    LoadString(g_hInst, IDS_UNSECURE, szMenuItem, ARRAYSIZE(szMenuItem));
    InsertMenu(hmenu, indexMenu++, MF_BYPOSITION, idCmdFirst + MENUVERB_DISPLAY, szMenuItem);
    // other verbs could go here...

    // indicate that we added one verb.
    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(1));
}

const ICIVERBTOIDMAP* _CmdIDToMapUnsecure(UINT_PTR idCmd, BOOL fUnicode, const ICIVERBTOIDMAP* pmap)
{
    const ICIVERBTOIDMAP* pmapRet = NULL;
    if (IS_INTRESOURCE(idCmd))
    {
        UINT idVerb = (UINT)idCmd;
        while (!pmapRet && -1 != pmap->idCmd)
        {
            if (pmap->idCmd == idVerb)
            {
                pmapRet = pmap;
            }
            pmap++;
        }
    }
    else if (fUnicode)
    {
        LPCWSTR pszVerb = (LPCWSTR)idCmd;
        while (!pmapRet && -1 != pmap->idCmd)
        {
            if (pmap->pszCmd && 0 == StrCmpIC(pszVerb, pmap->pszCmd))
            {
                pmapRet = pmap;
            }
            pmap++;
        }
    }
    else
    {
        LPCSTR pszVerbA = (LPCSTR)idCmd;
        while (!pmapRet && -1 != pmap->idCmd)
        {
            if (pmap->pszCmdA && 0 == StrCmpICA(pszVerbA, pmap->pszCmdA))
            {
                pmapRet = pmap;
            }
            pmap++;
        }
    }
    return pmapRet;
}

#define IS_UNICODE_ICI(pici) (((pici)->fMask & CMIC_MASK_UNICODE) == CMIC_MASK_UNICODE)

HRESULT _MapICIVerbToCmdIDUnsecure(LPCMINVOKECOMMANDINFO pici, const ICIVERBTOIDMAP* pmap, UINT* pid)
{
    HRESULT hr = E_FAIL;

    if (!IS_INTRESOURCE(pici->lpVerb))
    {
        UINT_PTR idCmd;
        BOOL fUnicode;

        if (IS_UNICODE_ICI(pici) && ((LPCMINVOKECOMMANDINFOEX)pici)->lpVerbW)
        {
            fUnicode = TRUE;
            idCmd = (UINT_PTR)(((LPCMINVOKECOMMANDINFOEX)pici)->lpVerbW);
        }
        else
        {
            fUnicode = FALSE;
            idCmd = (UINT_PTR)(pici->lpVerb);
        }

        pmap = _CmdIDToMapUnsecure(idCmd, fUnicode, pmap);
        if (pmap)
        {
            *pid = pmap->idCmd;
            hr = S_OK;
        }
    }
    else
    {
        *pid = LOWORD((UINT_PTR)pici->lpVerb);
        hr = S_OK;
    }
    return hr;
}

HRESULT CFolderViewImplContextMenuUnsecure::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr = E_INVALIDARG;
    UINT uID;
    // Is this command for us?
    if (SUCCEEDED(_MapICIVerbToCmdIDUnsecure(pici, c_FolderViewImplContextMenuUnsecureIDMap, &uID)) && uID == MENUVERB_DISPLAY && _pdtobj)
    {
        IShellItemArray *psia;
        hr = SHCreateShellItemArrayFromDataObject(_pdtobj, IID_PPV_ARGS(&psia));
        if (SUCCEEDED(hr))
        {
			hr = Unsecure(psia, pici->hwnd);
			psia->Release();
        }
    }
    return hr;
}

HRESULT CFolderViewImplContextMenuUnsecure::GetCommandString(UINT_PTR /* idCmd */, UINT /* uType */, UINT * /* pRes */, LPSTR /* pszName */, UINT /* cchMax */)
{
    return E_NOTIMPL;
}

HRESULT CFolderViewImplContextMenuUnsecure::Initialize(PCIDLIST_ABSOLUTE /* pidlFolder */, IDataObject *pdtobj, HKEY /* hkeyProgID */)
{
    if (_pdtobj)
    {
        _pdtobj->Release();
        _pdtobj = NULL;
    }

    _pdtobj = pdtobj;
    if (pdtobj)
    {
        pdtobj->AddRef();
    }
    return S_OK;
}

HRESULT CFolderViewImplContextMenuUnsecure::SetSite(IUnknown *punkSite)
{
    if (_punkSite)
    {
        _punkSite->Release();
        _punkSite = NULL;
    }

    _punkSite = punkSite;
    if (punkSite)
    {
        punkSite->AddRef();
    }
    return S_OK;
}

HRESULT CFolderViewImplContextMenuUnsecure::GetSite(REFIID riid, void **ppvSite)
{
    return _punkSite ? _punkSite->QueryInterface(riid, ppvSite) : E_FAIL;
}
