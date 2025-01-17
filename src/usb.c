#include "tusb.h"
#include "device/usbd.h"
#include "device/usbd_pvt.h"
#include "canokey-core/interfaces/USB/core/inc/usbd_def.h"
#include <admin.h>
#include <string.h>
#include <usb_device.h>
#include "hardware/irq.h"

#ifndef CFG_TUD_ENDPOINT_MAX
#define CFG_TUD_ENDPOINT_MAX 8
#endif

#define USB_RHPORT_NUM              0
#define USB_MAX_PACKET_SIZE         64
#define USB_EP_ATTR_XFER_MASK       0x03
#define USB_EP_ATTR_SYNC_MASK       0x03
#define USB_EP_ATTR_USAGE_MASK      0x03
#define USB_EP_ATTR_SYNC_SHIFT      2
#define USB_EP_ATTR_USAGE_SHIFT     4
#define USB_EP_INTERVAL_DEFAULT     0
#define USB_INITIAL_INTERFACE_NUM   0
#define USB_INITIAL_ENDPOINT_NUM    1

typedef enum {
    USBD_EP_FREE = 0,
    USBD_EP_BUSY,
    USBD_EP_STALL
} USBD_EP_StatusTypeDef;

static tusb_desc_endpoint_t ep_desc;

USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev) {
    if (pdev == NULL) return USBD_FAIL;

    pdev->dev_state = USBD_STATE_DEFAULT;
    pdev->dev_old_state = USBD_STATE_DEFAULT;
    pdev->dev_config = 0;
    pdev->dev_config_status = 0;
    pdev->dev_remote_wakeup = 0;
    pdev->ep0_state = USBD_EP0_IDLE;
    pdev->ep0_data_len = 0;
    pdev->dev_speed = USBD_SPEED_FULL;

    for (uint8_t i = 0; i < USBD_EP_SIZE; i++) {
        pdev->ep_in[i].status = USBD_EP_FREE;
        pdev->ep_in[i].total_length = 0;
        pdev->ep_in[i].rem_length = 0;
        pdev->ep_in[i].maxpacket = 0;

        pdev->ep_out[i].status = USBD_EP_FREE;
        pdev->ep_out[i].total_length = 0;
        pdev->ep_out[i].rem_length = 0;
        pdev->ep_out[i].maxpacket = 0;
    }

    return tusb_init() ? USBD_OK : USBD_FAIL;
}

USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev) {
    for (uint8_t i = 0; i < USBD_EP_SIZE; i++) {
        if (pdev->ep_in[i].status != USBD_EP_FREE) {
            usbd_edpt_close(USB_RHPORT_NUM, i | TUSB_DIR_IN_MASK);
            pdev->ep_in[i].status = USBD_EP_FREE;
        }
        if (pdev->ep_out[i].status != USBD_EP_FREE) {
            usbd_edpt_close(USB_RHPORT_NUM, i);
            pdev->ep_out[i].status = USBD_EP_FREE;
        }
    }
    pdev->dev_state = USBD_STATE_DEFAULT;
    return USBD_OK;
}

void USBD_LL_Init_Done(USBD_HandleTypeDef *pdev) {
    if (pdev == NULL) {
        return;
    }

    pdev->dev_state = USBD_STATE_CONFIGURED;
    pdev->dev_config = 1;
    pdev->dev_config_status = 1;
    
    // Zresetuj endpoint kontrolny
    pdev->ep0_state = USBD_EP0_IDLE;
    pdev->ep0_data_len = 0;
    
    for (uint8_t i = 0; i < USBD_EP_SIZE; i++) {
        if (pdev->ep_in[i].status == USBD_EP_BUSY) {
            continue;
        }
        pdev->ep_in[i].status = USBD_EP_FREE;
        
        if (pdev->ep_out[i].status == USBD_EP_BUSY) {
            continue;
        }
        pdev->ep_out[i].status = USBD_EP_FREE;
    }
    
    irq_set_enabled(USBCTRL_IRQ, true);
    
    // tud_connect();
    
    pdev->dev_old_state = pdev->dev_state;
}

USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev) {
    pdev->dev_state = USBD_STATE_DEFAULT;
    tud_connect();
    return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev) {
    pdev->dev_state = USBD_STATE_DEFAULT;
    tud_disconnect();
    return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps) {
    uint8_t ep_idx = tu_edpt_number(ep_addr);
    bool is_in = tu_edpt_dir(ep_addr) == TUSB_DIR_IN;

    ep_desc.bLength = sizeof(tusb_desc_endpoint_t);
    ep_desc.bDescriptorType = TUSB_DESC_ENDPOINT;
    ep_desc.bEndpointAddress = ep_addr;
    ep_desc.bmAttributes.xfer = ep_type & USB_EP_ATTR_XFER_MASK;
    ep_desc.bmAttributes.sync = (ep_type >> USB_EP_ATTR_SYNC_SHIFT) & USB_EP_ATTR_SYNC_MASK;
    ep_desc.bmAttributes.usage = (ep_type >> USB_EP_ATTR_USAGE_SHIFT) & USB_EP_ATTR_USAGE_MASK;
    ep_desc.wMaxPacketSize = ep_mps;
    ep_desc.bInterval = USB_EP_INTERVAL_DEFAULT;

    if (usbd_edpt_open(USB_RHPORT_NUM, &ep_desc)) {
        if (is_in) {
            pdev->ep_in[ep_idx].status = USBD_EP_BUSY;
            pdev->ep_in[ep_idx].maxpacket = ep_mps;
            pdev->ep_in[ep_idx].total_length = 0;
            pdev->ep_in[ep_idx].rem_length = 0;
        } else {
            pdev->ep_out[ep_idx].status = USBD_EP_BUSY;
            pdev->ep_out[ep_idx].maxpacket = ep_mps;
            pdev->ep_out[ep_idx].total_length = 0;
            pdev->ep_out[ep_idx].rem_length = 0;
        }
        return USBD_OK;
    }
    return USBD_FAIL;
}

USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
    uint8_t ep_idx = tu_edpt_number(ep_addr);
    bool is_in = tu_edpt_dir(ep_addr) == TUSB_DIR_IN;

    usbd_edpt_close(USB_RHPORT_NUM, ep_addr);
    if (is_in) {
        pdev->ep_in[ep_idx].status = USBD_EP_FREE;
    } else {
        pdev->ep_out[ep_idx].status = USBD_EP_FREE;
    }
    return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
    uint8_t ep_idx = tu_edpt_number(ep_addr);
    bool is_in = tu_edpt_dir(ep_addr) == TUSB_DIR_IN;

    usbd_edpt_stall(USB_RHPORT_NUM, ep_addr);
    if (is_in) {
        pdev->ep_in[ep_idx].status = USBD_EP_STALL;
    } else {
        pdev->ep_out[ep_idx].status = USBD_EP_STALL;
    }
    return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
    uint8_t ep_idx = tu_edpt_number(ep_addr);
    bool is_in = tu_edpt_dir(ep_addr) == TUSB_DIR_IN;

    usbd_edpt_clear_stall(USB_RHPORT_NUM, ep_addr);
    if (is_in) {
        pdev->ep_in[ep_idx].status = USBD_EP_BUSY;
    } else {
        pdev->ep_out[ep_idx].status = USBD_EP_BUSY;
    }
    return USBD_OK;
}

uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
    uint8_t ep_idx = tu_edpt_number(ep_addr);
    bool is_in = tu_edpt_dir(ep_addr) == TUSB_DIR_IN;
    
    return (is_in ? pdev->ep_in[ep_idx].status : pdev->ep_out[ep_idx].status) == USBD_EP_STALL;
}

USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr) {
    if (dev_addr != 0) {
        pdev->dev_state = USBD_STATE_ADDRESSED;
    } else {
        pdev->dev_state = USBD_STATE_DEFAULT;
    }
    return USBD_OK;
}

static USBD_StatusTypeDef usbd_ll_xfer(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size) {
    uint8_t ep_idx = tu_edpt_number(ep_addr);
    bool is_in = tu_edpt_dir(ep_addr) == TUSB_DIR_IN;

    if (usbd_edpt_xfer(USB_RHPORT_NUM, ep_addr, pbuf, size)) {
        if (is_in) {
            pdev->ep_in[ep_idx].status = USBD_EP_BUSY;
            pdev->ep_in[ep_idx].total_length = size;
            pdev->ep_in[ep_idx].rem_length = size;
        } else {
            pdev->ep_out[ep_idx].status = USBD_EP_BUSY;
            pdev->ep_out[ep_idx].total_length = size;
            pdev->ep_out[ep_idx].rem_length = size;
        }
        return USBD_OK;
    }
    return USBD_FAIL;
}

USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size) {
    return usbd_ll_xfer(pdev, ep_addr, pbuf, size);
}

USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size) {
    return usbd_ll_xfer(pdev, ep_addr, pbuf, size);
}

uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
    uint8_t ep_idx = tu_edpt_number(ep_addr);
    
    if (ep_idx == 0) {
        return pdev->ep0_data_len;
    }
    
    return pdev->ep_out[ep_idx].total_length - pdev->ep_out[ep_idx].rem_length;
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
