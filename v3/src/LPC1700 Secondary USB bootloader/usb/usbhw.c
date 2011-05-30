/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    USBHW.C
 *      Purpose: USB Hardware Layer Module for NXP LPC17xx
 *      Version: V1.10
 *----------------------------------------------------------------------------
 *      This file is part of the uVision/ARM development tools.
 *      This software may only be used under the terms of a valid, current,
 *      end user licence from KEIL for a compatible version of KEIL software
 *      development tools. Nothing else gives you the right to use it.
 *
 *      Copyright (c) 2005-2007 Keil Software.
 *---------------------------------------------------------------------------*/

#include "LPC17xx.h"
#include "usb.h"
#include "usbcfg.h"
#include "usbreg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "usbuser.h"



#pragma diag_suppress 1441


#define EP_MSK_CTRL 0x0001      /* Control Endpoint Logical Address Mask */
#define EP_MSK_BULK 0xC924      /* Bulk Endpoint Logical Address Mask */
#define EP_MSK_INT  0x4492      /* Interrupt Endpoint Logical Address Mask */
#define EP_MSK_ISO  0x1248      /* Isochronous Endpoint Logical Address Mask */


#if USB_DMA

#pragma arm section zidata = "USB_RAM"
DWORD UDCA[USB_EP_NUM];                     /* UDCA in USB RAM */
DWORD DD_NISO_Mem[4*DD_NISO_CNT];           /* Non-Iso DMA Descriptor Memory */
DWORD DD_ISO_Mem [5*DD_ISO_CNT];            /* Iso DMA Descriptor Memory */
#pragma arm section zidata
DWORD udca[USB_EP_NUM];                     /* UDCA saved values */

DWORD DDMemMap[2];                          /* DMA Descriptor Memory Usage */

#endif


/*
 *  Get Endpoint Physical Address
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    Endpoint Physical Address
 */

uint32_t EPAdr (uint32_t EPNum) {
  uint32_t val;

  val = (EPNum & 0x0F) << 1;
  if (EPNum & 0x80) {
    val += 1;
  }
  return (val);
}


/*
 *  Write Command
 *    Parameters:      cmd:   Command
 *    Return Value:    None
 */

void WrCmd (uint32_t cmd) {

  LPC_USB->USBDevIntClr = CCEMTY_INT;
  LPC_USB->USBCmdCode = cmd;
  while ((LPC_USB->USBDevIntSt & CCEMTY_INT) == 0);
}


/*
 *  Write Command Data
 *    Parameters:      cmd:   Command
 *                     val:   Data
 *    Return Value:    None
 */

void WrCmdDat (uint32_t cmd, uint32_t val) {

  LPC_USB->USBDevIntClr = CCEMTY_INT;
  LPC_USB->USBCmdCode = cmd;
  while ((LPC_USB->USBDevIntSt & CCEMTY_INT) == 0);
  LPC_USB->USBDevIntClr = CCEMTY_INT;
  LPC_USB->USBCmdCode = val;
  while ((LPC_USB->USBDevIntSt & CCEMTY_INT) == 0);
}


/*
 *  Read Command Data
 *    Parameters:      cmd:   Command
 *    Return Value:    Data Value
 */

uint32_t RdCmdDat (uint32_t cmd) {

  LPC_USB->USBDevIntClr = CCEMTY_INT | CDFULL_INT;
  LPC_USB->USBCmdCode = cmd;
  while ((LPC_USB->USBDevIntSt & CDFULL_INT) == 0);
  return (LPC_USB->USBCmdData);
}


/*
 *  USB Initialize Function
 *   Called by the User to initialize USB
 *    Return Value:    None
 */

void USB_Init (void) {
  //NVIC_InitTypeDef NVIC_InitStructure;

  LPC_USB->OTGClkCtrl = 0x1A;	/* This module is sent for USB device compliance 
			test. Host and Device are enabled. The host enabling is
			to address USB needClk issue. */ 
  while ( (LPC_USB->OTGClkSt & 0x1A) != 0x1A );
 
  // P2.9 -> USB_CONNECT
  LPC_PINCON->PINSEL4 &= ~0x000C0000;
  LPC_PINCON->PINSEL4 |= 0x00040000;

#if 0
  // P1.18 -> USB_UP_LED
  // P1.30 -> VBUS
  LPC_PINCON->PINSEL3 &= ~0x30000030;
  LPC_PINCON->PINSEL3 |= 0x20000010;
#endif
#if 1
  // P1.30 -> VBUS
  LPC_PINCON->PINSEL3 &= ~0x30000000;
  LPC_PINCON->PINSEL3 |= 0x20000000;
#endif

  // P0.29 -> USB_D+
  // P0.30 -> USB_D-
  LPC_PINCON->PINSEL1 &= ~0x3C000000;
  LPC_PINCON->PINSEL1 |= 0x14000000;

	NVIC_EnableIRQ(USB_IRQn);

#if 0
  USBDevIntEn = DEV_STAT_INT;                /* Enable Device Status Interrupt */
#endif

#if 1 /* Partial Manual Reset since Automatic Bus Reset is not working */
  USB_Reset();
  USB_SetAddress(0);
#endif
}


/*
 *  USB Connect Function
 *   Called by the User to Connect/Disconnect USB
 *    Parameters:      con:   Connect/Disconnect
 *    Return Value:    None
 */

void USB_Connect (uint32_t con) {
  WrCmdDat(CMD_SET_DEV_STAT, DAT_WR_BYTE(con ? DEV_CON : 0));
}


/*
 *  USB Reset Function
 *   Called automatically on USB Reset
 *    Return Value:    None
 */

void USB_Reset (void) {
#if USB_DMA
  uint32_t n;
#endif

  LPC_USB->USBEpInd = 0;
  LPC_USB->USBMaxPSize = USB_MAX_PACKET0;
  LPC_USB->USBEpInd = 1;
  LPC_USB->USBMaxPSize = USB_MAX_PACKET0;
  while ((LPC_USB->USBDevIntSt & EP_RLZED_INT) == 0);

  LPC_USB->USBEpIntClr  = 0xFFFFFFFF;
  LPC_USB->USBEpIntEn   = 0xFFFFFFFF ^ USB_DMA_EP;
  LPC_USB->USBDevIntClr = 0xFFFFFFFF;
  LPC_USB->USBDevIntEn  = DEV_STAT_INT    | EP_SLOW_INT    |
               (USB_SOF_EVENT   ? FRAME_INT : 0) |
               (USB_ERROR_EVENT ? ERR_INT   : 0);

#if USB_DMA
  USBUDCAH   = USB_RAM_ADR;
  USBDMARClr = 0xFFFFFFFF;
  USBEpDMADis  = 0xFFFFFFFF;
  USBEpDMAEn   = USB_DMA_EP;
  USBEoTIntClr = 0xFFFFFFFF;
  USBNDDRIntClr = 0xFFFFFFFF;
  USBSysErrIntClr = 0xFFFFFFFF;
  USBDMAIntEn  = 0x00000007;
  DDMemMap[0] = 0x00000000;
  DDMemMap[1] = 0x00000000;
  for (n = 0; n < USB_EP_NUM; n++) {
    udca[n] = 0;
    UDCA[n] = 0;
  }
#endif
}


/*
 *  USB Suspend Function
 *   Called automatically on USB Suspend
 *    Return Value:    None
 */

void USB_Suspend (void) {
  /* Performed by Hardware */
}


/*
 *  USB Resume Function
 *   Called automatically on USB Resume
 *    Return Value:    None
 */

void USB_Resume (void) {
  /* Performed by Hardware */
}


/*
 *  USB Remote Wakeup Function
 *   Called automatically on USB Remote Wakeup
 *    Return Value:    None
 */

void USB_WakeUp (void) {

  if (USB_DeviceStatus & USB_GETSTATUS_REMOTE_WAKEUP) {
    WrCmdDat(CMD_SET_DEV_STAT, DAT_WR_BYTE(DEV_CON));
  }
}


/*
 *  USB Remote Wakeup Configuration Function
 *    Parameters:      cfg:   Enable/Disable
 *    Return Value:    None
 */

void USB_WakeUpCfg (uint32_t cfg) {
  /* Not needed */
}


/*
 *  USB Set Address Function
 *    Parameters:      adr:   USB Address
 *    Return Value:    None
 */

void USB_SetAddress (uint32_t adr) {
  WrCmdDat(CMD_SET_ADDR, DAT_WR_BYTE(DEV_EN | adr)); /* Don't wait for next */
  WrCmdDat(CMD_SET_ADDR, DAT_WR_BYTE(DEV_EN | adr)); /*  Setup Status Phase */
}


/*
 *  USB Configure Function
 *    Parameters:      cfg:   Configure/Deconfigure
 *    Return Value:    None
 */

void USB_Configure (uint32_t cfg) {

  WrCmdDat(CMD_CFG_DEV, DAT_WR_BYTE(cfg ? CONF_DVICE : 0));

  LPC_USB->USBReEp = 0x00000003;
  while ((LPC_USB->USBDevIntSt & EP_RLZED_INT) == 0);
  LPC_USB->USBDevIntClr = EP_RLZED_INT;
}


/*
 *  Configure USB Endpoint according to Descriptor
 *    Parameters:      pEPD:  Pointer to Endpoint Descriptor
 *    Return Value:    None
 */

void USB_ConfigEP (USB_ENDPOINT_DESCRIPTOR *pEPD) {
  uint32_t num;

  num = EPAdr(pEPD->bEndpointAddress);
  LPC_USB->USBReEp |= (1 << num);
  LPC_USB->USBEpInd = num;
  LPC_USB->USBMaxPSize = pEPD->wMaxPacketSize;
  while ((LPC_USB->USBDevIntSt & EP_RLZED_INT) == 0);
  LPC_USB->USBDevIntClr = EP_RLZED_INT;
}


/*
 *  Set Direction for USB Control Endpoint
 *    Parameters:      dir:   Out (dir == 0), In (dir <> 0)
 *    Return Value:    None
 */

void USB_DirCtrlEP (uint32_t dir) {
  /* Not needed */
}


/*
 *  Enable USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_EnableEP (uint32_t EPNum) {
  WrCmdDat(CMD_SET_EP_STAT(EPAdr(EPNum)), DAT_WR_BYTE(0));
}


/*
 *  Disable USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_DisableEP (uint32_t EPNum) {
  WrCmdDat(CMD_SET_EP_STAT(EPAdr(EPNum)), DAT_WR_BYTE(EP_STAT_DA));
}


/*
 *  Reset USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ResetEP (uint32_t EPNum) {
  WrCmdDat(CMD_SET_EP_STAT(EPAdr(EPNum)), DAT_WR_BYTE(0));
}


/*
 *  Set Stall for USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_SetStallEP (uint32_t EPNum) {
  WrCmdDat(CMD_SET_EP_STAT(EPAdr(EPNum)), DAT_WR_BYTE(EP_STAT_ST));
}


/*
 *  Clear Stall for USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ClrStallEP (uint32_t EPNum) {
  WrCmdDat(CMD_SET_EP_STAT(EPAdr(EPNum)), DAT_WR_BYTE(0));
}


/*
 *  Read USB Endpoint Data
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *                     pData: Pointer to Data Buffer
 *    Return Value:    Number of bytes read
 */

uint32_t USB_ReadEP (uint32_t EPNum, uint8_t *pData) {
  uint32_t cnt, n;

  LPC_USB->USBCtrl = ((EPNum & 0x0F) << 2) | CTRL_RD_EN;

  do {
    cnt = LPC_USB->USBRxPLen;
  } while ((cnt & PKT_RDY) == 0);
  cnt &= PKT_LNGTH_MASK;

  for (n = 0; n < (cnt + 3) / 4; n++) {
    *((__packed uint32_t *)pData) = LPC_USB->USBRxData;
    pData += 4;
  }

  LPC_USB->USBCtrl = 0;

  if (((EP_MSK_ISO >> EPNum) & 1) == 0) {   /* Non-Isochronous Endpoint */
    WrCmd(CMD_SEL_EP(EPAdr(EPNum)));
    WrCmd(CMD_CLR_BUF);
  }

  return (cnt);
}


/*
 *  Write USB Endpoint Data
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *                     pData: Pointer to Data Buffer
 *                     cnt:   Number of bytes to write
 *    Return Value:    Number of bytes written
 */

uint32_t USB_WriteEP (uint32_t EPNum, uint8_t *pData, uint32_t cnt) {
  uint32_t n;

  LPC_USB->USBCtrl = ((EPNum & 0x0F) << 2) | CTRL_WR_EN;

  LPC_USB->USBTxPLen = cnt;

  for (n = 0; n < (cnt + 3) / 4; n++) {
    LPC_USB->USBTxData = *((__packed uint32_t *)pData);
    pData += 4;
  }

  LPC_USB->USBCtrl = 0;

  WrCmd(CMD_SEL_EP(EPAdr(EPNum)));
  WrCmd(CMD_VALID_BUF);

  return (cnt);
}


#if USB_DMA


/* DMA Descriptor Memory Layout */
const uint32_t DDAdr[2] = { DD_NISO_ADR, DD_ISO_ADR };
const uint32_t DDSz [2] = { 16,          20         };


/*
 *  Setup USB DMA Transfer for selected Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                     pDD: Pointer to DMA Descriptor
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t USB_DMA_Setup(uint32_t EPNum, USB_DMA_DESCRIPTOR *pDD) {
  uint32_t num, ptr, nxt, iso, n;

  iso = pDD->Cfg.Type.IsoEP;                /* Iso or Non-Iso Descriptor */
  num = EPAdr(EPNum);                       /* Endpoint's Physical Address */

  ptr = 0;                                  /* Current Descriptor */
  nxt = udca[num];                          /* Initial Descriptor */
  while (nxt) {                             /* Go through Descriptor List */
    ptr = nxt;                              /* Current Descriptor */
    if (!pDD->Cfg.Type.Link) {              /* Check for Linked Descriptors */
      n = (ptr - DDAdr[iso]) / DDSz[iso];   /* Descriptor Index */
      DDMemMap[iso] &= ~(1 << n);           /* Unmark Memory Usage */
    }
    nxt = *((DWORD *)ptr);                  /* Next Descriptor */
  }

  for (n = 0; n < 32; n++) {                /* Search for available Memory */
    if ((DDMemMap[iso] & (1 << n)) == 0) {
      break;                                /* Memory found */
    }
  }
  if (n == 32) return (FALSE);              /* Memory not available */

  DDMemMap[iso] |= 1 << n;                  /* Mark Memory Usage */
  nxt = DDAdr[iso] + n * DDSz[iso];         /* Next Descriptor */

  if (ptr && pDD->Cfg.Type.Link) {
    *((DWORD *)(ptr + 0))  = nxt;           /* Link in new Descriptor */
    *((DWORD *)(ptr + 4)) |= 0x00000004;    /* Next DD is Valid */
  } else {
    udca[num] = nxt;                        /* Save new Descriptor */
    UDCA[num] = nxt;                        /* Update UDCA in USB */
  }

  /* Fill in DMA Descriptor */
  *(((uint32_t *)nxt)++) =  0;                 /* Next DD Pointer */
  *(((uint32_t *)nxt)++) =  pDD->Cfg.Type.ATLE |
                       (pDD->Cfg.Type.IsoEP << 4) |
                       (pDD->MaxSize <<  5) |
                       (pDD->BufLen  << 16);
  *(((DWORD *)nxt)++) =  pDD->BufAdr;
  *(((DWORD *)nxt)++) =  pDD->Cfg.Type.LenPos << 8;
  if (iso) {
    *((uint32_t *)nxt) =  pDD->InfoAdr;
  }

  return (TRUE); /* Success */
}


/*
 *  Enable USB DMA Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_DMA_Enable (uint32_t EPNum) {
  USBEpDMAEn = 1 << EPAdr(EPNum);
}


/*
 *  Disable USB DMA Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_DMA_Disable (uint32_t EPNum) {
  USBEpDMADis = 1 << EPAdr(EPNum);
}


/*
 *  Get USB DMA Endpoint Status
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    DMA Status
 */

DWORD USB_DMA_Status (uint32_t EPNum) {
  DWORD ptr, val;

  ptr = UDCA[EPAdr(EPNum)];                 /* Current Descriptor */
  if (ptr == 0) return (USB_DMA_INVALID);

  val = *((DWORD *)(ptr + 3*4));            /* Status Information */
  switch ((val >> 1) & 0x0F) {
    case 0x00:                              /* Not serviced */
      return (USB_DMA_IDLE);
    case 0x01:                              /* Being serviced */
      return (USB_DMA_BUSY);
    case 0x02:                              /* Normal Completition */
      return (USB_DMA_DONE);
    case 0x03:                              /* Data Under Run */
      return (USB_DMA_UNDER_RUN);
    case 0x08:                              /* Data Over Run */
      return (USB_DMA_OVER_RUN);
    case 0x09:                              /* System Error */
      return (USB_DMA_ERROR);
  }

  return (USB_DMA_UNKNOWN);
}


/*
 *  Get USB DMA Endpoint Current Buffer Address
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    DMA Address (or -1 when DMA is Invalid)
 */

uint32_t USB_DMA_BufAdr (uint32_t EPNum) {
  uint32_t ptr, val;

  ptr = UDCA[EPAdr(EPNum)];                 /* Current Descriptor */
  if (ptr == 0) return ((DWORD)(-1));                /* DMA Invalid */

  val = *((DWORD *)(ptr + 2*4));            /* Buffer Address */

  return (val);                             /* Current Address */
}


/*
 *  Get USB DMA Endpoint Current Buffer Count
 *   Number of transfered Bytes or Iso Packets
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    DMA Count (or -1 when DMA is Invalid)
 */

uint32_t USB_DMA_BufCnt (uint32_t EPNum) {
  uint32_t ptr, val;

  ptr = UDCA[EPAdr(EPNum)];                 /* Current Descriptor */
  if (ptr == 0) return ((DWORD)(-1));       /* DMA Invalid */

  val = *((DWORD *)(ptr + 3*4));            /* Status Information */

  return (val >> 16);                       /* Current Count */
}


#endif /* USB_DMA */


/*
 *  Get USB Last Frame Number
 *    Parameters:      None
 *    Return Value:    Frame Number
 */

uint32_t USB_GetFrame (void) {
  uint32_t val;

  WrCmd(CMD_RD_FRAME);
  val = RdCmdDat(DAT_RD_FRAME);
  val = val | (RdCmdDat(DAT_RD_FRAME) << 8);

  return (val);
}


/*
 *  USB Interrupt Service Routine
 */

void USB_IRQHandler (void) {
  uint32_t disr, val, n, m;

  disr = LPC_USB->USBDevIntSt;                      /* Device Interrupt Status */
  LPC_USB->USBDevIntClr = disr;					   /* A known issue on LPC214x */

  /* Device Status Interrupt (Reset, Connect change, Suspend/Resume) */
  if (disr & DEV_STAT_INT) {
    WrCmd(CMD_GET_DEV_STAT);
    val = RdCmdDat(DAT_GET_DEV_STAT);       /* Device Status */
    if (val & DEV_RST) {                    /* Reset */
      USB_Reset();
#if   USB_RESET_EVENT
      USB_Reset_Event();
#endif
      goto isr_end;
    }
    if (val & DEV_CON_CH) {                 /* Connect change */
#if   USB_POWER_EVENT
      USB_Power_Event(val & DEV_CON);
#endif
      goto isr_end;
    }
    if (val & DEV_SUS_CH) {                 /* Suspend/Resume */
      if (val & DEV_SUS) {                  /* Suspend */
        USB_Suspend();
#if     USB_SUSPEND_EVENT
        USB_Suspend_Event();
#endif
      } else {                              /* Resume */
        USB_Resume();
#if     USB_RESUME_EVENT
        USB_Resume_Event();
#endif
      }
      goto isr_end;
    }
  }

#if USB_SOF_EVENT
  /* Start of Frame Interrupt */
  if (disr & FRAME_INT) {
    USB_SOF_Event();
  }
#endif

#if USB_ERROR_EVENT
  /* Error Interrupt */
  if (disr & ERR_INT) {
    WrCmd(CMD_RD_ERR_STAT);
    val = RdCmdDat(DAT_RD_ERR_STAT);
    USB_Error_Event(val);
  }
#endif

  /* Endpoint's Slow Interrupt */
  if (disr & EP_SLOW_INT) {

    while (LPC_USB->USBEpIntSt) {                   /* Endpoint Interrupt Status */

      for (n = 0; n < USB_EP_NUM; n++) {    /* Check All Endpoints */
        if (LPC_USB->USBEpIntSt & (1 << n)) {
          m = n >> 1;

          LPC_USB->USBEpIntClr = 1 << n;
          while ((LPC_USB->USBDevIntSt & CDFULL_INT) == 0);
          val = LPC_USB->USBCmdData;

          if ((n & 1) == 0) {               /* OUT Endpoint */
            if (n == 0) {                   /* Control OUT Endpoint */
              if (val & EP_SEL_STP) {       /* Setup Packet */
                if (USB_P_EP[0]) {
                  USB_P_EP[0](USB_EVT_SETUP);
                  continue;
                }
              }
            }
            if (USB_P_EP[m]) {
              USB_P_EP[m](USB_EVT_OUT);
            }
          } else {                          /* IN Endpoint */
            if (USB_P_EP[m]) {
              USB_P_EP[m](USB_EVT_IN);
            }
          }
        }
      }
    }
  }

#if USB_DMA

  if (USBDMAIntSt & 0x00000001) {          /* End of Transfer Interrupt */
    val = USBEoTIntSt;
    for (n = 2; n < USB_EP_NUM; n++) {      /* Check All Endpoints */
      if (val & (1 << n)) {
        m = n >> 1;
        if ((n & 1) == 0) {                 /* OUT Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_OUT_DMA_EOT);
          }
        } else {                            /* IN Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_IN_DMA_EOT);
          }
        }
      }
    }
    USBEoTIntClr = val;
  }

  if (USBDMAIntSt & 0x00000002) {          /* New DD Request Interrupt */
    val = USBNDDRIntSt;
    for (n = 2; n < USB_EP_NUM; n++) {      /* Check All Endpoints */
      if (val & (1 << n)) {
        m = n >> 1;
        if ((n & 1) == 0) {                 /* OUT Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_OUT_DMA_NDR);
          }
        } else {                            /* IN Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_IN_DMA_NDR);
          }
        }
      }
    }
    USBNDDRIntClr = val;
  }

  if (USBDMAIntSt & 0x00000004) {          /* System Error Interrupt */
    val = USBSysErrIntSt;
    for (n = 2; n < USB_EP_NUM; n++) {      /* Check All Endpoints */
      if (val & (1 << n)) {
        m = n >> 1;
        if ((n & 1) == 0) {                 /* OUT Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_OUT_DMA_ERR);
          }
        } else {                            /* IN Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_IN_DMA_ERR);
          }
        }
      }
    }
    USBSysErrIntClr = val;
  }

#endif /* USB_DMA */

isr_end:
//  USBDevIntClr = disr;
  return;
}
