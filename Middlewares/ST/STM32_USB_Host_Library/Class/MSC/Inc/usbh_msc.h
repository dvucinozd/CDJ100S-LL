/**
 ******************************************************************************
 * @file    usbh_msc.h
 * @brief   USB Host MSC class header
 ******************************************************************************
 */

#ifndef __USBH_MSC_H
#define __USBH_MSC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbh_core.h"

#define USB_MSC_CLASS 0x08U
#define MSC_TRANSPARENT 0x06U
#define MSC_BOT 0x50U

#define MSC_MAX_SUPPORTED_LUN 2U

typedef enum {
  MSC_INIT = 0,
  MSC_IDLE,
  MSC_TEST_UNIT_READY,
  MSC_READ_CAPACITY10,
  MSC_READ_INQUIRY,
  MSC_REQUEST_SENSE,
  MSC_READ,
  MSC_WRITE,
  MSC_UNRECOVERED_ERROR,
  MSC_PERIODIC_CHECK,
} MSC_StateTypeDef;

typedef enum {
  MSC_OK,
  MSC_NOT_READY,
  MSC_ERROR,
} MSC_ErrorTypeDef;

typedef enum {
  MSC_REQ_IDLE = 0,
  MSC_REQ_RESET,
  MSC_REQ_GET_MAX_LUN,
  MSC_REQ_ERROR,
} MSC_ReqStateTypeDef;

typedef struct {
  uint32_t Signature;
  uint32_t Tag;
  uint32_t DataTransferLength;
  uint8_t Flags;
  uint8_t LUN;
  uint8_t CBLength;
  uint8_t CB[16];
} USBH_MSC_BOT_CBWTypeDef;

typedef struct {
  uint32_t Signature;
  uint32_t Tag;
  uint32_t DataResidue;
  uint8_t Status;
} USBH_MSC_BOT_CSWTypeDef;

typedef struct {
  uint8_t DeviceType;
  uint8_t PeripheralQualifier;
  uint8_t RemovableMedia;
  uint8_t Reserved[5];
  uint8_t VendorID[8];
  uint8_t ProductID[16];
  uint8_t ProductRev[4];
} USBH_MSC_InquiryTypeDef;

typedef struct {
  uint32_t block_nbr;
  uint16_t block_size;
} USBH_MSC_CapacityTypeDef;

typedef struct {
  MSC_StateTypeDef state;
  MSC_ErrorTypeDef error;
  MSC_ReqStateTypeDef req_state;
  uint8_t max_lun;
  uint8_t current_lun;
  uint8_t InPipe;
  uint8_t OutPipe;
  uint8_t InEp;
  uint8_t OutEp;
  uint16_t InEpSize;
  uint16_t OutEpSize;
  uint32_t timer;
  USBH_MSC_BOT_CBWTypeDef cbw;
  USBH_MSC_BOT_CSWTypeDef csw;
  uint8_t pbuf[512];
  struct {
    USBH_MSC_InquiryTypeDef inquiry;
    USBH_MSC_CapacityTypeDef capacity;
    uint8_t ready;
    uint8_t changed;
    uint8_t prev_ready;
  } unit[MSC_MAX_SUPPORTED_LUN];
} MSC_HandleTypeDef;

extern USBH_ClassTypeDef USBH_msc;
#define USBH_MSC_CLASS &USBH_msc

/* Exported functions */
uint8_t USBH_MSC_GetMaxLUN(USBH_HandleTypeDef *phost);
uint8_t USBH_MSC_UnitIsReady(USBH_HandleTypeDef *phost, uint8_t lun);
USBH_StatusTypeDef USBH_MSC_GetLUNInfo(USBH_HandleTypeDef *phost, uint8_t lun,
                                       USBH_MSC_InquiryTypeDef *info);
USBH_StatusTypeDef USBH_MSC_Read(USBH_HandleTypeDef *phost, uint8_t lun,
                                 uint32_t address, uint8_t *pbuf,
                                 uint32_t length);
USBH_StatusTypeDef USBH_MSC_Write(USBH_HandleTypeDef *phost, uint8_t lun,
                                  uint32_t address, uint8_t *pbuf,
                                  uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* __USBH_MSC_H */
