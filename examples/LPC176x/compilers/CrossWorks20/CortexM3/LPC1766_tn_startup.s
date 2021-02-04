
/*
*
*   LPC1766 startup file for CrossWorks 1.7
*
*/


  .global reset_handler

  .section .vectors, "ax"
  .code 16
  .align 0
  .global _vectors

.macro DEFAULT_ISR_HANDLER name=
  .thumb_func
  .weak \name
\name:
1: b 1b /* endless loop */
.endm

_vectors:
  .word (0x10008000 - 16)   /*  __stack_end__  */
  .word reset_handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0xEFFF7B40         /* Checksum for FLASH stand-alone start */
  .word 0                  /* Reserved  */
  .word 0                  /* Reserved  */
  .word 0                  /* Reserved  */
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0                  /* Reserved  */
  .word PendSV_Handler
  .word SysTick_Handler

 /* ---- External Interrupts ---- */

  .word WDT_IRQHandler            /* 16: Watchdog Timer */
  .word TIMER0_IRQHandler         /* 17: Timer0         */
  .word TIMER1_IRQHandler         /* 18: Timer1         */
  .word TIMER2_IRQHandler         /* 19: Timer2         */
  .word TIMER3_IRQHandler         /* 20: Timer3         */
  .word UART0_IRQHandler          /* 21: UART0          */
  .word UART1_IRQHandler          /* 22: UART1          */
  .word UART2_IRQHandler          /* 23: UART2          */
  .word UART3_IRQHandler          /* 24: UART3          */
  .word PWM1_IRQHandler           /* 25: PWM1           */
  .word I2C0_IRQHandler           /* 26: I2C0           */
  .word I2C1_IRQHandler           /* 27: I2C1           */
  .word I2C2_IRQHandler           /* 28: I2C2           */
  .word SPI_IRQHandler            /* 29: SPI            */
  .word SSP0_IRQHandler           /* 30: SSP0           */
  .word SSP1_IRQHandler           /* 31: SSP1           */
  .word PLL0_IRQHandler           /* 32: PLL0 Lock (Main PLL)           */
  .word RTC_IRQHandler            /* 33: Real Time Clock                */
  .word EINT0_IRQHandler          /* 34: External Interrupt 0           */
  .word EINT1_IRQHandler          /* 35: External Interrupt 1           */
  .word EINT2_IRQHandler          /* 36: External Interrupt 2           */
  .word EINT3_IRQHandler          /* 37: External Interrupt 3           */
  .word ADC_IRQHandler            /* 38: A/D Converter                  */
  .word BOD_IRQHandler            /* 39: Brown-Out Detect               */
  .word USB_IRQHandler            /* 40: USB                            */
  .word CAN_IRQHandler            /* 41: CAN                            */
  .word DMA_IRQHandler            /* 42: General Purpose DMA            */
  .word I2S_IRQHandler            /* 43: I2S                            */
  .word ENET_IRQHandler           /* 44: Ethernet                       */
  .word RIT_IRQHandler            /* 45: Repetitive Interrupt Timer     */
  .word MCPWM_IRQHandler          /* 46: Motor Control PWM              */
  .word QEI_IRQHandler            /* 47: Quadrature Encoder Interface   */
  .word PLL1_IRQHandler           /* 48: PLL1 Lock (USB PLL)            */

 /* --- code protection---- */

  .section .flashprot, "ax"
  .code 16
  .align 0
  .global flash_pcell

flash_pcell:    .word  0 //0x87654321  /* 0x0 */

 /* ----------------------- */
  .section .vectors /*, "ax" */
  .code 16
  .align 0

  .extern  tn_startup_hardware_init

  .thumb_func
     
reset_handler:

   ldr   r0,=tn_startup_hardware_init      /* vital hardware init */
   blx   r0

   b _start

DEFAULT_ISR_HANDLER  NMI_Handler
DEFAULT_ISR_HANDLER  HardFault_Handler
DEFAULT_ISR_HANDLER  MemManage_Handler
DEFAULT_ISR_HANDLER  BusFault_Handler
DEFAULT_ISR_HANDLER  UsageFault_Handler


  .section .init, "ax"
  .thumb_func

DEFAULT_ISR_HANDLER  SVC_Handler
DEFAULT_ISR_HANDLER  DebugMon_Handler
DEFAULT_ISR_HANDLER  PendSV_Handler
DEFAULT_ISR_HANDLER  SysTick_Handler

DEFAULT_ISR_HANDLER  WDT_IRQHandler
DEFAULT_ISR_HANDLER  TIMER0_IRQHandler
DEFAULT_ISR_HANDLER  TIMER1_IRQHandler
DEFAULT_ISR_HANDLER  TIMER2_IRQHandler
DEFAULT_ISR_HANDLER  TIMER3_IRQHandler
DEFAULT_ISR_HANDLER  UART0_IRQHandler
DEFAULT_ISR_HANDLER  UART1_IRQHandler
DEFAULT_ISR_HANDLER  UART2_IRQHandler
DEFAULT_ISR_HANDLER  UART3_IRQHandler
DEFAULT_ISR_HANDLER  PWM1_IRQHandler
DEFAULT_ISR_HANDLER  I2C0_IRQHandler
DEFAULT_ISR_HANDLER  I2C1_IRQHandler
DEFAULT_ISR_HANDLER  I2C2_IRQHandler
DEFAULT_ISR_HANDLER  SPI_IRQHandler
DEFAULT_ISR_HANDLER  SSP0_IRQHandler
DEFAULT_ISR_HANDLER  SSP1_IRQHandler
DEFAULT_ISR_HANDLER  PLL0_IRQHandler
DEFAULT_ISR_HANDLER  RTC_IRQHandler
DEFAULT_ISR_HANDLER  EINT0_IRQHandler
DEFAULT_ISR_HANDLER  EINT1_IRQHandler
DEFAULT_ISR_HANDLER  EINT2_IRQHandler
DEFAULT_ISR_HANDLER  EINT3_IRQHandler
DEFAULT_ISR_HANDLER  ADC_IRQHandler
DEFAULT_ISR_HANDLER  BOD_IRQHandler
DEFAULT_ISR_HANDLER  USB_IRQHandler
DEFAULT_ISR_HANDLER  CAN_IRQHandler
DEFAULT_ISR_HANDLER  DMA_IRQHandler
DEFAULT_ISR_HANDLER  I2S_IRQHandler
DEFAULT_ISR_HANDLER  ENET_IRQHandler
DEFAULT_ISR_HANDLER  RIT_IRQHandler
DEFAULT_ISR_HANDLER  MCPWM_IRQHandler
DEFAULT_ISR_HANDLER  QEI_IRQHandler
DEFAULT_ISR_HANDLER  PLL1_IRQHandler

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */













