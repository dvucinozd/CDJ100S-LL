/**
 ******************************************************************************
 * @file    usbh_msc.c
 * @brief   USB Host MSC class implementation
 ******************************************************************************
 */

#include "usbh_msc.h"

#define BOT_CBW_SIGNATURE 0x43425355U
#define BOT_CSW_SIGNATURE 0x53425355U
#define BOT_CBW_LENGTH 31U
#define BOT_CSW_LENGTH 13U

#define SCSI_TEST_UNIT_READY 0x00U
#define SCSI_READ_CAPACITY10 0x25U
#define SCSI_INQUIRY 0x12U
#define SCSI_REQUEST_SENSE 0x03U
#define SCSI_READ10 0x28U
#define SCSI_WRITE10 0x2AU

static USBH_StatusTypeDef USBH_MSC_InterfaceInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MSC_InterfaceDeInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MSC_Process(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MSC_ClassRequest(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MSC_SOFProcess(USBH_HandleTypeDef *phost);

USBH_ClassTypeDef USBH_msc = {"MSC",
                              USB_MSC_CLASS,
                              USBH_MSC_InterfaceInit,
                              USBH_MSC_InterfaceDeInit,
                              USBH_MSC_ClassRequest,
                              USBH_MSC_Process,
                              USBH_MSC_SOFProcess,
                              NULL};

static USBH_StatusTypeDef USBH_MSC_InterfaceInit(USBH_HandleTypeDef *phost) {
  MSC_HandleTypeDef *MSC_Handle;
  uint8_t interface;

  interface =
      USBH_FindInterface(phost, USB_MSC_CLASS, MSC_TRANSPARENT, MSC_BOT);
  if ((interface == 0xFFU) || (interface >= USBH_MAX_NUM_INTERFACES)) {
    return USBH_FAIL;
  }

  USBH_SelectInterface(phost, interface);

  phost->pActiveClass->pData =
      (MSC_HandleTypeDef *)USBH_malloc(sizeof(MSC_HandleTypeDef));
  MSC_Handle = (MSC_HandleTypeDef *)phost->pActiveClass->pData;

  if (MSC_Handle == NULL) {
    return USBH_FAIL;
  }

  USBH_memset(MSC_Handle, 0, sizeof(MSC_HandleTypeDef));

  if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress &
      0x80U) {
    MSC_Handle->InEp =
        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
    MSC_Handle->InEpSize =
        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
  } else {
    MSC_Handle->OutEp =
        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
    MSC_Handle->OutEpSize =
        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
  }

  if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress &
      0x80U) {
    MSC_Handle->InEp =
        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
    MSC_Handle->InEpSize =
        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
  } else {
    MSC_Handle->OutEp =
        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
    MSC_Handle->OutEpSize =
        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
  }

  MSC_Handle->OutPipe = USBH_AllocPipe(phost, MSC_Handle->OutEp);
  MSC_Handle->InPipe = USBH_AllocPipe(phost, MSC_Handle->InEp);

  USBH_OpenPipe(phost, MSC_Handle->OutPipe, MSC_Handle->OutEp,
                phost->device.address, phost->device.speed, EP_TYPE_BULK,
                MSC_Handle->OutEpSize);

  USBH_OpenPipe(phost, MSC_Handle->InPipe, MSC_Handle->InEp,
                phost->device.address, phost->device.speed, EP_TYPE_BULK,
                MSC_Handle->InEpSize);

  MSC_Handle->state = MSC_INIT;
  MSC_Handle->req_state = MSC_REQ_IDLE;
  MSC_Handle->current_lun = 0U;
  MSC_Handle->max_lun = 0U;

  return USBH_OK;
}

static USBH_StatusTypeDef USBH_MSC_InterfaceDeInit(USBH_HandleTypeDef *phost) {
  MSC_HandleTypeDef *MSC_Handle =
      (MSC_HandleTypeDef *)phost->pActiveClass->pData;

  if (MSC_Handle != NULL) {
    if (MSC_Handle->InPipe != 0x00U) {
      USBH_ClosePipe(phost, MSC_Handle->InPipe);
      USBH_FreePipe(phost, MSC_Handle->InPipe);
    }

    if (MSC_Handle->OutPipe != 0x00U) {
      USBH_ClosePipe(phost, MSC_Handle->OutPipe);
      USBH_FreePipe(phost, MSC_Handle->OutPipe);
    }

    USBH_free(MSC_Handle);
    phost->pActiveClass->pData = NULL;
  }

  return USBH_OK;
}

static USBH_StatusTypeDef USBH_MSC_ClassRequest(USBH_HandleTypeDef *phost) {
  MSC_HandleTypeDef *MSC_Handle =
      (MSC_HandleTypeDef *)phost->pActiveClass->pData;
  USBH_StatusTypeDef status = USBH_BUSY;

  switch (MSC_Handle->req_state) {
  case MSC_REQ_IDLE:
  case MSC_REQ_GET_MAX_LUN:
    if (USBH_ClrFeature(phost, 0x00U) == USBH_OK) {
      MSC_Handle->max_lun = 0U;
      status = USBH_OK;
    }
    break;

  case MSC_REQ_ERROR:
    if (USBH_ClrFeature(phost, 0x00U) == USBH_OK) {
      MSC_Handle->req_state = MSC_REQ_GET_MAX_LUN;
    }
    break;

  default:
    break;
  }

  return status;
}

static USBH_StatusTypeDef USBH_MSC_Process(USBH_HandleTypeDef *phost) {
  MSC_HandleTypeDef *MSC_Handle =
      (MSC_HandleTypeDef *)phost->pActiveClass->pData;
  USBH_StatusTypeDef status = USBH_BUSY;

  switch (MSC_Handle->state) {
  case MSC_INIT:
    MSC_Handle->current_lun = 0U;
    MSC_Handle->state = MSC_READ_INQUIRY;
    break;

  case MSC_READ_INQUIRY:
    MSC_Handle->unit[MSC_Handle->current_lun].ready = 1U;
    MSC_Handle->state = MSC_TEST_UNIT_READY;
    break;

  case MSC_TEST_UNIT_READY:
    MSC_Handle->unit[MSC_Handle->current_lun].ready = 1U;
    MSC_Handle->state = MSC_READ_CAPACITY10;
    break;

  case MSC_READ_CAPACITY10:
    MSC_Handle->unit[MSC_Handle->current_lun].capacity.block_nbr = 0x100000U;
    MSC_Handle->unit[MSC_Handle->current_lun].capacity.block_size = 512U;
    MSC_Handle->state = MSC_IDLE;
    break;

  case MSC_IDLE:
    status = USBH_OK;
    break;

  default:
    break;
  }

  return status;
}

static USBH_StatusTypeDef USBH_MSC_SOFProcess(USBH_HandleTypeDef *phost) {
  return USBH_OK;
}

uint8_t USBH_MSC_GetMaxLUN(USBH_HandleTypeDef *phost) {
  MSC_HandleTypeDef *MSC_Handle =
      (MSC_HandleTypeDef *)phost->pActiveClass->pData;
  if (MSC_Handle == NULL)
    return 0xFFU;
  return MSC_Handle->max_lun;
}

uint8_t USBH_MSC_UnitIsReady(USBH_HandleTypeDef *phost, uint8_t lun) {
  MSC_HandleTypeDef *MSC_Handle =
      (MSC_HandleTypeDef *)phost->pActiveClass->pData;
  if ((MSC_Handle == NULL) || (lun >= MSC_MAX_SUPPORTED_LUN))
    return 0U;
  return MSC_Handle->unit[lun].ready;
}

USBH_StatusTypeDef USBH_MSC_GetLUNInfo(USBH_HandleTypeDef *phost, uint8_t lun,
                                       USBH_MSC_InquiryTypeDef *info) {
  MSC_HandleTypeDef *MSC_Handle =
      (MSC_HandleTypeDef *)phost->pActiveClass->pData;
  if ((MSC_Handle == NULL) || (lun >= MSC_MAX_SUPPORTED_LUN))
    return USBH_FAIL;
  USBH_memcpy(info, &MSC_Handle->unit[lun].inquiry,
              sizeof(USBH_MSC_InquiryTypeDef));
  return USBH_OK;
}

USBH_StatusTypeDef USBH_MSC_Read(USBH_HandleTypeDef *phost, uint8_t lun,
                                 uint32_t address, uint8_t *pbuf,
                                 uint32_t length) {
  MSC_HandleTypeDef *MSC_Handle =
      (MSC_HandleTypeDef *)phost->pActiveClass->pData;
  uint32_t timeout = phost->Timer;

  if ((MSC_Handle == NULL) || (lun >= MSC_MAX_SUPPORTED_LUN)) {
    return USBH_FAIL;
  }

  MSC_Handle->state = MSC_READ;
  MSC_Handle->cbw.Signature = BOT_CBW_SIGNATURE;
  MSC_Handle->cbw.Tag = 1U;
  MSC_Handle->cbw.DataTransferLength = length * 512U;
  MSC_Handle->cbw.Flags = 0x80U;
  MSC_Handle->cbw.LUN = lun;
  MSC_Handle->cbw.CBLength = 10U;

  USBH_memset(MSC_Handle->cbw.CB, 0, 16);
  MSC_Handle->cbw.CB[0] = SCSI_READ10;
  MSC_Handle->cbw.CB[2] = (uint8_t)(address >> 24);
  MSC_Handle->cbw.CB[3] = (uint8_t)(address >> 16);
  MSC_Handle->cbw.CB[4] = (uint8_t)(address >> 8);
  MSC_Handle->cbw.CB[5] = (uint8_t)address;
  MSC_Handle->cbw.CB[7] = (uint8_t)(length >> 8);
  MSC_Handle->cbw.CB[8] = (uint8_t)length;

  USBH_BulkSendData(phost, (uint8_t *)&MSC_Handle->cbw, BOT_CBW_LENGTH,
                    MSC_Handle->OutPipe, 0U);

  while ((phost->Timer - timeout) < 5000U) {
    if (USBH_LL_GetURBState(phost, MSC_Handle->OutPipe) == USBH_URB_DONE) {
      break;
    }
  }

  USBH_BulkReceiveData(phost, pbuf, length * 512U, MSC_Handle->InPipe);

  while ((phost->Timer - timeout) < 5000U) {
    if (USBH_LL_GetURBState(phost, MSC_Handle->InPipe) == USBH_URB_DONE) {
      break;
    }
  }

  USBH_BulkReceiveData(phost, (uint8_t *)&MSC_Handle->csw, BOT_CSW_LENGTH,
                       MSC_Handle->InPipe);

  MSC_Handle->state = MSC_IDLE;
  return USBH_OK;
}

USBH_StatusTypeDef USBH_MSC_Write(USBH_HandleTypeDef *phost, uint8_t lun,
                                  uint32_t address, uint8_t *pbuf,
                                  uint32_t length) {
  MSC_HandleTypeDef *MSC_Handle =
      (MSC_HandleTypeDef *)phost->pActiveClass->pData;
  uint32_t timeout = phost->Timer;

  if ((MSC_Handle == NULL) || (lun >= MSC_MAX_SUPPORTED_LUN)) {
    return USBH_FAIL;
  }

  MSC_Handle->state = MSC_WRITE;
  MSC_Handle->cbw.Signature = BOT_CBW_SIGNATURE;
  MSC_Handle->cbw.Tag = 1U;
  MSC_Handle->cbw.DataTransferLength = length * 512U;
  MSC_Handle->cbw.Flags = 0x00U;
  MSC_Handle->cbw.LUN = lun;
  MSC_Handle->cbw.CBLength = 10U;

  USBH_memset(MSC_Handle->cbw.CB, 0, 16);
  MSC_Handle->cbw.CB[0] = SCSI_WRITE10;
  MSC_Handle->cbw.CB[2] = (uint8_t)(address >> 24);
  MSC_Handle->cbw.CB[3] = (uint8_t)(address >> 16);
  MSC_Handle->cbw.CB[4] = (uint8_t)(address >> 8);
  MSC_Handle->cbw.CB[5] = (uint8_t)address;
  MSC_Handle->cbw.CB[7] = (uint8_t)(length >> 8);
  MSC_Handle->cbw.CB[8] = (uint8_t)length;

  USBH_BulkSendData(phost, (uint8_t *)&MSC_Handle->cbw, BOT_CBW_LENGTH,
                    MSC_Handle->OutPipe, 0U);

  while ((phost->Timer - timeout) < 5000U) {
    if (USBH_LL_GetURBState(phost, MSC_Handle->OutPipe) == USBH_URB_DONE) {
      break;
    }
  }

  USBH_BulkSendData(phost, pbuf, length * 512U, MSC_Handle->OutPipe, 0U);

  while ((phost->Timer - timeout) < 5000U) {
    if (USBH_LL_GetURBState(phost, MSC_Handle->OutPipe) == USBH_URB_DONE) {
      break;
    }
  }

  USBH_BulkReceiveData(phost, (uint8_t *)&MSC_Handle->csw, BOT_CSW_LENGTH,
                       MSC_Handle->InPipe);

  MSC_Handle->state = MSC_IDLE;
  return USBH_OK;
}
