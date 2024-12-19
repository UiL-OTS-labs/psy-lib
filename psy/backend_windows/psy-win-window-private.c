

#include <d3d11.h>
#include <dxgi.h>
#include <wchar.h>

#include "psy-display-info.h"
#include "psy-utils.h"

/**
 * psy_win_window_enumerate_adapters:(skip)
 *
 * Use DXGI (DirectX Graphics Infrastructure) to obtain
 * the available adapters.
 *
 * Return:(transfer full)(element-type IDXGIAdapter*)
 */
GPtrArray *
psy_win_window_enumerate_adapters(void)
{
    IDXGIFactory1 *factory = NULL;
    HRESULT hr = CreateDXGIFactory1(&IID_IDXGIFactory1, (void **) &factory);

    if (FAILED(hr)) {
        char error[1024];
        psy_strerr(GetLastError(), error, sizeof(error));
        g_critical("Unable to get DXGIFactory1: %s", error);
        factory->lpVtbl->Release(factory);
        return NULL;
    }

    GPtrArray *adapters
        = g_ptr_array_new_full(16, psy_release_com_instance); // VideoCards

    IDXGIAdapter *adapter     = NULL;
    int           nth_adapter = 0; // nth adapter
    while (factory->lpVtbl->EnumAdapters(factory, nth_adapter, &adapter)
           != DXGI_ERROR_NOT_FOUND) {
        g_ptr_array_add(adapters, adapter);
        nth_adapter++;
    }

    factory->lpVtbl->Release(factory);

    return adapters;
}

static void
append_adapter_monitors(gpointer a, gpointer m)
{
    IDXGIAdapter *adapter  = a;
    GPtrArray    *monitors = m;
    int           nth_mon  = 0; // nth_monitor
    IDXGIOutput  *monitor  = NULL;

    while (adapter->lpVtbl->EnumOutputs(adapter, nth_mon, &monitor)
           != DXGI_ERROR_NOT_FOUND) {
        nth_mon++;
        g_ptr_array_add(monitors, monitor);
    }
}

/**
 * psy_win_window_enumerate_outputs:(skip)
 *
 * Use DXGI (DirectX Graphics Infrastructure) to obtain
 * the available adapters.
 *
 * Return:(transfer full)(element-type IDXGIOutput*)
 */
GPtrArray *
psy_win_window_enumerate_outputs(void)
{
    GPtrArray *adapters = psy_win_window_enumerate_adapters();
    GPtrArray *monitors
        = g_ptr_array_new_full(16, psy_release_com_instance); // outputs

    // Obtain all monitors for each adapter
    g_ptr_array_foreach(adapters, append_adapter_monitors, monitors);

    g_ptr_array_free(adapters, TRUE);

    return monitors;
}

void
append_display_info(gpointer output, gpointer display_info_array)
{
    HRESULT      hr;
    IDXGIOutput *monitor    = output;
    GPtrArray   *disp_infos = display_info_array;
    PsyDuration *frame_dur  = NULL;
    char         error[1024];
    GError      *conv_error = NULL;

    DXGI_OUTPUT_DESC description;
    MONITORINFOEX    mon_info;
    mon_info.cbSize = sizeof(mon_info);
    DEVMODEA device_mode;

    hr = monitor->lpVtbl->GetDesc(monitor, &description);
    if (FAILED(hr)) {
        psy_strerr(hr, error, sizeof(error));
        g_critical("Unable to get monitor description: GetDesc failed:%s",
                   error);
        return;
    }
    if (!GetMonitorInfoA(description.Monitor, (LPMONITORINFO) &mon_info)) {
        psy_strerr(hr, error, sizeof(error));
        g_critical(
            "Unable to get monitor description: GetMonitorInfo failed:%s",
            error);
        return;
    }

    // guint  n_wchar_bytes = wcslen(description.DeviceName) * sizeof(wchar_t);
    // gchar *device_name   = g_convert(description.DeviceName,
    //                                n_wchar_bytes,
    //                                "ascii",
    //                                "utf16",
    //                                NULL,
    //                                NULL,
    //                                &conv_error);
    if (!EnumDisplaySettingsA(
            mon_info.szDevice, ENUM_CURRENT_SETTINGS, &device_mode)) {
        psy_strerr(hr, error, sizeof(error));
        g_critical("Unable to get device-mode: EnumDisplaySettingsA failed:%s",
                   error);
        return;
    }
    if (device_mode.dmDisplayFrequency > 1) {
        frame_dur = psy_duration_new(1.0 / device_mode.dmDisplayFrequency);
    }
    PsyDisplayInfo *info
        = psy_display_info_new_full(mon_info.szDevice,
                                    frame_dur,
                                    description.DesktopCoordinates.left,
                                    description.DesktopCoordinates.top,
                                    description.DesktopCoordinates.right
                                        - description.DesktopCoordinates.left,
                                    description.DesktopCoordinates.bottom
                                        - description.DesktopCoordinates.top,
                                    -1,
                                    -1);
    g_ptr_array_add(disp_infos, info);
}

GPtrArray *
psy_win_window_enumerate_displays(void)
{
    GPtrArray *display_info_s = NULL;
    GPtrArray *outputs        = psy_win_window_enumerate_outputs();
    if (!outputs)
        return NULL;

    display_info_s
        = g_ptr_array_new_full(16, (GDestroyNotify) psy_display_info_free);
    g_ptr_array_foreach(outputs, append_display_info, display_info_s);

    g_ptr_array_free(outputs, TRUE);

    return display_info_s;
}
