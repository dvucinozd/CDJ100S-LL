/**
 ******************************************************************************
 * @file    usbh_ctlreq.c
 * @brief   USB Host control request handling
 ******************************************************************************
 */

#include "usbh_ctlreq.h"

static USBH_StatusTypeDef USBH_HandleControl(USBH_HandleTypeDef *phost);

USBH_StatusTypeDef USBH_CtlReq(USBH_HandleTypeDef *phost, uint8_t *buff,
                               uint16_t length) {
  USBH_StatusTypeDef status;
  status = USBH_HandleControl(phost);
  return status;
}

static USBH_StatusTypeDef USBH_HandleControl(USBH_HandleTypeDef *phost) {
  USBH_URBStateTypeDef URB_Status;
  USBH_StatusTypeDef status = USBH_BUSY;

  switch (phost->Control.state) {
  case CTRL_SETUP:
    USBH_CtlSendSetup(phost, (uint8_t *)&phost->Control.setup,
                      phost->Control.pipe_out);
    phost->Control.state = CTRL_SETUP_WAIT;
    break;

  case CTRL_SETUP_WAIT:
    URB_Status = USBH_LL_GetURBState(phost, phost->Control.pipe_out);
    if (URB_Status == USBH_URB_DONE) {
      if (phost->Control.setup.b.wLength != 0U) {
        if ((phost->Control.setup.b.bmRequestType & 0x80U) != 0U) {
          phost->Control.state = CTRL_DATA_IN;
        } else {
          phost->Control.state = CTRL_DATA_OUT;
        }
      } else {
        phost->Control.state = CTRL_STATUS_IN;
      }
    } else if (URB_Status == USBH_URB_ERROR) {
      phost->Control.state = CTRL_ERROR;
    }
    break;

  case CTRL_DATA_IN:
    USBH_CtlReceiveData(phost, phost->Control.buff, phost->Control.length,
                        phost->Control.pipe_in);
    phost->Control.state = CTRL_DATA_IN_WAIT;
    break;

  case CTRL_DATA_IN_WAIT:
    URB_Status = USBH_LL_GetURBState(phost, phost->Control.pipe_in);
    if (URB_Status == USBH_URB_DONE) {
      phost->Control.state = CTRL_STATUS_OUT;
    } else if (URB_Status == USBH_URB_ERROR) {
      phost->Control.state = CTRL_ERROR;
    }
    break;

  case CTRL_DATA_OUT:
    USBH_CtlSendData(phost, phost->Control.buff, phost->Control.length,
                     phost->Control.pipe_out, 0U);
    phost->Control.state = CTRL_DATA_OUT_WAIT;
    break;

  case CTRL_DATA_OUT_WAIT:
    URB_Status = USBH_LL_GetURBState(phost, phost->Control.pipe_out);
    if (URB_Status == USBH_URB_DONE) {
      phost->Control.state = CTRL_STATUS_IN;
    } else if (URB_Status == USBH_URB_ERROR) {
      phost->Control.state = CTRL_ERROR;
    }
    break;

  case CTRL_STATUS_IN:
    USBH_CtlReceiveData(phost, NULL, 0U, phost->Control.pipe_in);
    phost->Control.state = CTRL_STATUS_IN_WAIT;
    break;

  case CTRL_STATUS_IN_WAIT:
    URB_Status = USBH_LL_GetURBState(phost, phost->Control.pipe_in);
    if (URB_Status == USBH_URB_DONE) {
      phost->Control.state = CTRL_COMPLETE;
      status = USBH_OK;
    }
    break;

  case CTRL_STATUS_OUT:
    USBH_CtlSendData(phost, NULL, 0U, phost->Control.pipe_out, 0U);
    phost->Control.state = CTRL_STATUS_OUT_WAIT;
    break;

  case CTRL_STATUS_OUT_WAIT:
    URB_Status = USBH_LL_GetURBState(phost, phost->Control.pipe_out);
    if (URB_Status == USBH_URB_DONE) {
      phost->Control.state = CTRL_COMPLETE;
      status = USBH_OK;
    }
    break;

  case CTRL_ERROR:
    if (++phost->Control.errorcount <= USBH_MAX_ERROR_COUNT) {
      phost->Control.state = CTRL_SETUP;
    } else {
      status = USBH_FAIL;
    }
    break;

  default:
    break;
  }
  return status;
}

USBH_StatusTypeDef USBH_GetDescriptor(USBH_HandleTypeDef *phost,
                                      uint8_t req_type, uint16_t value_idx,
                                      uint8_t *buff, uint16_t length) {
  if (phost->RequestState == USBH_OK) {
    phost->Control.setup.b.bmRequestType = USB_D2H | req_type;
    phost->Control.setup.b.bRequest = USB_REQ_GET_DESCRIPTOR;
    phost->Control.setup.b.wValue = value_idx;
    phost->Control.setup.b.wIndex = 0U;
    phost->Control.setup.b.wLength = length;
    phost->Control.state = CTRL_SETUP;
    phost->Control.buff = buff;
    phost->Control.length = length;
  }
  return USBH_CtlReq(phost, buff, length);
}

USBH_StatusTypeDef USBH_GetDevDesc(USBH_HandleTypeDef *phost, uint8_t length) {
  USBH_StatusTypeDef status;
  status = USBH_GetDescriptor(
      phost, USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD,
      USB_DESC_TYPE_DEVICE << 8, phost->device.Data, length);
  if (status == USBH_OK) {
    USBH_ParseDevDesc(&phost->device.DevDesc, phost->device.Data, length);
  }
  return status;
}

USBH_StatusTypeDef USBH_GetCfgDesc(USBH_HandleTypeDef *phost, uint16_t length) {
  USBH_StatusTypeDef status;
  uint8_t *pData = phost->device.CfgDesc_Raw;
  status = USBH_GetDescriptor(phost,
                              USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD,
                              USB_DESC_TYPE_CONFIGURATION << 8, pData, length);
  if (status == USBH_OK) {
    USBH_ParseCfgDesc(&phost->device.CfgDesc, pData, length);
  }
  return status;
}

USBH_StatusTypeDef USBH_GetString(USBH_HandleTypeDef *phost,
                                  uint8_t string_index, uint8_t *buff,
                                  uint16_t length) {
  return USBH_GetDescriptor(
      phost, USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD,
      (USB_DESC_TYPE_STRING << 8) | string_index, buff, length);
}

USBH_StatusTypeDef USBH_SetCfg(USBH_HandleTypeDef *phost, uint16_t cfg_idx) {
  if (phost->RequestState == USBH_OK) {
    phost->Control.setup.b.bmRequestType =
        USB_H2D | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD;
    phost->Control.setup.b.bRequest = USB_REQ_SET_CONFIGURATION;
    phost->Control.setup.b.wValue = cfg_idx;
    phost->Control.setup.b.wIndex = 0U;
    phost->Control.setup.b.wLength = 0U;
    phost->Control.state = CTRL_SETUP;
    phost->Control.buff = NULL;
    phost->Control.length = 0U;
  }
  return USBH_CtlReq(phost, NULL, 0U);
}

USBH_StatusTypeDef USBH_SetAddress(USBH_HandleTypeDef *phost,
                                   uint8_t DeviceAddress) {
  if (phost->RequestState == USBH_OK) {
    phost->Control.setup.b.bmRequestType =
        USB_H2D | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD;
    phost->Control.setup.b.bRequest = USB_REQ_SET_ADDRESS;
    phost->Control.setup.b.wValue = (uint16_t)DeviceAddress;
    phost->Control.setup.b.wIndex = 0U;
    phost->Control.setup.b.wLength = 0U;
    phost->Control.state = CTRL_SETUP;
  }
  return USBH_CtlReq(phost, NULL, 0U);
}

USBH_StatusTypeDef USBH_SetInterface(USBH_HandleTypeDef *phost, uint8_t iface,
                                     uint8_t altSetting) {
  if (phost->RequestState == USBH_OK) {
    phost->Control.setup.b.bmRequestType =
        USB_H2D | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_STANDARD;
    phost->Control.setup.b.bRequest = USB_REQ_SET_INTERFACE;
    phost->Control.setup.b.wValue = (uint16_t)altSetting;
    phost->Control.setup.b.wIndex = (uint16_t)iface;
    phost->Control.setup.b.wLength = 0U;
    phost->Control.state = CTRL_SETUP;
  }
  return USBH_CtlReq(phost, NULL, 0U);
}

USBH_StatusTypeDef USBH_SetFeature(USBH_HandleTypeDef *phost, uint8_t feature) {
  if (phost->RequestState == USBH_OK) {
    phost->Control.setup.b.bmRequestType =
        USB_H2D | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD;
    phost->Control.setup.b.bRequest = USB_REQ_SET_FEATURE;
    phost->Control.setup.b.wValue = feature;
    phost->Control.setup.b.wIndex = 0U;
    phost->Control.setup.b.wLength = 0U;
    phost->Control.state = CTRL_SETUP;
  }
  return USBH_CtlReq(phost, NULL, 0U);
}

USBH_StatusTypeDef USBH_ClrFeature(USBH_HandleTypeDef *phost, uint8_t ep_num) {
  if (phost->RequestState == USBH_OK) {
    phost->Control.setup.b.bmRequestType =
        USB_H2D | USB_REQ_RECIPIENT_ENDPOINT | USB_REQ_TYPE_STANDARD;
    phost->Control.setup.b.bRequest = USB_REQ_CLEAR_FEATURE;
    phost->Control.setup.b.wValue = FEATURE_SELECTOR_ENDPOINT;
    phost->Control.setup.b.wIndex = (uint16_t)ep_num;
    phost->Control.setup.b.wLength = 0U;
    phost->Control.state = CTRL_SETUP;
  }
  return USBH_CtlReq(phost, NULL, 0U);
}

void USBH_ParseDevDesc(USBH_DevDescTypeDef *dev_desc, uint8_t *buf,
                       uint16_t length) {
  dev_desc->bLength = buf[0];
  dev_desc->bDescriptorType = buf[1];
  dev_desc->bcdUSB = buf[2] | ((uint16_t)buf[3] << 8);
  dev_desc->bDeviceClass = buf[4];
  dev_desc->bDeviceSubClass = buf[5];
  dev_desc->bDeviceProtocol = buf[6];
  dev_desc->bMaxPacketSize = buf[7];
  if (length > 8U) {
    dev_desc->idVendor = buf[8] | ((uint16_t)buf[9] << 8);
    dev_desc->idProduct = buf[10] | ((uint16_t)buf[11] << 8);
    dev_desc->bcdDevice = buf[12] | ((uint16_t)buf[13] << 8);
    dev_desc->iManufacturer = buf[14];
    dev_desc->iProduct = buf[15];
    dev_desc->iSerialNumber = buf[16];
    dev_desc->bNumConfigurations = buf[17];
  }
}

void USBH_ParseCfgDesc(USBH_CfgDescTypeDef *cfg_desc, uint8_t *buf,
                       uint16_t length) {
  USBH_InterfaceDescTypeDef *pif;
  USBH_EpDescTypeDef *pep;
  USBH_DescHeader_t *pdesc;
  uint16_t ptr = 0U;
  uint8_t if_ix = 0U;
  uint8_t ep_ix = 0U;

  cfg_desc->bLength = buf[0];
  cfg_desc->bDescriptorType = buf[1];
  cfg_desc->wTotalLength = buf[2] | ((uint16_t)buf[3] << 8);
  cfg_desc->bNumInterfaces = buf[4];
  cfg_desc->bConfigurationValue = buf[5];
  cfg_desc->iConfiguration = buf[6];
  cfg_desc->bmAttributes = buf[7];
  cfg_desc->bMaxPower = buf[8];

  if (length > 9U) {
    ptr = 9U;
    pif = &cfg_desc->Itf_Desc[0];

    while ((if_ix < USBH_MAX_NUM_INTERFACES) && (ptr < length)) {
      pdesc = USBH_GetNextDesc((uint8_t *)buf, &ptr);
      if (pdesc->bDescriptorType == USB_DESC_TYPE_INTERFACE) {
        USBH_ParseInterfaceDesc(pif, (uint8_t *)pdesc);
        ep_ix = 0U;
        pep = &pif->Ep_Desc[0];
        while ((ep_ix < pif->bNumEndpoints) && (ptr < length)) {
          pdesc = USBH_GetNextDesc((uint8_t *)buf, &ptr);
          if (pdesc->bDescriptorType == USB_DESC_TYPE_ENDPOINT) {
            USBH_ParseEPDesc(pep, (uint8_t *)pdesc);
            pep++;
            ep_ix++;
          }
        }
        pif++;
        if_ix++;
      }
    }
  }
}

void USBH_ParseInterfaceDesc(USBH_InterfaceDescTypeDef *if_desc, uint8_t *buf) {
  if_desc->bLength = buf[0];
  if_desc->bDescriptorType = buf[1];
  if_desc->bInterfaceNumber = buf[2];
  if_desc->bAlternateSetting = buf[3];
  if_desc->bNumEndpoints = buf[4];
  if_desc->bInterfaceClass = buf[5];
  if_desc->bInterfaceSubClass = buf[6];
  if_desc->bInterfaceProtocol = buf[7];
  if_desc->iInterface = buf[8];
}

void USBH_ParseEPDesc(USBH_EpDescTypeDef *ep_desc, uint8_t *buf) {
  ep_desc->bLength = buf[0];
  ep_desc->bDescriptorType = buf[1];
  ep_desc->bEndpointAddress = buf[2];
  ep_desc->bmAttributes = buf[3];
  ep_desc->wMaxPacketSize = buf[4] | ((uint16_t)buf[5] << 8);
  ep_desc->bInterval = buf[6];
}

USBH_DescHeader_t *USBH_GetNextDesc(uint8_t *pbuf, uint16_t *ptr) {
  USBH_DescHeader_t *pnext;
  *ptr += ((USBH_DescHeader_t *)pbuf)->bLength;
  pnext = (USBH_DescHeader_t *)((uint8_t *)pbuf +
                                ((USBH_DescHeader_t *)pbuf)->bLength);
  return pnext;
}
