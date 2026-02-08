/**
 ******************************************************************************
 * @file    usbh_ioreq.h
 * @author  MCD Application Team
 * @brief   Header file for usbh_ioreq.c
 ******************************************************************************
 */

#ifndef __USBH_IOREQ_H
#define __USBH_IOREQ_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_def.h"

/* Exported functions --------------------------------------------------------*/
USBH_StatusTypeDef USBH_CtlSendSetup(USBH_HandleTypeDef *phost, uint8_t *buff,
                                     uint8_t pipe_num);
USBH_StatusTypeDef USBH_CtlSendData(USBH_HandleTypeDef *phost, uint8_t *buff,
                                    uint16_t length, uint8_t pipe_num,
                                    uint8_t do_ping);
USBH_StatusTypeDef USBH_CtlReceiveData(USBH_HandleTypeDef *phost, uint8_t *buff,
                                       uint16_t length, uint8_t pipe_num);
USBH_StatusTypeDef USBH_BulkSendData(USBH_HandleTypeDef *phost, uint8_t *buff,
                                     uint16_t length, uint8_t pipe_num,
                                     uint8_t do_ping);
USBH_StatusTypeDef USBH_BulkReceiveData(USBH_HandleTypeDef *phost,
                                        uint8_t *buff, uint16_t length,
                                        uint8_t pipe_num);
USBH_StatusTypeDef USBH_InterruptReceiveData(USBH_HandleTypeDef *phost,
                                             uint8_t *buff, uint8_t length,
                                             uint8_t pipe_num);
USBH_StatusTypeDef USBH_InterruptSendData(USBH_HandleTypeDef *phost,
                                          uint8_t *buff, uint8_t length,
                                          uint8_t pipe_num);
USBH_StatusTypeDef USBH_IsocReceiveData(USBH_HandleTypeDef *phost,
                                        uint8_t *buff, uint32_t length,
                                        uint8_t pipe_num);
USBH_StatusTypeDef USBH_IsocSendData(USBH_HandleTypeDef *phost, uint8_t *buff,
                                     uint32_t length, uint8_t pipe_num);

#ifdef __cplusplus
}
#endif

#endif /* __USBH_IOREQ_H */
