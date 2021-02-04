/*
TNKernel real-time kernel

Copyright © 2006, 2009 Yuri Tiomkin
All rights reserved.

Permission to use, copy, modify, and distribute this software in source
and binary forms and its documentation for any purpose and without fee
is hereby granted, provided that the above copyright notice appear
in all copies and that both that copyright notice and this permission
notice appear in supporting documentation.

THIS SOFTWARE IS PROVIDED BY THE YURI TIOMKIN AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL YURI TIOMKIN OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#include <stddef.h>
#include "../../../cpu/lpc17xx/LPC176x.h"
#include "../../../TNKernel/tn.h"
#include "types.h"
#include "globals.h"

/* -- Project specific pinout description -

       LPC1766, SK_MLPC2368_LPC1766 board 

Port            Pin desc           Pin N   Name in prj   Using as    I/O dir
------   -----------------------   -----   -----------   --------   ---------

 P0.0    RD1/TXD3/SDA1               46
 P0.1    TD1/RXD3/SCL1               47
 P0.2    TXD0/AD0.7                  98     TXD0           TXD
 P0.3    RXD0/AD0.6                  99     RXD0           RXD
 P0.4    I2SRX_CLK/RD2/CAP2.0        81
 P0.5    I2SRX_WS/TD2/CAP2.1         80
 P0.6    I2SRX_SDA/SSEL1/MAT2.0      79
 P0.7    I2STX_CLK/SCK1/MAT2.1       78
 P0.8    I2STX_WS/MISO1/MAT2.2       77
 P0.9    I2STX_SDA/MOSI1/MAT2.3      76     Test 1(LED)    I/O         Out
 P0.10   TXD2/SDA2/MAT3.0            48
 P0.11   RXD2/SCL2/MAT3.1            49
 P0.15   TXD1/SCK0/SCK               62     SCK            SCK
 P0.16   RXD1/SSEL0/SSEL             63     Auto
 P0.17   CTS1/MISO0/MISO             61     MISO           MISO
 P0.18   DCD1/MOSI0/MOSI             60     MOSI           MOSI
 P0.19   DSR1/SDA1                   59
 P0.20   DTR1/SCL1                   58
 P0.21   RI1/RD1                     57
 P0.22   RTS1/TD1                    56
 P0.23   AD0.0 /I2SRX_CLK/CAP3.0      9
 P0.24   AD0.1 /I2SRX_WS/CAP3.1       8
 P0.25   AD0.2 /I2SRX_SDA/TXD3        7
 P0.26   AD0.3 /AOUT/RXD3             6
 P0.27   SDA0/USB_SDA                25
 P0.28   SCL0/USB_SCL                24
 P0.29   USB_D+                      29
 P0.30   USB_D-                      30

 P1.0    ENET_TXD0                   95
 P1.1    ENET_TXD1                   94
 P1.4    ENET_TX_EN                  93
 P1.8    ENET_CRS                    92
 P1.9    ENET_RXD0                   91
 P1.10   ENET_RXD1                   90
 P1.14   ENET_RX_ER                  89
 P1.15   ENET_REF_CLK                88
 P1.16   ENET_MDC                    87
 P1.17   ENET_MDIO                   86
 P1.18   USB_UP_LED/PWM1.1 /CAP1.0   32
 P1.19   MCOA0/USB_PPWR/CAP1.1       33
 P1.20   MCI0/PWM1.2 /SCK0           34
 P1.21   MCABORT/PWM1.3 /SSEL0       35
 P1.22   MCOB0/USB_PWRD/MAT1.0       36
 P1.23   MCI1/PWM1.4 /MISO0          37
 P1.24   MCI2/PWM1.5 /MOSI0          38
 P1.25   MCOA1/MAT1.1                39     ANT-CTRL       MAT1.1
 P1.26   MCOB1/PWM1.6 /CAP0.0        40   LED
 P1.27   CLKOUT/USB_OVRCR/CAP0.1     43
 P1.28   MCOA2/PCAP1.0 /MAT0.0       44
 P1.29   MCOB2/PCAP1.1 /MAT0.1       45
 P1.30   VBUS/AD0.4                  21
 P1.31   SCK1/AD0.5                  20

 P2.0    PWM1.1 /TXD1                75
 P2.1    PWM1.2 /RXD1                74
 P2.2    PWM1.3 /CTS1/TRACEDATA.3    73
 P2.3    PWM1.4 /DCD1/TRACEDATA.2    70     SP1            I/O        Out
 P2.4    PWM1.5 /DSR1/TRACEDATA.1    69     SP2            I/O        Out
 P2.5    PWM1.6 /DTR1/TRACEDATA.0    68
 P2.6    PCAP1.0 /RI1/TRACECLK       67     CURR-CH        I/O         In
 P2.7    RD2/RTS1                    66     SR-OE          I/O         Out
 P2.8    TD2/TXD2/ENET_MDC           65
 P2.9    USB_CONNECT/RXD2/ENET_MDIO  64
 P2.10   EINT0/NMI                   53     ISPreq         I/O        In
 P2.11   EINT1/I2STX_CLK             52
 P2.12   EINT2/I2STX_WS              51     SSEL           I/O        Out
 P2.13   EINT3/I2STX_SDA             50

 P3.26   STCLK/MAT0.1 /PWM1.3        26     TB             PWM1.3
 P3.25   MAT0.0 /PWM1.2              27

 P4.28   RX_MCLK/MAT2.0 /TXD3        82
 P4.29   TX_MCLK/MAT2.1 /RXD3        85
------------------------------------------------------------------------------*/

//---- Local functions prototypes --

static void InitPWM(void);
static void InitUART0(void);
static void InitSPI(void);
static void InitTimer1(void);

//----------------------------------------------------------------------------
void HardwareInit(void)
{
  //---- System clock ---

    //-- Fosc = 14.7456 MHz, Fcco = 2 * 12 * 14.7456 = 353.8944 MHz
    //-- cclk = 353.8944 / 4 = 88.4736 MHz
    //-- cclk/2 = 44.2368 MHz

     //-- Also was tested with

     //--   Fosc = 14.7456 MHz, Fcco = 2 * 10 * 14.7456 = 294.912 MHz,
     //--   cclk = 294.912 / 3 = 98.304 MHz


     //--   Fosc = 12.0000 MHz, Fcco = 2 * (11 + 1) * 12.0000 = 288.000 MHz,
     //--   cclk = 288.000 / (2 + 1) = 96 MHz

  rSCS = (1<<5);                //-- Start main oscillator,  freq < 20MHz
  while((rSCS & (1<<6)) == 0);  //-- Wait for Oscillator to be ready

  rCCLKCFG = CCLKCFG_VAL;       //-- Setup Clock Divider

  rCLKSRCSEL = 1;               //-- Select Clock Source for PLL0 - Main oscillator

  rPLL0CFG   = PLL0CFG_VAL;

  rPLL0CON   = 0x01;             //--- PLL0 Enable
  rPLL0FEED  = 0xAA;
  rPLL0FEED  = 0x55;
  while( (rPLL0STAT & (1<<26)) == 0); //-- Wait for PLOCK0

  rFLASHCFG  &= 0x00000FFF;  // dont change bits 11:0
  rFLASHCFG  |= 0x4 << 12;  //4+1 clk - up to 100 MHz   FLASHCFG_VAL;     //-- Flash speed

  rPLL0CON   = 0x03;             //--- PLL0 Enable & Connect

  rPLL0FEED  = 0xAA;
  rPLL0FEED  = 0x55;

 //--- !!! Peripheral Clock Selection - before PLL operations (see Errata file)

 //--  UART0, SPI, Timer 1, PWM1 - Enabled by default

 // rPCONP     = ();        // Power Control for Peripherals


//-------------- I/O pins ---------------------------

// P0.0    RD1/TXD3/SDA1               46
// P0.1    TD1/RXD3/SCL1               47

#define MASK_P0_0_1  (1 | (1<<1))

   rFIO0DIR |= MASK_P0_0_1;   //-- Set as Output
   rFIO0SET |= MASK_P0_0_1;   //-- Set to '1'

// P0.2    TXD0/AD0.7                  98     TXD0           TXD
// P0.3    RXD0/AD0.6                  99     RXD0           RXD

   InitUART0();

// P0.4    I2SRX_CLK/RD2/CAP2.0        81
// P0.5    I2SRX_WS/TD2/CAP2.1         80
// P0.6    I2SRX_SDA/SSEL1/MAT2.0      79
// P0.7    I2STX_CLK/SCK1/MAT2.1       78
// P0.8    I2STX_WS/MISO1/MAT2.2       77

#define MASK_P0_4_8  ((1<<4) | (1<<5) | (1<<6) | (1<<7) | (1<<8))

   rFIO0DIR |= MASK_P0_4_8;   //-- Set as Output
   rFIO0SET |= MASK_P0_4_8;   //-- Set to '1'

// P0.9    I2STX_SDA/MOSI1/MAT2.3      76     Test 1(LED)    I/O         Out

   rFIO0DIR |= MASK_LED1;   //-- Pin - Set as Output
   rFIO0CLR |= MASK_LED1;   //-- Set to '0'

// P0.10   TXD2/SDA2/MAT3.0            48
// P0.11   RXD2/SCL2/MAT3.1            49

#define MASK_P0_10_11  ((1<<10) | (1<<11))

   rFIO0DIR |= MASK_P0_10_11;   //-- Set as Output
   rFIO0SET |= MASK_P0_10_11;   //-- Set to '1'

// P0.15   TXD1/SCK0/SCK               62     SCK            SCK
// P0.16   RXD1/SSEL0/SSEL             63     Auto
// P0.17   CTS1/MISO0/MISO             61     MISO           MISO
// P0.18   DCD1/MOSI0/MOSI             60     MOSI           MOSI

   InitSPI();

// P0.19   DSR1/SDA1                   59
// P0.20   DTR1/SCL1                   58
// P0.21   RI1/RD1                     57
// P0.22   RTS1/TD1                    56
// P0.23   AD0.0 /I2SRX_CLK/CAP3.0      9
// P0.24   AD0.1 /I2SRX_WS/CAP3.1       8
// P0.25   AD0.2 /I2SRX_SDA/TXD3        7
// P0.26   AD0.3 /AOUT/RXD3             6
// P0.27   SDA0/USB_SDA                25
// P0.28   SCL0/USB_SCL                24

#define MASK_P0_19_28  ((1<<19) | (1<<20) | (1<<21) | (1<<22) | (1<<23) | (1<<24) | \
                        (1<<25) | (1<<26) | (1<<27) | (1<<28))

   rFIO0DIR |= MASK_P0_19_28;   //-- Set as Output
   rFIO0SET |= MASK_P0_19_28;   //-- Set to '1'

// P0.29   USB_D+                      29
// P0.30   USB_D-                      30

   //-- Default

// P1.0    ENET_TXD0                   95
// P1.1    ENET_TXD1                   94
// P1.4    ENET_TX_EN                  93
// P1.8    ENET_CRS                    92
// P1.9    ENET_RXD0                   91
// P1.10   ENET_RXD1                   90
// P1.14   ENET_RX_ER                  89
// P1.15   ENET_REF_CLK                88
// P1.16   ENET_MDC                    87
// P1.17   ENET_MDIO                   86
// P1.18   USB_UP_LED/PWM1.1 /CAP1.0   32
// P1.19   MCOA0/USB_PPWR/CAP1.1       33
// P1.20   MCI0/PWM1.2 /SCK0           34
// P1.21   MCABORT/PWM1.3 /SSEL0       35
// P1.22   MCOB0/USB_PWRD/MAT1.0       36
// P1.23   MCI1/PWM1.4 /MISO0          37
// P1.24   MCI2/PWM1.5 /MOSI0          38

#define MASK_P1_0_24  (1 | (1<<1) |  (1<<4) |  (1<<1) |  (1<<4) | (1<<8) | \
                 (1<<9) | (1<<20) | (1<<21) | (1<<22) | (1<<23) | (1<<24))

   rFIO1DIR |= MASK_P1_0_24;   //-- Set as Output
   rFIO1SET |= MASK_P1_0_24;   //-- Set to '1'

// P1.25   MCOA1/MAT1.1                39     ANT-CTRL       MAT1.1

   InitTimer1();

// P1.26   MCOB1/PWM1.6 /CAP0.0        40

   rFIO1DIR |= (1<<26);   //-- Set as Output
   rFIO1SET |= (1<<26);   //-- Set to '1'

// P1.27   CLKOUT/USB_OVRCR/CAP0.1     43

       //-- Clock Output Configuration - CPU CLP div by 2 and enabled

   rPINSEL3    |= (1<<22);                   //-- Set pin P1.27 as clock output
   rCLKOUTCFG  |= (1<<8) | (1<<4);

// P1.28   MCOA2/PCAP1.0 /MAT0.0       44
// P1.29   MCOB2/PCAP1.1 /MAT0.1       45
// P1.30   VBUS/AD0.4                  21
// P1.31   SCK1/AD0.5                  20

#define MASK_P1_28_31  ((1<<28) | (1<<29) | (1<<30) | (1u<<31))

   rFIO1DIR |= MASK_P1_28_31;   //-- Set as Output
   rFIO1SET |= MASK_P1_28_31;   //-- Set to '1'

// P2.0    PWM1.1 /TXD1                75
// P2.1    PWM1.2 /RXD1                74
// P2.2    PWM1.3 /CTS1/TRACEDATA.3    73

#define MASK_P2_0_2  ( 1 |(1<<1) | (1<<2))

   rFIO2DIR |= MASK_P2_0_2;   //-- Set as Output
   rFIO2SET |= MASK_P2_0_2;   //-- Set to '1'

// P2.3    PWM1.4 /DCD1/TRACEDATA.2    70     SP1            I/O        Out

   rFIO2DIR |= MASK_SP1;   //-- Set as Output
   rFIO2SET |= MASK_SP1;   //-- Set to '1'

// P2.4    PWM1.5 /DSR1/TRACEDATA.1    69     SP2            I/O        Out

   rFIO2DIR |= MASK_SP2;   //-- Set as Output
   rFIO2SET |= MASK_SP2;   //-- Set to '1'

// P2.5    PWM1.6 /DTR1/TRACEDATA.0    68

   rFIO2DIR |= (1<<5);   //-- Set as Output
   rFIO2SET |= (1<<5);   //-- Set to '1'

// P2.6    PCAP1.0 /RI1/TRACECLK       67         CURR-CH        I/O         In

     //-- Input

// P2.7    RD2/RTS1                    66         SR-OE

   rFIO2DIR |= MASK_SR_OE;   //-- Set as Output
   rFIO2SET |= MASK_SR_OE;   //-- Set to '1'

// P2.8    TD2/TXD2/ENET_MDC           65
// P2.9    USB_CONNECT/RXD2/ENET_MDIO  64

#define MASK_P2_8_9  ((1<<8) | (1<<9))

   rFIO2DIR |= MASK_P2_8_9;   //-- Set as Output
   rFIO2SET |= MASK_P2_8_9;   //-- Set to '1'

// P2.10   EINT0/NMI                   53     ISPreq         I/O        In

     //-- Input

// P2.11   EINT1/I2STX_CLK             52

   rFIO2DIR |= (1<<11);   //-- Set as Output
   rFIO2SET |= (1<<11);   //-- Set to '1'

// P2.12   EINT2/I2STX_WS              51     SSEL           I/O        Out

   rFIO2DIR |= MASK_SSEL;   //-- Set as Output
   rFIO2SET |= MASK_SSEL;   //-- Set to '1'

// P2.13   EINT3/I2STX_SDA             50

   rFIO2DIR |= (1<<13);   //-- Set as Output
   rFIO2SET |= (1<<13);   //-- Set to '1'

// P3.26   STCLK/MAT0.1 /PWM1.3        26     TB             PWM1.3

   InitPWM();

// P3.25   MAT0.0 /PWM1.2              27

   rFIO3DIR |= (1<<25);   //-- Set as Output
   rFIO3SET |= (1<<25);   //-- Set to '1'

// P4.28   RX_MCLK/MAT2.0 /TXD3        82
// P4.29   TX_MCLK/MAT2.1 /RXD3        85

#define MASK_P4_28_29  ((1<<28) | (1<<29))

   rFIO4DIR |= MASK_P4_28_29;   //-- Set as Output
   rFIO4SET |= MASK_P4_28_29;   //-- Set to '1'

//------ OS tick timer - SysTick = 1 ms -----

   SysTick_Config(96000); //88000);//88473);  //-- cpu clk/1000
}

//----------------------------------------------------------------------------
static void InitPWM(void)
{
   rPCONP |= 1<< 6;
   rPCLKSEL0 |= 1<<12; //-- Bits 13:12 = 01 PCLK_peripheral = CCLK

   //-- Pin P3.26 - PWM1.3 output

   rPINSEL7 |= (3<<20);  //-- Bits 21:20 = 11 - PWM1.3

   //--- PWM - timebase 0.5 ms

   //rPWM1TCR = 1<<1;  //-- bit 1 = 1 - TCR reset

   rPWM1PR  = 691/2;           //-- 128 * 691 = 88474
   rPWM1PC  = 691/2;

   rPWM1MCR = 1<<1;            //-- Enable MACH0 reseting

   rPWM1MR0 = 128;             //-- duty factor
   rPWM1MR3 =  96;

   rPWM1PCR |= 1<<11;          //-- Enable PWM1.3 output - single edge

//   rPWM1MCR |= 1<<9;          //-- Enable PWM1.3 interrupt

   rPWM1TCR = 9;      //-- 1001 bit 0 - counter enabld, bit 3 - PWM enabled

//   NVIC_EnableIRQ(PWM1_IRQn);

}

//----------------------------------------------------------------------------
static void InitUART0(void)
{
   unsigned int div_val;
   unsigned int cclk;
   unsigned int baudrate;

   //-- Pins

   rPINSEL0 |= (1<<4) |  //-- P0.2 Pins 5:4 = 01 TXD0
               (1<<6);   //-- P0.3 Pins 7:6 = 01 RXD0

   //-- enable access to divisor latch regs

   rU0LCR = LCR_ENABLE_LATCH_ACCESS;

   //-- set divisor for desired baud rate

   cclk     = 96000000; // 88000000;
   baudrate = 115200;

   div_val = (cclk / 16 ) / baudrate;
   rU0DLM = div_val / 256;
   rU0DLL = div_val % 256;

   //-- disable access to divisor latch regs (enable access to xmit/rcv fifos
   //-- and int enable regs)

   rU0LCR = LCR_DISABLE_LATCH_ACCESS;

   //rU0IER = 3;  //-- Enable Rx & Tx int
   
   rU0IER = 1; //-- Enable Rx int
   
   rU0FCR = (0x3<<6) | 1;  //-- Int Trigger - 14 bytes, Enable FIFO,Reset Tx FIFO & Rx FIFO

   //-- setup line control reg - disable break transmittion, even parity,
   //-- 1 stop bit, 8 bit chars

   rU0LCR = 0x13;//-- 0b00010011

  //---

   NVIC_EnableIRQ(UART0_IRQn);  //-- It is a safe case global interrupts are not yet enabled

}

//----------------------------------------------------------------------------
static void InitSPI(void)
{
   rPINSEL0 |= (3u << 30); //-- P0.15 Bits 31:30 = 11  SCK
	
   rPINSEL1 |= (3<<2) |    //-- P0.17 Bits 3:2   = 11  MISO
               (3<<4);    //-- P0.18 Bits 5:4   = 11  MOSI
	
   //-- P0.16  SSEL - as regular I/O

   rFIO0DIR |= 1<<16;   //-- Set as Output
   rFIO0SET |= 1<<16;   //-- Set to '1'
	//--------------------------------------------
   rS0SPCCR  = 16; //8; //-- Max speed
   rS0SPCR  = (1<<5);  //-- Master mode
            // | (1<<4) | (1<<3);
}

//----------------------------------------------------------------------------
static void InitTimer1(void)
{
   rFIO1DIR |= MASK_RADIO_TX;   //-- Set as Output
   rFIO1CLR |= MASK_RADIO_TX;   //-- Set to '0'

  //-- Timer 1 (MAT1.1) - radio frequency output

   rPINSEL3 |=  (3 << 18);    //-- P1.25  Bits 19:18 = 11 MAT1.1
   rT1PR = 111;    //-- Prescaler
   rT1PC = 0;

   //--- Match register -----

   rT1MR1 = 1;// SYNTH_MARK_VALUE; //SYNTH_SPACE_VALUE;
   rT1MCR =  (1<<4);     //--  bit 4=1 - Reset on MR1
   rT1EMR |= (0x3<<6);   //-- External Mach 1 - toggle
  //------------------------------------
   rT1TC  = 0;  //-- 1st cycle - full 0.5 mS

   rT1TCR = 1; //-- Timer 0 - run
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void uart0_send_str(unsigned char * buf)
{
   int ind;

   if(buf == NULL || buf[0] == 0)
      return;
   
   tn_sem_acquire(&semTxUart0, TN_WAIT_INFINITE);

   for(ind = 0;;)
   {
      if(buf[ind] == 0)
         break;
			
		  while (!(rU0LSR & 0x20))
			   tn_task_sleep(1);
			
      rU0THR = buf[ind++];
   }

   tn_sem_signal(&semTxUart0);
}

//----------------------------------------------------------------------------
void uart0_send_char(unsigned char ch)
{
   tn_sem_acquire(&semTxUart0, TN_WAIT_INFINITE);
			
   while (!(rU0LSR & 0x20))
	    tn_task_sleep(1);
   rU0THR = ch;

   tn_sem_signal(&semTxUart0);
}
//----------------------------------------------------------------------------
int spi_wr(unsigned char data)
{
   rS0SPDR = data;
   while((rS0SPSR & (1<<7)) == 0);
   return rS0SPDR;
}  

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


