#include <admin.h>

#include <string.h>
#include <usb_device.h>
#include "tusb.h"

#define USB_INITIAL_INTERFACE_NUM   0
#define USB_INITIAL_ENDPOINT_NUM    1

void USBD_IRQHandler(void) {
    tud_int_handler(0);
}

void usb_resources_alloc(void) {
    uint8_t iface = USB_INITIAL_INTERFACE_NUM;
    uint8_t ep = USB_INITIAL_ENDPOINT_NUM;

    memset(&IFACE_TABLE, 0xFF, sizeof(IFACE_TABLE));
    memset(&EP_TABLE, 0xFF, sizeof(EP_TABLE));

    EP_TABLE.ctap_hid = ep++;
    IFACE_TABLE.ctap_hid = iface++;
    IFACE_TABLE.webusb = iface++;
    EP_TABLE.ccid = ep++;
    IFACE_TABLE.ccid = iface++;
    EP_TABLE.kbd_hid = ep;
    IFACE_TABLE.kbd_hid = iface;
}

uint8_t const* tud_descriptor_device_qualifier_cb(void) {
    static uint8_t qualifier[] = {
        0x0A,           // bLength
        0x06,           // bDescriptorType (Device Qualifier)
        0x10, 0x02,     // bcdUSB 2.1
        0x00,           // bDeviceClass (Use class information in the Interface Descriptors)
        0x00,           // bDeviceSubClass
        0x00,           // bDeviceProtocol
        64,             // bMaxPacketSize0
        0x01,           // bNumConfigurations
        0x00            // bReserved
    };
    return qualifier;
}
