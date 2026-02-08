/**
 ******************************************************************************
 * @file    usbh_diskio.c
 * @brief   USB Host Disk I/O driver for FatFS
 ******************************************************************************
 * @attention
 *
 * This file provides the disk I/O low-level driver for USB Mass Storage
 * devices using the STM32 USB Host Library.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usbh_diskio.h"
#include "ff_gen_drv.h"
#include "usbh_core.h"
#include "usbh_msc.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define USB_DEFAULT_BLOCK_SIZE 512

/* Private variables ---------------------------------------------------------*/
static volatile DSTATUS Stat = STA_NOINIT;

extern USBH_HandleTypeDef hUsbHostFS;

/* Private function prototypes -----------------------------------------------*/
static DSTATUS USBH_CheckStatus(BYTE lun);
DSTATUS USBH_initialize(BYTE);
DSTATUS USBH_status(BYTE);
DRESULT USBH_read(BYTE, BYTE *, DWORD, UINT);
#if _USE_WRITE == 1
DRESULT USBH_write(BYTE, const BYTE *, DWORD, UINT);
#endif
#if _USE_IOCTL == 1
DRESULT USBH_ioctl(BYTE, BYTE, void *);
#endif

const Diskio_drvTypeDef USBH_Driver = {
    USBH_initialize, USBH_status, USBH_read,
#if _USE_WRITE == 1
    USBH_write,
#endif
#if _USE_IOCTL == 1
    USBH_ioctl,
#endif
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Check USB disk status
 * @param  lun: Logical unit number
 * @retval DSTATUS: Disk status
 */
static DSTATUS USBH_CheckStatus(BYTE lun) {
  Stat = STA_NOINIT;

  if (USBH_MSC_UnitIsReady(&hUsbHostFS, lun)) {
    Stat &= ~STA_NOINIT;
  }

  return Stat;
}

/**
 * @brief  Initializes a USB drive
 * @param  lun: Logical unit number
 * @retval DSTATUS: Disk status
 */
DSTATUS USBH_initialize(BYTE lun) {
  /* Don't call USBH_Init here - it should be called in main */
  Stat = USBH_CheckStatus(lun);
  return Stat;
}

/**
 * @brief  Gets USB disk status
 * @param  lun: Logical unit number
 * @retval DSTATUS: Disk status
 */
DSTATUS USBH_status(BYTE lun) { return USBH_CheckStatus(lun); }

/**
 * @brief  Reads sector(s) from USB disk
 * @param  lun: Logical unit number
 * @param  *buff: Data buffer to store read data
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to read
 * @retval DRESULT: Operation result
 */
DRESULT USBH_read(BYTE lun, BYTE *buff, DWORD sector, UINT count) {
  DRESULT res = RES_ERROR;
  MSC_LUNTypeDef info;
  USBH_StatusTypeDef status;

  if ((Stat & STA_NOINIT) || (buff == NULL) || (count == 0)) {
    return RES_PARERR;
  }

  if (USBH_MSC_GetLUNInfo(&hUsbHostFS, lun, &info) == USBH_OK) {
    do {
      status = USBH_MSC_Read(&hUsbHostFS, lun, sector, buff, count);
      if (status == USBH_BUSY) {
        USBH_Process(&hUsbHostFS);
      }
    } while (status == USBH_BUSY);

    if (status == USBH_OK) {
      res = RES_OK;
    }
  }

  return res;
}

/**
 * @brief  Writes sector(s) to USB disk
 * @param  lun: Logical unit number
 * @param  *buff: Data to be written
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to write
 * @retval DRESULT: Operation result
 */
#if _USE_WRITE == 1
DRESULT USBH_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count) {
  DRESULT res = RES_ERROR;
  MSC_LUNTypeDef info;
  USBH_StatusTypeDef status;

  if ((Stat & STA_NOINIT) || (buff == NULL) || (count == 0)) {
    return RES_PARERR;
  }

  if (USBH_MSC_GetLUNInfo(&hUsbHostFS, lun, &info) == USBH_OK) {
    do {
      status = USBH_MSC_Write(&hUsbHostFS, lun, sector, (BYTE *)buff, count);
      if (status == USBH_BUSY) {
        USBH_Process(&hUsbHostFS);
      }
    } while (status == USBH_BUSY);

    if (status == USBH_OK) {
      res = RES_OK;
    }
  }

  return res;
}
#endif

/**
 * @brief  I/O control operation for USB disk
 * @param  lun: Logical unit number
 * @param  cmd: Control code
 * @param  *buff: Buffer to send/receive control data
 * @retval DRESULT: Operation result
 */
#if _USE_IOCTL == 1
DRESULT USBH_ioctl(BYTE lun, BYTE cmd, void *buff) {
  DRESULT res = RES_ERROR;
  MSC_LUNTypeDef info;

  if (Stat & STA_NOINIT) {
    return RES_NOTRDY;
  }

  if (USBH_MSC_GetLUNInfo(&hUsbHostFS, lun, &info) != USBH_OK) {
    return RES_ERROR;
  }

  switch (cmd) {
  case CTRL_SYNC:
    res = RES_OK;
    break;

  case GET_SECTOR_COUNT:
    *(DWORD *)buff = info.capacity.block_nbr;
    res = RES_OK;
    break;

  case GET_SECTOR_SIZE:
    *(WORD *)buff = info.capacity.block_size;
    res = RES_OK;
    break;

  case GET_BLOCK_SIZE:
    *(DWORD *)buff = info.capacity.block_size / USB_DEFAULT_BLOCK_SIZE;
    res = RES_OK;
    break;

  default:
    res = RES_PARERR;
    break;
  }

  return res;
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
