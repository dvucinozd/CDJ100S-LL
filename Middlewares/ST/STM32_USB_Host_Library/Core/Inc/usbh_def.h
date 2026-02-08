/**
 ******************************************************************************
 * @file    usbh_def.h
 * @author  MCD Application Team
 * @brief   Definitions used in the USB host library
 ******************************************************************************
 */

#ifndef USBH_DEF_H
#define USBH_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_conf.h"

#ifndef NULL
#define NULL 0U
#endif

#ifndef FALSE
#define FALSE 0U
#endif

#ifndef TRUE
#define TRUE 1U
#endif

/* Macro definitions ---------------------------------------------------------*/
#define USBH_MAX_ERROR_COUNT 2U
#define USBH_DEVICE_ADDRESS_DEFAULT 0U
#define USBH_DEVICE_ADDRESS 1U

/* USB Host Speed */
#define USBH_SPEED_HIGH 0U
#define USBH_SPEED_FULL 1U
#define USBH_SPEED_LOW 2U

/* USB Host Channel Direction */
#define CH_H2D 0U
#define CH_D2H 1U

/* General USB defines */
#define USBH_DEV_RESET_TIMEOUT 100U
#define USBH_DEVICE_ADDRESS_MAX 127U
#define USBH_MAX_NB_DATA_PKT 0xFFFFU

/* USB Host PID tokens */
#define USBH_PID_SETUP 0U
#define USBH_PID_DATA 1U

/* USB Host Endpoint Type */
#define EP_TYPE_CTRL 0U
#define EP_TYPE_ISOC 1U
#define EP_TYPE_BULK 2U
#define EP_TYPE_INTR 3U

/* USB Control request direction */
#define USB_D2H 1U
#define USB_H2D 0U

/* USB Request Recipient */
#define USB_REQ_RECIPIENT_DEVICE 0x00U
#define USB_REQ_RECIPIENT_INTERFACE 0x01U
#define USB_REQ_RECIPIENT_ENDPOINT 0x02U
#define USB_REQ_RECIPIENT_OTHER 0x03U

/* USB Request Type */
#define USB_REQ_TYPE_STANDARD 0x00U
#define USB_REQ_TYPE_CLASS 0x20U
#define USB_REQ_TYPE_VENDOR 0x40U
#define USB_REQ_TYPE_RESERVED 0x60U

/* USB Standard Request Codes */
#define USB_REQ_GET_STATUS 0x00U
#define USB_REQ_CLEAR_FEATURE 0x01U
#define USB_REQ_SET_FEATURE 0x03U
#define USB_REQ_SET_ADDRESS 0x05U
#define USB_REQ_GET_DESCRIPTOR 0x06U
#define USB_REQ_SET_DESCRIPTOR 0x07U
#define USB_REQ_GET_CONFIGURATION 0x08U
#define USB_REQ_SET_CONFIGURATION 0x09U
#define USB_REQ_GET_INTERFACE 0x0AU
#define USB_REQ_SET_INTERFACE 0x0BU
#define USB_REQ_SYNCH_FRAME 0x0CU

/* USB Descriptor Types */
#define USB_DESC_TYPE_DEVICE 0x01U
#define USB_DESC_TYPE_CONFIGURATION 0x02U
#define USB_DESC_TYPE_STRING 0x03U
#define USB_DESC_TYPE_INTERFACE 0x04U
#define USB_DESC_TYPE_ENDPOINT 0x05U
#define USB_DESC_TYPE_DEVICE_QUALIFIER 0x06U
#define USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION 0x07U
#define USB_DESC_TYPE_INTERFACE_POWER 0x08U
#define USB_DESC_TYPE_HID 0x21U
#define USB_DESC_TYPE_HID_REPORT 0x22U

/* USB Endpoint Feature selectors */
#define FEATURE_SELECTOR_ENDPOINT 0x00U
#define FEATURE_SELECTOR_DEVICE 0x01U

/* Device connection speed */
typedef enum {
  USBH_SPEED_HIGH = 0,
  USBH_SPEED_FULL = 1,
  USBH_SPEED_LOW = 2,
} USBH_SpeedTypeDef;

/* USB URB State */
typedef enum {
  USBH_URB_IDLE = 0,
  USBH_URB_DONE,
  USBH_URB_NOTREADY,
  USBH_URB_NYET,
  USBH_URB_ERROR,
  USBH_URB_STALL
} USBH_URBStateTypeDef;

/* Library Status */
typedef enum {
  USBH_OK = 0,
  USBH_BUSY,
  USBH_FAIL,
  USBH_NOT_SUPPORTED,
  USBH_UNRECOVERED_ERROR,
  USBH_ERROR_SPEED_UNKNOWN,
} USBH_StatusTypeDef;

/* USB Host State Machine States */
typedef enum {
  HOST_IDLE = 0,
  HOST_DEV_WAIT_FOR_ATTACHMENT,
  HOST_DEV_ATTACHED,
  HOST_DEV_DISCONNECTED,
  HOST_DETECT_DEVICE_SPEED,
  HOST_ENUMERATION,
  HOST_CLASS_REQUEST,
  HOST_INPUT,
  HOST_SET_CONFIGURATION,
  HOST_SET_WAKEUP_FEATURE,
  HOST_CHECK_CLASS,
  HOST_CLASS,
  HOST_SUSPENDED,
  HOST_ABORT_STATE,
} HOST_StateTypeDef;

/* USB Host Enumeration State */
typedef enum {
  ENUM_IDLE = 0,
  ENUM_GET_FULL_DEV_DESC,
  ENUM_SET_ADDR,
  ENUM_GET_CFG_DESC,
  ENUM_GET_FULL_CFG_DESC,
  ENUM_GET_MFC_STRING_DESC,
  ENUM_GET_PRODUCT_STRING_DESC,
  ENUM_GET_SERIALNUM_STRING_DESC,
} ENUM_StateTypeDef;

/* USB Host Control State Machine */
typedef enum {
  CTRL_IDLE = 0,
  CTRL_SETUP,
  CTRL_SETUP_WAIT,
  CTRL_DATA_IN,
  CTRL_DATA_IN_WAIT,
  CTRL_DATA_OUT,
  CTRL_DATA_OUT_WAIT,
  CTRL_STATUS_IN,
  CTRL_STATUS_IN_WAIT,
  CTRL_STATUS_OUT,
  CTRL_STATUS_OUT_WAIT,
  CTRL_ERROR,
  CTRL_STALLED,
  CTRL_COMPLETE
} CTRL_StateTypeDef;

/* USB Control Request Structure */
typedef union {
  uint8_t d8[8];
  struct _bmRequest {
    uint8_t Recipient : 5;
    uint8_t Type : 2;
    uint8_t Direction : 1;
  } bm;
  struct _SetupPkt {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
  } b;
} USBH_SetupReqTypedef;

/* USB Descriptor Header */
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
} USBH_DescHeader_t;

/* USB Device Descriptor */
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t bcdUSB;
  uint8_t bDeviceClass;
  uint8_t bDeviceSubClass;
  uint8_t bDeviceProtocol;
  uint8_t bMaxPacketSize;
  uint16_t idVendor;
  uint16_t idProduct;
  uint16_t bcdDevice;
  uint8_t iManufacturer;
  uint8_t iProduct;
  uint8_t iSerialNumber;
  uint8_t bNumConfigurations;
} USBH_DevDescTypeDef;

/* USB Endpoint Descriptor */
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bEndpointAddress;
  uint8_t bmAttributes;
  uint16_t wMaxPacketSize;
  uint8_t bInterval;
} USBH_EpDescTypeDef;

/* USB Interface Descriptor */
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bNumEndpoints;
  uint8_t bInterfaceClass;
  uint8_t bInterfaceSubClass;
  uint8_t bInterfaceProtocol;
  uint8_t iInterface;
  USBH_EpDescTypeDef Ep_Desc[USBH_MAX_NUM_ENDPOINTS];
} USBH_InterfaceDescTypeDef;

/* USB Configuration Descriptor */
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t wTotalLength;
  uint8_t bNumInterfaces;
  uint8_t bConfigurationValue;
  uint8_t iConfiguration;
  uint8_t bmAttributes;
  uint8_t bMaxPower;
  USBH_InterfaceDescTypeDef Itf_Desc[USBH_MAX_NUM_INTERFACES];
} USBH_CfgDescTypeDef;

/* Current USB device info */
typedef struct {
  uint8_t address;
  uint8_t speed;
  uint8_t is_connected;
  uint8_t current_interface;
  USBH_DevDescTypeDef DevDesc;
  USBH_CfgDescTypeDef CfgDesc;
  uint8_t Data[USBH_MAX_DATA_BUFFER];
  uint8_t CfgDesc_Raw[USBH_MAX_SIZE_CONFIGURATION];
} USBH_DeviceTypeDef;

/* Forward declaration */
struct _USBH_HandleTypeDef;

/* USB Host Class Structure */
typedef struct {
  const char *Name;
  uint8_t ClassCode;
  USBH_StatusTypeDef (*Init)(struct _USBH_HandleTypeDef *phost);
  USBH_StatusTypeDef (*DeInit)(struct _USBH_HandleTypeDef *phost);
  USBH_StatusTypeDef (*Requests)(struct _USBH_HandleTypeDef *phost);
  USBH_StatusTypeDef (*BgndProcess)(struct _USBH_HandleTypeDef *phost);
  USBH_StatusTypeDef (*SOFProcess)(struct _USBH_HandleTypeDef *phost);
  void *pData;
} USBH_ClassTypeDef;

/* Control Request Structure */
typedef struct {
  uint8_t pipe_in;
  uint8_t pipe_out;
  uint8_t pipe_size;
  uint8_t *buff;
  uint16_t length;
  uint16_t timer;
  USBH_SetupReqTypedef setup;
  CTRL_StateTypeDef state;
  uint8_t errorcount;
} USBH_CtrlTypeDef;

/* USB Host Main Handle */
typedef struct _USBH_HandleTypeDef {
  HOST_StateTypeDef gState;
  ENUM_StateTypeDef EnumState;
  USBH_StatusTypeDef RequestState;
  USBH_CtrlTypeDef Control;
  USBH_DeviceTypeDef device;
  USBH_ClassTypeDef *pClass[USBH_MAX_NUM_SUPPORTED_CLASS];
  USBH_ClassTypeDef *pActiveClass;
  uint32_t ClassNumber;
  uint32_t Timer;
  uint8_t id;
  void *pData;
  void (*pUser)(struct _USBH_HandleTypeDef *phost, uint8_t id);
#if (USBH_KEEP_CFG_DESCRIPTOR == 1)
  uint8_t CfgDescRaw[USBH_MAX_SIZE_CONFIGURATION];
#endif
} USBH_HandleTypeDef;

/* USB Host User messages */
#define HOST_USER_SELECT_CONFIGURATION 0x01U
#define HOST_USER_CLASS_ACTIVE 0x02U
#define HOST_USER_CLASS_SELECTED 0x03U
#define HOST_USER_CONNECTION 0x04U
#define HOST_USER_DISCONNECTION 0x05U
#define HOST_USER_UNRECOVERED_ERROR 0x06U

#ifdef __cplusplus
}
#endif

#endif /* USBH_DEF_H */
