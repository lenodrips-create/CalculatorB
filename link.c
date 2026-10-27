#include <usbdrvce.h>
#include <srldrvce.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "link.h"

#define BAUD_RATE 115200
#define RX_MAX    320

static srl_device_t srl;
static bool has_srl_device;
static uint8_t srl_buf[512];
static char rx_buf[RX_MAX + 1];
static size_t rx_len;

static usb_error_t handle_usb_event(usb_event_t event, void *event_data,
                                    usb_callback_data_t *callback_data)
{
    usb_error_t err = srl_UsbEventCallback(event, event_data, callback_data);
    if (err != USB_SUCCESS) {
        return err;
    }

    // The calculator is the USB host; the Wi-Fi board is the device
    if (event == USB_DEVICE_CONNECTED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE)) {
        usb_ResetDevice(event_data);
    }

    if (event == USB_DEVICE_ENABLED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE) && !has_srl_device) {
        if (srl_Open(&srl, event_data, srl_buf, sizeof srl_buf, SRL_INTERFACE_ANY, BAUD_RATE) == SRL_SUCCESS) {
            has_srl_device = true;
            rx_len = 0;
        }
    }

    if (event == USB_DEVICE_DISCONNECTED_EVENT && has_srl_device && event_data == srl.dev) {
        srl_Close(&srl);
        has_srl_device = false;
    }

    return USB_SUCCESS;
}

void link_init(void)
{
    usb_Init(handle_usb_event, NULL, srl_GetCDCStandardDescriptors(), USB_DEFAULT_INIT_FLAGS);
}

void link_end(void)
{
    usb_Cleanup();
}

void link_update(void)
{
    usb_HandleEvents();
}

bool link_connected(void)
{
    return has_srl_device;
}

bool link_send_line(const char *s)
{
    if (!has_srl_device) {
        return false;
    }
    srl_Write(&srl, s, strlen(s));
    srl_Write(&srl, "\n", 1);
    return true;
}

const char *link_read_line(void)
{
    char c;

    if (!has_srl_device) {
        return NULL;
    }
    while (srl_Read(&srl, &c, 1) == 1) {
        if (c == '\n') {
            rx_buf[rx_len] = '\0';
            rx_len = 0;
            return rx_buf;
        }
        if (c != '\r' && rx_len < RX_MAX) {
            rx_buf[rx_len++] = c;
        }
    }
    return NULL;
}
