
#include <stdio.h>

#define _WIN32_DCOM

#include <wbemidl.h>
#include <windows.h>

int g_port_num = 1;

int
main(int argc, char **argv)
{
    HRESULT        hr;
    IWbemLocator  *pLoc = NULL;
    IWbemServices *pSvc = NULL;

    hr = CoInitializeEx(0, COINIT_MULTITHREADED);

    if (FAILED(hr)) {
        fprintf(stderr, "Unable to init COM: 0x%lx", hr);
        return EXIT_FAILURE;
    }

    hr = CoInitializeSecurity(NULL,
                              -1,
                              NULL,
                              NULL,
                              RPC_C_AUTHN_LEVEL_DEFAULT,
                              RPC_C_IMP_LEVEL_IMPERSONATE,
                              NULL,
                              EOAC_NONE,
                              NULL);
    if (FAILED(hr)) {
        fprintf(stderr, "Unable to setup security: 0x%lx", hr);
        goto end;
    }

    hr = CoCreateInstance(&CLSID_WbemLocator,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_IWbemLocator,
                          (LPVOID *) &pLoc);
    BSTR namespace
        = SysAllocString(L"ROOT\\CIMV2"); // Object path of WMI namespace

    hr = pLoc->lpVtbl->ConnectServer(
        pLoc,      // self
        namespace, // Object pat of WMI namespace
        NULL,      // user: NULL = current user
        NULL,      // user_pass word, NULL is current password
        NULL,      // locale 0 current
        0,         // security flags
        NULL,      // Authority e.g. Kerberos
        NULL,      // Context object
        &pSvc);    // OUTPUT Pointer to IwbemServicesProxy

    if (FAILED(hr)) {
        fprintf(stderr, "Unable to connect: 0x%lx", hr);
        goto end;
    }

    hr = CoSetProxyBlanket((IUnknown *) pSvc,
                           RPC_C_AUTHN_WINNT,
                           RPC_C_AUTHZ_NONE,
                           NULL,
                           RPC_C_AUTHN_LEVEL_CALL,
                           RPC_C_IMP_LEVEL_IMPERSONATE,
                           NULL,
                           EOAC_NONE);
    if (FAILED(hr)) {
        fprintf(stderr, "Unable to set proxy blanket: 0x%lx", hr);
        goto end;
    }

    IEnumWbemClassObject *pPortEnumerator = NULL;

    BSTR    wql = SysAllocString(L"WQL");
    wchar_t query_plain[1024];
    swprintf(query_plain,
             1024,
             L"select * from Win32_ParallelPort where DeviceID=\"LPT%d\"",
             g_port_num);
    BSTR port_query = SysAllocString(query_plain);

    hr = pSvc->lpVtbl->ExecQuery(pSvc,       // self
                                 wql,        // use WQL for queries
                                 port_query, // the actual query
                                 WBEM_FLAG_FORWARD_ONLY
                                     | WBEM_FLAG_RETURN_IMMEDIATELY,
                                 NULL,
                                 &pPortEnumerator);
    if (FAILED(hr)) {
        fprintf(stderr, "Unable to query: 0x%lx\n", hr);
        goto end;
    }

    IWbemClassObject *pClsObject;
    ULONG             result;
    VARIANT           value;
    BSTR              pnp_dev_id = NULL;

    while (pPortEnumerator) {
        hr = pPortEnumerator->lpVtbl->Next(
            pPortEnumerator, WBEM_INFINITE, 1, &pClsObject, &result);
        if (result == 0) {
            break;
        }

        if (FAILED(hr)) {
            fprintf(stderr, "Unable to enumerate parallel ports\n");
            goto end;
        }

        hr = pClsObject->lpVtbl->Get(
            pClsObject, L"PNPDeviceID", 0, &value, NULL, NULL);
        pnp_dev_id = SysAllocString(value.bstrVal);
        VariantClear(&value);
    }

    if (pnp_dev_id) {
        if (fprintf(stdout, "dev-id = %ls\n", pnp_dev_id) < 0) {
            perror("fprintf");
        }
    }
    else {
        fprintf(stdout, "LPT%d, not found\n", g_port_num);
    }

end:
    SysFreeString(wql);
    pSvc->lpVtbl->Release(pSvc);
    pLoc->lpVtbl->Release(pLoc);
    CoUninitialize();
}