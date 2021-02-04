#ifndef _TYPES_H_
#define _TYPES_H_

   //--- NVIC

#define  ISRPENDING      (1u<<22)
#define  VECTACTIVE      (0x000001FF)
#define  rNVIC_ICSR      (*((volatile unsigned int*)0xE000ED04))

   //--- Sys clock

#define  PLL0CFG_VAL              11  /* 11+1  */
#define  CCLKCFG_VAL               2  /* 2+1  */

#define  FLASHCFG_VAL    0x0000403A

  //-- Also was tested
//#define  PLL0CFG_VAL               9  /* 9+1  */
//#define  CCLKCFG_VAL               2  /* 2+1  */

   //--- UART

#define  LCR_DISABLE_LATCH_ACCESS    0x00000000
#define  LCR_ENABLE_LATCH_ACCESS     0x00000080

         //-- UART status register bits

#define  RX_INT    (0x2<<1)
#define  TO_INT    (0x6<<1)
#define  TX_INT    (0x1<<1)

#define  UART_FIFO_SIZE   16
#define  UART0_RX_PAYLOAD_BUF_SIZE   64
typedef struct _UARTDRV
{
   unsigned char * buf;
   int max_buf_size;
   int pos;
}UARTDRV;

   //--- I/O pins masks

#define  MASK_SSEL       (1<<12)
#define  MASK_SP1        (1<<3)
#define  MASK_SP2        (1<<4)
#define  MASK_LED1       (1<<9)
#define  MASK_SR_OE      (1<<7)
#define  MASK_RADIO_TX   (1<<25)
#define  MASK_CH_NUM     (1<<6)

#define  MASK_LED        (1<<26)
#define  LED_MASK        (1<<26)

//--------------------------------------------------------------------------

  //--- utils.c

void HardwareInit(void);
void uart0_send_str(unsigned char * buf);
void uart0_send_char(unsigned char ch);

   //--- tn_snprintf.c

int tn_snprintf( char *outStr, int maxLen, const char *fmt, ... );

   //--- startup_hardware_init.s

void tn_startup_hardware_init(void);

  //--- utils_uart.c

int uart_rx_drv_ex(UARTDRV * ud, unsigned char in_byte);
void send_data_uart0(unsigned char * buf, int size);
void send_data_uart0_str(unsigned char * buf);

#endif

