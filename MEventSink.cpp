// MEventSink.cpp --- MZC4 event sink
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software.

#include "MEventSink.hpp"
#include <exdisp.h>
#include <cstdio>
#include <cassert>

MEventSink::MEventSink() :
    m_cRefs(1),
    m_pListener(NULL),
    m_dwCookie(0),
    m_pConnectPoint(NULL)
{
}

MEventSink::~MEventSink()
{
}

/*static*/ MEventSink *MEventSink::Create()
{
    return new MEventSink();
}

bool MEventSink::Connect(IUnknown *pUnknown, MEventSinkListener *pListener)
{
    Disconnect();

    assert(pListener);
    m_pListener = pListener;

    IConnectionPointContainer *pContainer = NULL;

    HRESULT hr = pUnknown->QueryInterface(IID_PPV_ARGS(&pContainer));
    if (SUCCEEDED(hr))
    {
        hr = pContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &m_pConnectPoint);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pConnectPoint->Advise(this, &m_dwCookie);
    }

    if (pContainer)
    {
        pContainer->Release();
    }

    return SUCCEEDED(hr);
}

void MEventSink::Disconnect()
{
    if (m_dwCookie && m_pConnectPoint)
    {
        m_pConnectPoint->Unadvise(m_dwCookie);
        m_pConnectPoint->Release();
        m_pConnectPoint = NULL;
        m_dwCookie = 0;
    }
    m_pListener = NULL;
}

STDMETHODIMP MEventSink::QueryInterface(REFIID riid, void **ppvObj)
{
    if (!ppvObj)
        return E_POINTER;

    *ppvObj = NULL;

    if (riid == __uuidof(IUnknown) || riid == __uuidof(IDispatch) ||
        riid == __uuidof(DWebBrowserEvents2))
    {
        *ppvObj = static_cast<IDispatch *>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    AddRef();

    return S_OK;
}

STDMETHODIMP_(ULONG) MEventSink::AddRef()
{
	return ::InterlockedIncrement(&m_cRefs);
}

STDMETHODIMP_(ULONG) MEventSink::Release()
{
    if (!::InterlockedDecrement(&m_cRefs))
    {
        delete this;
        return 0;
    }
    return m_cRefs;
}

STDMETHODIMP MEventSink::GetTypeInfoCount(UINT *pctinfo)
{
    *pctinfo = 0;
    return S_OK;
}

STDMETHODIMP MEventSink::GetTypeInfo(
    UINT iTInfo,
    LCID lcid,
    ITypeInfo **ppTInfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP MEventSink::GetIDsOfNames(
    REFIID riid,
    LPOLESTR *rgszNames,
    UINT cNames,
    LCID lcid,
    DISPID *rgDispId)
{
    return E_NOTIMPL;
}

STDMETHODIMP MEventSink::Invoke(
    DISPID dispIdMember,
    REFIID riid,
    LCID lcid,
    WORD wFlags,
    DISPPARAMS *pDispParams,
    VARIANT *pVarResult,
    EXCEPINFO *pExcepInfo,
    UINT *puArgErr)
{
    if (m_pListener == NULL)
        return DISP_E_MEMBERNOTFOUND;

    switch (dispIdMember)
    {
    case DISPID_BEFORENAVIGATE2:
        assert(pDispParams->rgvarg[6].vt == VT_DISPATCH);
        assert(pDispParams->rgvarg[5].vt == VARTYPE(VT_BYREF | VT_VARIANT));
        assert(pDispParams->rgvarg[5].pvarVal->vt == VT_BSTR);
        assert(pDispParams->rgvarg[4].vt == VARTYPE(VT_BYREF | VT_VARIANT));
        assert(pDispParams->rgvarg[4].pvarVal->vt == VT_I4);
        assert(pDispParams->rgvarg[3].vt == VARTYPE(VT_BYREF | VT_VARIANT));
        assert(pDispParams->rgvarg[3].pvarVal->vt == VT_BSTR);
        assert(pDispParams->rgvarg[2].vt == VARTYPE(VT_BYREF | VT_VARIANT));
        assert(pDispParams->rgvarg[2].pvarVal->vt == VARTYPE(VT_BYREF | VT_VARIANT));
        assert(pDispParams->rgvarg[1].vt == VARTYPE(VT_BYREF | VT_VARIANT));
        assert(pDispParams->rgvarg[1].pvarVal->vt == VT_BSTR);
        assert(pDispParams->rgvarg[0].vt == VARTYPE(VT_BYREF | VT_BOOL));
        m_pListener->BeforeNavigate2(
            pDispParams->rgvarg[6].pdispVal,
            pDispParams->rgvarg[5].pvarVal,
            pDispParams->rgvarg[4].pvarVal,
            pDispParams->rgvarg[3].pvarVal,
            pDispParams->rgvarg[2].pvarVal,
            pDispParams->rgvarg[1].pvarVal,
            pDispParams->rgvarg[0].pboolVal);
        break;
    case DISPID_NAVIGATECOMPLETE2:
        assert(pDispParams->rgvarg[1].vt == VT_DISPATCH);
        assert(pDispParams->rgvarg[0].vt == VARTYPE(VT_BYREF | VT_VARIANT));
        m_pListener->NavigateComplete2(
            pDispParams->rgvarg[1].pdispVal,
            pDispParams->rgvarg[0].pvarVal->bstrVal);
        break;
    case DISPID_NEWWINDOW3:
        assert(pDispParams->rgvarg[4].vt == VARTYPE(VT_BYREF | VT_DISPATCH));
        assert(pDispParams->rgvarg[3].vt == VARTYPE(VT_BYREF | VT_BOOL));
        assert(pDispParams->rgvarg[2].vt == VT_I4);
        assert(pDispParams->rgvarg[1].vt == VT_BSTR);
        assert(pDispParams->rgvarg[0].vt == VT_BSTR);
        m_pListener->NewWindow3(
            pDispParams->rgvarg[4].ppdispVal,
            pDispParams->rgvarg[3].pboolVal,
            pDispParams->rgvarg[2].lVal,
            pDispParams->rgvarg[1].bstrVal,
            pDispParams->rgvarg[0].bstrVal);
        break;
    case DISPID_COMMANDSTATECHANGE:
        assert(pDispParams->rgvarg[1].vt == VT_I4);
        assert(pDispParams->rgvarg[0].vt == VT_BOOL);
        m_pListener->CommandStateChange(
            pDispParams->rgvarg[1].lVal,
            pDispParams->rgvarg[0].boolVal);
        break;
    case DISPID_TITLECHANGE:
        assert(pDispParams->rgvarg[0].vt == VT_BSTR);
        m_pListener->TitleTextChange(pDispParams->rgvarg[0].bstrVal);
        break;
    case DISPID_STATUSTEXTCHANGE:
        assert(pDispParams->rgvarg[0].vt == VT_BSTR);
        m_pListener->StatusTextChange(pDispParams->rgvarg[0].bstrVal);
        break;
    case DISPID_FILEDOWNLOAD:
        assert(pDispParams->rgvarg[1].vt == VT_BOOL);
        assert(pDispParams->rgvarg[0].vt == VARTYPE(VT_BYREF | VT_BOOL));
        m_pListener->FileDownload(
            pDispParams->rgvarg[1].boolVal,
            pDispParams->rgvarg[0].pboolVal);
        break;
    case DISPID_DOCUMENTCOMPLETE:
        assert(pDispParams->rgvarg[1].vt == VT_DISPATCH);
        assert(pDispParams->rgvarg[0].vt == VARTYPE(VT_BYREF | VT_VARIANT));
        assert(pDispParams->rgvarg[0].pvarVal->vt == VT_BSTR);
        m_pListener->DocumentComplete(
            pDispParams->rgvarg[1].pdispVal,
            pDispParams->rgvarg[0].pvarVal->bstrVal);
        break;
    case DISPID_NAVIGATEERROR:
        assert(pDispParams->rgvarg[4].vt == VT_DISPATCH);
        assert(pDispParams->rgvarg[3].vt == VARTYPE(VT_BYREF | VT_VARIANT));
        assert(pDispParams->rgvarg[3].pvarVal->vt == VT_BSTR);
        assert(pDispParams->rgvarg[2].vt == VARTYPE(VT_BYREF | VT_VARIANT));
        assert(pDispParams->rgvarg[2].pvarVal->vt == VT_BSTR);
        assert(pDispParams->rgvarg[1].vt == VARTYPE(VT_BYREF | VT_VARIANT));
        assert(pDispParams->rgvarg[1].pvarVal->vt == VT_I4);
        assert(pDispParams->rgvarg[0].vt == VARTYPE(VT_BYREF | VT_BOOL));
        m_pListener->NavigateError(
            pDispParams->rgvarg[4].pdispVal,
            pDispParams->rgvarg[3].pvarVal,
            pDispParams->rgvarg[2].pvarVal,
            pDispParams->rgvarg[1].pvarVal->lVal,
            pDispParams->rgvarg[0].pboolVal);
        break;
    case DISPID_DOWNLOADBEGIN:
        m_pListener->DownloadBegin();
        break;
    case DISPID_DOWNLOADCOMPLETE:
        m_pListener->DownloadComplete();
        break;
    case DISPID_SETSECURELOCKICON:
        assert(pDispParams->rgvarg[0].vt == VT_I4);
        m_pListener->SetSecureLockIcon(pDispParams->rgvarg[0].lVal);
        break;
    case DISPID_PROGRESSCHANGE:
        assert(pDispParams->rgvarg[1].vt == VT_I4);
        assert(pDispParams->rgvarg[0].vt == VT_I4);
        m_pListener->ProgressChange(
            pDispParams->rgvarg[1].lVal,
            pDispParams->rgvarg[0].lVal);
        break;
#ifndef DISPID_BEFORESCRIPTEXECUTE
    #define DISPID_BEFORESCRIPTEXECUTE 290
#endif
    case DISPID_BEFORESCRIPTEXECUTE:
        assert(pDispParams->cArgs == 1);
        assert(pDispParams->rgvarg[0].vt == VT_DISPATCH);
        m_pListener->BeforeScriptExecute(pDispParams->rgvarg[0].pdispVal);
        break;
    case DISPID_QUIT:
        assert(pDispParams->cArgs == 0);
        m_pListener->OnQuit();
        break;
    default:
        printf("dispIdMember: %ld\n", dispIdMember);
        return DISP_E_MEMBERNOTFOUND;
    }

    return S_OK;
}
