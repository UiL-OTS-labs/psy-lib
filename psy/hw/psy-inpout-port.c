
#include <stdio.h>

#include "../psy-utils.h"
#include "psy-config.h"
#include "psy-inpout-port.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

/* ******* loading of inpout dll for controlling SPP registers ******** */

static gint      g_open_count = 0;
static HINSTANCE g_inpout_dll = NULL;
static GMutex    g_open_mutex;

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
load_inpout(GError **error)
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

        if (!opened_dll())
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
unload_inpout(void)
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
}

static void
inpout_port_open(PsyParallelPort *self, gint port_num, GError **error)
{
    gchar          buffer[64];
    PsyInpoutPort *pp = PSY_INPOUT_PORT(self);

    PsyParallelPortClass *parallel_cls = PSY_PARALLEL_PORT_GET_CLASS(self);

    psy_parallel_port_close(self);

    switch (port_num) {
    case 0:
        pp->data_register = 0x378;
        break;
    case 1:
        pp->data_register = 0x278;
        break;
    case 2:
        pp->data_register = 0x3BC;
        break;
    default:
        g_set_error(error,
                    PSY_PARALLEL_PORT_ERROR,
                    PSY_PARALLEL_PORT_ERROR_OPEN,
                    "Unable to open port, chosen invalid number %d, choose "
                    "from [0,1,2]",
                    port_num);
        return;
    }
    pp->status_register  = pp->data_register + 1;
    pp->control_register = pp->data_register + 2;

    load_inpout(error);
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
    unload_inpout(); // close inpout.dll on close of last input/output device

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
psy_inpout_port_class_init(PsyInpoutPortClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);
    (void) obj_cls;

    PsyParallelPortClass *parallel_cls = PSY_PARALLEL_PORT_CLASS(cls);

    parallel_cls->open      = inpout_port_open;
    parallel_cls->close     = inpout_port_close;
    parallel_cls->write     = inpout_port_write;
    parallel_cls->write_pin = inpout_port_write_pin;
    parallel_cls->read      = inpout_port_read;
    parallel_cls->read_pin  = inpout_port_read_pin;
}
