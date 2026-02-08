/**
 ******************************************************************************
 * @file    usbh_ioreq.c
 * @author  MCD Application Team
 * @brief   USB Host I/O requests handling
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usbh_ioreq.h"

/**
 * @brief  USBH_CtlSendSetup - Sends the Setup Packet to the Device
 * @param  phost: Host Handle
 * @param  buff: Buffer pointer
 * @param  pipe_num: Pipe Number
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_CtlSendSetup(USBH_HandleTypeDef *phost, uint8_t *buff,
                                     uint8_t pipe_num) {
  USBH_LL_SubmitURB(phost, pipe_num, 0U, EP_TYPE_CTRL, USBH_PID_SETUP, buff, 8U,
                    0U);
  return USBH_OK;
}

/**
 * @brief  USBH_CtlSendData - Sends a data Packet to the Device
 * @param  phost: Host Handle
 * @param  buff: Buffer pointer
 * @param  length: Data length
 * @param  pipe_num: Pipe Number
 * @param  do_ping: PING token
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_CtlSendData(USBH_HandleTypeDef *phost, uint8_t *buff,
                                    uint16_t length, uint8_t pipe_num,
                                    uint8_t do_ping) {
  if (phost->device.speed != USBH_SPEED_HIGH) {
    do_ping = 0U;
  }

  USBH_LL_SubmitURB(phost, pipe_num, 0U, EP_TYPE_CTRL, USBH_PID_DATA, buff,
                    length, do_ping);
  return USBH_OK;
}

/**
 * @brief  USBH_CtlReceiveData - Receives the Device Response to the Setup
 * Packet
 * @param  phost: Host Handle
 * @param  buff: Buffer pointer
 * @param  length: Data length
 * @param  pipe_num: Pipe Number
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_CtlReceiveData(USBH_HandleTypeDef *phost, uint8_t *buff,
                                       uint16_t length, uint8_t pipe_num) {
  USBH_LL_SubmitURB(phost, pipe_num, 1U, EP_TYPE_CTRL, USBH_PID_DATA, buff,
                    length, 0U);
  return USBH_OK;
}

/**
 * @brief  USBH_BulkSendData - Sends the Bulk Packet to the device
 * @param  phost: Host Handle
 * @param  buff: Buffer pointer
 * @param  length: Data length
 * @param  pipe_num: Pipe Number
 * @param  do_ping: PING token
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_BulkSendData(USBH_HandleTypeDef *phost, uint8_t *buff,
                                     uint16_t length, uint8_t pipe_num,
                                     uint8_t do_ping) {
  if (phost->device.speed != USBH_SPEED_HIGH) {
    do_ping = 0U;
  }

  USBH_LL_SubmitURB(phost, pipe_num, 0U, EP_TYPE_BULK, USBH_PID_DATA, buff,
                    length, do_ping);
  return USBH_OK;
}

/**
 * @brief  USBH_BulkReceiveData - Receives IN bulk packet from device
 * @param  phost: Host Handle
 * @param  buff: Buffer pointer
 * @param  length: Data length
 * @param  pipe_num: Pipe Number
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_BulkReceiveData(USBH_HandleTypeDef *phost,
                                        uint8_t *buff, uint16_t length,
                                        uint8_t pipe_num) {
  USBH_LL_SubmitURB(phost, pipe_num, 1U, EP_TYPE_BULK, USBH_PID_DATA, buff,
                    length, 0U);
  return USBH_OK;
}

/**
 * @brief  USBH_InterruptReceiveData - Receive interrupt data
 * @param  phost: Host Handle
 * @param  buff: Buffer pointer
 * @param  length: Data length
 * @param  pipe_num: Pipe Number
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_InterruptReceiveData(USBH_HandleTypeDef *phost,
                                             uint8_t *buff, uint8_t length,
                                             uint8_t pipe_num) {
  USBH_LL_SubmitURB(phost, pipe_num, 1U, EP_TYPE_INTR, USBH_PID_DATA, buff,
                    (uint16_t)length, 0U);
  return USBH_OK;
}

/**
 * @brief  USBH_InterruptSendData - Send interrupt data
 * @param  phost: Host Handle
 * @param  buff: Buffer pointer
 * @param  length: Data length
 * @param  pipe_num: Pipe Number
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_InterruptSendData(USBH_HandleTypeDef *phost,
                                          uint8_t *buff, uint8_t length,
                                          uint8_t pipe_num) {
  USBH_LL_SubmitURB(phost, pipe_num, 0U, EP_TYPE_INTR, USBH_PID_DATA, buff,
                    (uint16_t)length, 0U);
  return USBH_OK;
}

/**
 * @brief  USBH_IsocReceiveData - Receive isochronous data
 * @param  phost: Host Handle
 * @param  buff: Buffer pointer
 * @param  length: Data length
 * @param  pipe_num: Pipe Number
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_IsocReceiveData(USBH_HandleTypeDef *phost,
                                        uint8_t *buff, uint32_t length,
                                        uint8_t pipe_num) {
  USBH_LL_SubmitURB(phost, pipe_num, 1U, EP_TYPE_ISOC, USBH_PID_DATA, buff,
                    (uint16_t)length, 0U);
  return USBH_OK;
}

/**
 * @brief  USBH_IsocSendData - Send isochronous data
 * @param  phost: Host Handle
 * @param  buff: Buffer pointer
 * @param  length: Data length
 * @param  pipe_num: Pipe Number
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_IsocSendData(USBH_HandleTypeDef *phost, uint8_t *buff,
                                     uint32_t length, uint8_t pipe_num) {
  USBH_LL_SubmitURB(phost, pipe_num, 0U, EP_TYPE_ISOC, USBH_PID_DATA, buff,
                    (uint16_t)length, 0U);
  return USBH_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
