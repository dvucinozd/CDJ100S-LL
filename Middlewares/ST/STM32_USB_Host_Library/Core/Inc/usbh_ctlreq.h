/**
 ******************************************************************************
 * @file    usbh_ctlreq.h
 * @author  MCD Application Team
 * @brief   Header file for usbh_ctlreq.c
 ******************************************************************************
 */

#ifndef __USBH_CTLREQ_H
#define __USBH_CTLREQ_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_def.h"

/* Exported functions --------------------------------------------------------*/
USBH_StatusTypeDef USBH_CtlReq(USBH_HandleTypeDef *phost, uint8_t *buff,
                               uint16_t length);

USBH_StatusTypeDef USBH_GetDescriptor(USBH_HandleTypeDef *phost,
                                      uint8_t req_type, uint16_t value_idx,
                                      uint8_t *buff, uint16_t length);

USBH_StatusTypeDef USBH_GetDevDesc(USBH_HandleTypeDef *phost, uint8_t length);
USBH_StatusTypeDef USBH_GetCfgDesc(USBH_HandleTypeDef *phost, uint16_t length);
USBH_StatusTypeDef USBH_GetString(USBH_HandleTypeDef *phost,
                                  uint8_t string_index, uint8_t *buff,
                                  uint16_t length);
USBH_StatusTypeDef USBH_SetCfg(USBH_HandleTypeDef *phost, uint16_t cfg_idx);
USBH_StatusTypeDef USBH_SetAddress(USBH_HandleTypeDef *phost,
                                   uint8_t DeviceAddress);
USBH_StatusTypeDef USBH_SetInterface(USBH_HandleTypeDef *phost, uint8_t iface,
                                     uint8_t altSetting);
USBH_StatusTypeDef USBH_SetFeature(USBH_HandleTypeDef *phost, uint8_t feature);
USBH_StatusTypeDef USBH_ClrFeature(USBH_HandleTypeDef *phost, uint8_t ep_num);

USBH_DescHeader_t *USBH_GetNextDesc(uint8_t *pbuf, uint16_t *ptr);

void USBH_ParseDevDesc(USBH_DevDescTypeDef *dev_desc, uint8_t *buf,
                       uint16_t length);
void USBH_ParseCfgDesc(USBH_CfgDescTypeDef *cfg_desc, uint8_t *buf,
                       uint16_t length);
void USBH_ParseEPDesc(USBH_EpDescTypeDef *ep_desc, uint8_t *buf);
void USBH_ParseInterfaceDesc(USBH_InterfaceDescTypeDef *if_desc, uint8_t *buf);

#ifdef __cplusplus
}
#endif

#endif /* __USBH_CTLREQ_H */
