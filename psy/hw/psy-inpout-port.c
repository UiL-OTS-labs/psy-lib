
#include <stdio.h>

#include "../psy-utils.h"
#include "psy-config.h"
#include "psy-inpout-port.h"
#include "psy-utils.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

    #include <combaseapi.h>
    #include <wbemidl.h>
#endif

// Helper to link parallel port to the hardware I/O range resources
typedef struct {
    BSTR port_name;
    BSTR port_pnp_dev_id;
} WmiParportInfo;

static WmiParportInfo *
wmi_parport_info_new(BSTR port_name, BSTR port_pnp_dev_id)
{
    WmiParportInfo *info  = g_new(WmiParportInfo, 1);
    info->port_name       = port_name;
    info->port_pnp_dev_id = port_pnp_dev_id;
    return info;
}

static void
wmi_parport_info_free(WmiParportInfo *self)
{
    SysFreeString(self->port_name);
    SysFreeString(self->port_pnp_dev_id);
    g_free(self);
}

typedef struct InpoutPortInfo {
    PsyParallelPortInfo *port_info;
    guint64              port_address;
} InpoutPortInfo;

InpoutPortInfo *
inpout_port_info_new(gint n, gchar *name, guint64 port_address)
{
    g_return_val_if_fail(n >= 0 && port_address != 0, NULL);
    g_return_val_if_fail(name != NULL, NULL);

    InpoutPortInfo *new = g_new(InpoutPortInfo, 1);
    new->port_info      = psy_parallel_port_info_new(n, name);
    new->port_address   = port_address;

    return new;
}

void
inpout_port_info_free(InpoutPortInfo *self)
{
    g_return_if_fail(self != NULL);

    psy_parallel_port_info_free(self->port_info);
    g_free(self);
}

/* ******* global state concerning loading of inpout dll ******** */

static gint g_open_count = 0;

static HINSTANCE g_inpout_dll = NULL;
static GMutex    g_open_mutex;

/* ******* global state concerning Enumeration of parallel ports ***** */

static GRecMutex             g_init_mutex;
static gint                  g_num_infos           = 0;
static gint                  g_init_count          = 0;
static PsyParallelPortInfo **g_port_infos          = NULL;
static InpoutPortInfo      **g_internal_port_infos = NULL;

// Function should be called with g_init_mutex locked
static void
psy_inpout_port_clear_enum_cache(void)
{
    for (gint i = 0; i < g_num_infos; i++) {
        psy_parallel_port_info_free(g_port_infos[i]);
        inpout_port_info_free(g_internal_port_infos[i]);
    }
    g_clear_pointer(&g_port_infos, g_free);
    g_clear_pointer(&g_internal_port_infos, g_free);
}

/* *****  Enumerating parallel ports on windows  ****** */

// Setup COM and WMI
static void
query_wmi_for_parallel_ports(void);

// Query all parallel_ports: obtains LPTx and PNP_DeviceID's.
static GPtrArray *
proxy_get_parallel_ports(IWbemServices *proxy);

static guint64
proxy_get_port_resources(IWbemServices *proxy, BSTR port_pnp_id);

static gpointer
enum_ports_thread(gpointer data)
{
    (void) data;
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    char    error_buf[1024];
    if (FAILED(hr)) {
        g_critical("Unable to enum parallelports: CoInitialize failed");
        return NULL;
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
        psy_strerr(HRESULT_CODE(hr), error_buf, sizeof(error_buf));
        g_critical(
            "Unable to enum parallelports CoInitializeSecurity failed: %s",
            error_buf);
    }

    query_wmi_for_parallel_ports();

    return NULL;
}

static void
query_wmi_for_parallel_ports(void)
{
    IWbemLocator  *locator = NULL;
    IWbemServices *proxy   = NULL;
    GPtrArray     *ports   = NULL;

    BSTR namespace = SysAllocString(L"ROOT\\CIMV2");

    char error_buf[1024] = {0};

    HRESULT hr = CoCreateInstance(&CLSID_WbemLocator,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  &IID_IWbemLocator,
                                  (LPVOID *) &locator);

    if (FAILED(hr)) {
        psy_strerr(HRESULT_CODE(hr), error_buf, 1024);
        g_critical(
            "Unable to enum parallel ports, unable to create WbemLocator: %s",
            error_buf);
        goto fail;
    }

    hr = locator->lpVtbl->ConnectServer(
        locator,   // self
        namespace, // Object path of WMI namespace
        NULL,      // user: NULL = current user
        NULL,      // user_pass word, NULL is current password
        NULL,      // locale 0 current
        0,         // security flags
        NULL,      // Authority e.g. Kerberos
        NULL,      // Context object
        &proxy);   // OUTPUT Pointer to IwbemServicesProxy

    if (FAILED(hr)) {
        psy_strerr(HRESULT_CODE(hr), error_buf, 1024);
        g_critical("Unable to enum parallel ports, unable to create proxy: %s",
                   error_buf);
        goto fail;
    }

    hr = CoSetProxyBlanket((IUnknown *) proxy,
                           RPC_C_AUTHN_WINNT,
                           RPC_C_AUTHZ_NONE,
                           NULL,
                           RPC_C_AUTHN_LEVEL_CALL,
                           RPC_C_IMP_LEVEL_IMPERSONATE,
                           NULL,
                           EOAC_NONE);

    if (FAILED(hr)) {
        psy_strerr(HRESULT_CODE(hr), error_buf, 1024);
        g_critical(
            "Unable to enum parallel ports, unable to set proxy blanket: %s",
            error_buf);
        goto fail;
    }

    ports = proxy_get_parallel_ports(proxy);
    if (!ports) {
        g_critical("proxy_get_parallel_ports returned NULL");
        goto fail;
    }

    GPtrArray *internal_info
        = g_ptr_array_new_full(4, (GDestroyNotify) inpout_port_info_free);
    GPtrArray *enum_info
        = g_ptr_array_new_full(4, (GDestroyNotify) psy_parallel_port_info_free);

    // Add ports to temp cache
    for (guint i = 0; i < ports->len && i < G_MAXINT; i++) {
        WmiParportInfo *info = g_ptr_array_index(ports, i);
        guint64         resource
            = proxy_get_port_resources(proxy, info->port_pnp_dev_id);

        gchar *name = g_convert((const gchar *) info->port_name,
                                SysStringByteLen(info->port_name),
                                "utf-8",
                                "utf-16",
                                NULL,
                                NULL,
                                NULL); // freed by port_info.

        InpoutPortInfo *port_info = inpout_port_info_new(i, name, resource);
        PsyParallelPortInfo *enum_port_info
            = psy_parallel_port_info_copy(port_info->port_info);
        g_ptr_array_add(internal_info, port_info);
        g_ptr_array_add(enum_info, enum_port_info);
    }

    // Add ports to global cache
    g_num_infos = enum_info->len;
    if (g_num_infos > 0) {
        g_port_infos
            = (PsyParallelPortInfo **) g_ptr_array_free(enum_info, FALSE);
        g_internal_port_infos
            = (InpoutPortInfo **) g_ptr_array_free(internal_info, FALSE);
    }

fail:

    SysFreeString(namespace);

    if (ports) {
        g_ptr_array_free(ports, TRUE);
    }

    if (locator) {
        locator->lpVtbl->Release(locator);
    }
    if (proxy) {
        proxy->lpVtbl->Release(proxy);
    }
}

static GPtrArray *
proxy_get_parallel_ports(IWbemServices *proxy)
{
    BSTR    wql   = SysAllocString(L"WQL");
    BSTR    query = SysAllocString(L"select * from Win32_ParallelPort");
    char    err_buff[1024] = {0};
    HRESULT hr;

    GPtrArray *ret
        = g_ptr_array_new_full(1, (GDestroyNotify) wmi_parport_info_free);

    IEnumWbemClassObject *ports = NULL;

    hr = proxy->lpVtbl->ExecQuery(proxy,
                                  wql,
                                  query,
                                  WBEM_FLAG_FORWARD_ONLY
                                      | WBEM_FLAG_RETURN_IMMEDIATELY,
                                  NULL,
                                  &ports);
    if (FAILED(hr)) {
        psy_strerr(HRESULT_CODE(hr), err_buff, sizeof(err_buff));
        g_critical("Unable to query Win32_ParallelPort from WMI");
        goto fail; // just return empty ports
    }

    while (ports) {

        VARIANT           value;
        IWbemClassObject *port = NULL;
        ULONG             result;

        hr = ports->lpVtbl->Next(ports, WBEM_INFINITE, 1, &port, &result);
        if (FAILED(hr)) {
            psy_strerr(HRESULT_CODE(hr), err_buff, sizeof(err_buff));
            g_critical("Unable to loop over selected parallel ports: %s",
                       err_buff);
            goto fail; // just return allready found ports.
        }

        if (result == 0)
            break;

        // These 2 BSTRs are added to wmi_parport_info, who will release
        // these when the return GPtrArray is destroyed.
        hr = port->lpVtbl->Get(port, L"DeviceID", 0, &value, NULL, NULL);

        BSTR device_id = SysAllocString(value.bstrVal);
        VariantClear(&value);

        hr = port->lpVtbl->Get(port, L"PNPDeviceID", 0, &value, NULL, NULL);

        BSTR pnp_dev_id = SysAllocString(value.bstrVal);

        WmiParportInfo *info = wmi_parport_info_new(device_id, pnp_dev_id);

        // array will cleanup the bstr's.
        g_ptr_array_add(ret, info);
    }

    ports->lpVtbl->Release(ports);

fail:
    SysFreeString(wql);
    SysFreeString(query);
    return ret;
}

static guint64
proxy_get_port_resources(IWbemServices *proxy, BSTR pnp_dev_id)
{
    BSTR    wql   = SysAllocString(L"WQL");
    BSTR    query = SysAllocString(L"select * from Win32_PNPAllocatedResource");
    char    err_buff[1024] = {0};
    HRESULT hr;

    guint64 ret = 0;

    IEnumWbemClassObject *resources = NULL;

    hr = proxy->lpVtbl->ExecQuery(proxy,
                                  wql,
                                  query,
                                  WBEM_FLAG_FORWARD_ONLY
                                      | WBEM_FLAG_RETURN_IMMEDIATELY,
                                  NULL,
                                  &resources);
    if (FAILED(hr)) {
        psy_strerr(HRESULT_CODE(hr), err_buff, sizeof(err_buff));
        g_critical("Unable to query Win32_PNPAllocatedResource from WMI");
        goto fail; // just return null
    }

    while (resources) {

        VARIANT           value;
        IWbemClassObject *pnp_resource = NULL;
        IWbemClassObject *reference    = NULL;
        ULONG             result;

        hr = resources->lpVtbl->Next(
            resources, WBEM_INFINITE, 1, &pnp_resource, &result);

        if (FAILED(hr)) {
            psy_strerr(HRESULT_CODE(hr), err_buff, sizeof(err_buff));
            g_critical("Unable to loop over selected Win32_PNPResource's: %s",
                       err_buff);
            goto fail; // just return allready found ports.
        }

        if (result == 0)
            break;

        // It might be easier to get a reference from an IWBemClassObject
        CIMTYPE cim_type;
        hr = pnp_resource->lpVtbl->Get(
            pnp_resource, L"Dependent", 0, &value, &cim_type, NULL);

        if (cim_type != CIM_REFERENCE) {
            g_warning("Expected CIM_REFERENCE");
            VariantClear(&value);
            continue;
        }

        hr = proxy->lpVtbl->GetObject(
            proxy, value.bstrVal, 0, NULL, &reference, NULL);
        if (FAILED(hr)) {
            g_critical("Unable to get Dependent object");
        }

        VariantClear(&value);
        hr = reference->lpVtbl->Get(
            reference, L"DeviceID", 0, &value, &cim_type, NULL);

        if (wcscmp(value.bstrVal, pnp_dev_id) != 0) {
            // This is another device, ignore it
            VariantClear(&value);
            continue;
        }

        // We've found the right port resource
        reference->lpVtbl->Release(reference);
        reference = NULL;
        VariantClear(&value);

        hr = pnp_resource->lpVtbl->Get(
            pnp_resource, L"Antecedent", 0, &value, &cim_type, NULL);

        g_assert(cim_type == CIM_REFERENCE);

        hr = proxy->lpVtbl->GetObject(
            proxy, value.bstrVal, 0, NULL, &reference, NULL);

        VariantClear(&value);

        hr = reference->lpVtbl->Get(
            reference, L"StartingAddress", 0, &value, &cim_type, NULL);
        if (FAILED(hr)) {
            g_critical("Failed to find StartingAddress");
        }
        g_assert(cim_type == CIM_UINT64);

        // Although it is a UINT64, it's still encoded as bstr in the value,
        // hence we need convert it to a number.

        GError *error = NULL;

        char *start_addr_utf8 = g_convert((const gchar *) value.bstrVal,
                                          SysStringByteLen(value.bstrVal),
                                          "UTF-8",
                                          "UTF-16",
                                          NULL,
                                          NULL,
                                          &error);
        if (error) {
            g_critical("Unable to convert from BSTR to utf8: %s",
                       error->message);
            g_clear_error(&error);
        }

        if (start_addr_utf8) {
            ret = g_ascii_strtoull(start_addr_utf8, NULL, 10);
            g_free(start_addr_utf8);
        }
        VariantClear(&value);
        reference->lpVtbl->Release(reference);

        break;
    }

    resources->lpVtbl->Release(resources);

fail:
    SysFreeString(wql);
    SysFreeString(query);
    return ret;
}

static void
psy_parallel_enumeration_procedure(PsyParallelPort       *self,
                                   PsyParallelPortInfo ***result,
                                   gint                  *num)
{
    (void) self;

    // Create thread for data, as COM must be initialized for each thread.
    // And if we do it this way, it doesn't matter how the client does
    // it.
    GThread *t = g_thread_new("enum-parallel-ports", enum_ports_thread, NULL);
    g_thread_join(t);

    if (g_port_infos) {
        *result = g_port_infos;
        *num    = g_num_infos;
    }
}

/* ******* loading of inpout dll for controlling SPP registers ******** */

typedef void(__stdcall *lpOut32)(short, short);
typedef short(__stdcall *lpInp32)(short);
typedef BOOL(__stdcall *lpIsInpOutDriverOpen)(void);

lpOut32              spp_write;
lpInp32              spp_read;
lpIsInpOutDriverOpen opened_dll;

#if PSY_TARGET_ARCH_X86
const gchar *g_dll_name = "inpout32.dll";
#elif PSY_TARGET_ARCH_X86_64
const gchar *g_dll_name = "inpoutx64.dll";
#else
    #error "Unsupported platform"
#endif

static gboolean
init_win_parallel_port(GError **error)
{
    g_mutex_lock(&g_open_mutex);

    g_open_count++;

    if (g_open_count == 1) {
        g_inpout_dll = LoadLibraryA(g_dll_name);

        if (g_inpout_dll == NULL) {
            char buff[BUFSIZ];
            gint error_code = GetLastError();
            psy_strerr(error_code, buff, BUFSIZ);

            g_set_error(error,
                        PSY_PARALLEL_PORT_ERROR,
                        PSY_PARALLEL_PORT_ERROR_OPEN,
                        "Unable to load library \"%s\": %s",
                        g_dll_name,
                        buff);
            goto error;
        }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
        spp_write  = (lpOut32) GetProcAddress(g_inpout_dll, "Out32");
        spp_read   = (lpInp32) GetProcAddress(g_inpout_dll, "Inp32");
        opened_dll = (lpIsInpOutDriverOpen) GetProcAddress(
            g_inpout_dll, "IsInpOutDriverOpen");
#pragma GCC diagnostic pop

        g_assert(spp_write != NULL && spp_read != 0 && opened_dll != NULL);

        if (opened_dll == NULL || !opened_dll())
            g_set_error(error,
                        PSY_PARALLEL_PORT_ERROR,
                        PSY_PARALLEL_PORT_ERROR_OPEN,
                        "Loaded library, but dll isn't open");
    }

    g_mutex_unlock(&g_open_mutex);
    return TRUE;

error:

    g_open_count--;

    g_mutex_unlock(&g_open_mutex);
    return FALSE;
}

static gboolean
deinit_win_parallel_port(void)
{
    g_mutex_lock(&g_open_mutex);

    g_open_count--;
    if (g_open_count < 0) {
        g_critical("open count less than zero");
    }

    if (g_open_count == 0) {
        FreeLibrary(g_inpout_dll);
        g_inpout_dll = NULL;

        spp_write  = NULL;
        spp_read   = NULL;
        opened_dll = NULL;
    }

    g_mutex_unlock(&g_open_mutex);
    return TRUE;
}

/**
 * PsyInpoutPort:
 *
 * PsyInpoutPort is a final class for parallel ports on Windows. It derives from
 * PsyParallelPort and implements it. Typically you can instantiate instances
 * of this class with [ctor@ParallelPort.new] and that constructor
 * determines the right backend for your system.
 *
 * The device is opened by a number, the numbers relate to addresses.
 * The following table applies:
 * - *0* = 0x378
 * - *1* = 0x278
 * - *2* = 0x3BC
 *
 * If you want to know which port it is use the windows device manager
 * to see what port maps to the address above. The ports are known as
 * LPTx where the x is 1, 2 or 3. To keep numbering consistent, psylib uses
 * 0, 1, 2.
 *
 * In order to use PsyInpoutPort, you'll need inpout.dll. PsyLib tries to
 * ship with this dll. You'll can find it online (November 2024) at:
 *
 * https://www.highrez.co.uk/Downloads/InpOut32/
 *
 * this is a website from Phillip Gibbons. Back when I was young, one
 * was able to write to the registers of a parallel port. This way you
 * could bit-bang transmission between a pc and a printer.
 * We (ab)use this in order to trigger interfaces. The dll makes this
 * possible PsyLib does sanitize the addresses one can write to.
 *
 * Because, on windows were just writing to the registers, we cannot tell
 * whether there actually is connected on that address, hence look up your
 * device address in windows device manager. If you have multiple devices,
 * they will appear on different addresses and you'll have to determine
 * which address belongs to which port, before using the device.
 */

struct LptAddressMap {
    gchar *port_name;    // Name of the port e.g. LPT1
    gint   port_number;  // for port LPTx port_number is x - 1.
    gint   port_address; // Address of the data_register
};

typedef struct _PsyInpoutPort {
    PsyParallelPort parent;
    gint16          data_register;
    gint16          status_register;
    gint16          control_register;
} PsyInpoutPort;

G_DEFINE_TYPE(PsyInpoutPort, psy_inpout_port, PSY_TYPE_PARALLEL_PORT)

static void
psy_inpout_port_init(PsyInpoutPort *self)
{
    (void) self;
    g_rec_mutex_lock(&g_init_mutex);

    if (++g_init_count == 1) {
        psy_parallel_port_enumerate(
            PSY_PARALLEL_PORT(self), &g_port_infos, &g_num_infos);
    }

    g_rec_mutex_unlock(&g_init_mutex);
}

static void
inpout_port_dispose(GObject *obj)
{
    g_rec_mutex_lock(&g_init_mutex);

    if (--g_init_count == 0) {
        psy_inpout_port_clear_enum_cache();
    }

    g_rec_mutex_unlock(&g_init_mutex);

    G_OBJECT_CLASS(psy_inpout_port_parent_class)->dispose(obj);
}

static void
inpout_port_open(PsyParallelPort *self, gint port_num, GError **error)
{
    gchar                 buffer[64];
    PsyInpoutPort        *pp        = PSY_INPOUT_PORT(self);
    PsyParallelPortInfo **ports     = NULL;
    gint                  num_ports = 0;

    PsyParallelPortClass *parallel_cls = PSY_PARALLEL_PORT_GET_CLASS(self);

    psy_parallel_port_enumerate(self, &ports, &num_ports);
    if (port_num >= num_ports) {
        g_set_error(error,
                    PSY_PARALLEL_PORT_ERROR,
                    PSY_PARALLEL_PORT_ERROR_OPEN,
                    "No device enumerated with port_num: %d",
                    port_num);
        return;
    }

    PsyParallelPortInfo *info = g_port_infos[port_num];
    InpoutPortInfo      *port_reg_info
        = g_internal_port_infos[psy_parallel_port_info_port_number(info)];

    // assign port numbers
    pp->data_register    = port_reg_info->port_address;
    pp->status_register  = pp->data_register + 1;
    pp->control_register = pp->data_register + 2;

    psy_parallel_port_close(self);

    init_win_parallel_port(error);
    if (*error != NULL)
        return;

    PSY_PARALLEL_PORT_CLASS(psy_inpout_port_parent_class)
        ->open(self, port_num, error);

    g_snprintf(buffer, sizeof(buffer), "0x%04X", pp->data_register);
    parallel_cls->set_port_name(self, buffer);
}

static void
inpout_port_close(PsyParallelPort *self)
{
    // close inpout.dll on close of last input/output device
    // clear enumeration info
    deinit_win_parallel_port();

    PSY_PARALLEL_PORT_CLASS(psy_inpout_port_parent_class)->close(self);
}

static void
inpout_port_write(PsyParallelPort *self, guint8 pins, GError **error)
{
    // TODO put duplicate code in psy-parallel-port.c
    PsyInpoutPort *pp      = PSY_INPOUT_PORT(self);
    gboolean       is_open = psy_parallel_port_is_open(self);
    gboolean       is_output
        = psy_parallel_port_get_direction(self) == PSY_IO_DIRECTION_OUT;

    if (!is_open) {
        g_set_error(error,
                    PSY_PARALLEL_PORT_ERROR,
                    PSY_PARALLEL_PORT_ERROR_DEV_CLOSED,
                    "Can't write to closed device");
        return;
    }

    if (!is_output) {
        g_set_error(
            error,
            PSY_PARALLEL_PORT_ERROR,
            PSY_PARALLEL_PORT_ERROR_DIRECTION,
            "Unable to write to a port that is not configured for output.");
    }

    spp_write(pp->data_register, pins);

    psy_parallel_port_set_pins(self, pins);
}

static void
inpout_port_write_pin(PsyParallelPort *self,
                      gint             pin,
                      PsyIoLevel       level,
                      GError         **error)
{
    guint8 current = psy_parallel_port_get_pins(self);
    guint8 final;
    if (level == PSY_IO_LEVEL_HIGH) {
        final = current | 1ul << pin;
    }
    else {
        final = current & ~(1ul << pin);
    }

    psy_parallel_port_write(self, final, error);
}

static guint8
inpout_port_read(PsyParallelPort *self, GError **error)
{
    PsyInpoutPort *pp      = PSY_INPOUT_PORT(self);
    gboolean       is_open = psy_parallel_port_is_open(self);
    gboolean       is_input
        = psy_parallel_port_get_direction(self) == PSY_IO_DIRECTION_IN;
    guint8 lines = 0;

    if (!is_open) {
        g_set_error(error,
                    PSY_PARALLEL_PORT_ERROR,
                    PSY_PARALLEL_PORT_ERROR_DEV_CLOSED,
                    "Can't read from closed device");
        return 0;
    }

    if (!is_input) {
        g_set_error(
            error,
            PSY_PARALLEL_PORT_ERROR,
            PSY_PARALLEL_PORT_ERROR_DIRECTION,
            "Unable to read from a port that is not configured as input.");
    }

    lines = spp_read(pp->data_register);

    psy_parallel_port_set_pins(self, lines);
    return lines;
}

static PsyIoLevel
inpout_port_read_pin(PsyParallelPort *self, gint pin, GError **error)
{
    guint8 pins = psy_parallel_port_read(self, error);
    if (error && (*error != NULL)) {
        return PSY_IO_LEVEL_LOW;
    }

    return pins & (1ul << pin) ? PSY_IO_LEVEL_HIGH : PSY_IO_LEVEL_LOW;
}

static void
inpout_port_enumerate(PsyParallelPort       *self,
                      PsyParallelPortInfo ***result,
                      gint                  *num)
{
    g_rec_mutex_lock(&g_init_mutex);

    if (g_port_infos != NULL) {
        *result = g_port_infos;
        *num    = g_num_infos;
    }
    else {
        psy_parallel_enumeration_procedure(self, result, num);
    }

    g_rec_mutex_unlock(&g_init_mutex);
}

static void
psy_inpout_port_class_init(PsyInpoutPortClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);
    obj_cls->dispose      = inpout_port_dispose;

    PsyParallelPortClass *parallel_cls = PSY_PARALLEL_PORT_CLASS(cls);

    parallel_cls->open      = inpout_port_open;
    parallel_cls->close     = inpout_port_close;
    parallel_cls->write     = inpout_port_write;
    parallel_cls->write_pin = inpout_port_write_pin;
    parallel_cls->read      = inpout_port_read;
    parallel_cls->read_pin  = inpout_port_read_pin;
    parallel_cls->enumerate = inpout_port_enumerate;
}
