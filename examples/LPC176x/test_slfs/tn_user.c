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

#include "../../../cpu/lpc17xx/LPC176x.h"
#include "../../../TNKernel/tn.h"
#include "types.h"
#include "globals.h"

void SysTick_Handler(void);
void UART0_IRQHandler(void);
     
static void tn_calc_performance(void);


//----------------------------------------------------------------------------
//  SysTick timer (1 ms)
//----------------------------------------------------------------------------
void SysTick_Handler(void)
{
   tn_tick_int_processing();

   tn_calc_performance();

  //-- !!!    For the Cortex CPU, this function always MUST be a last func in 
  //--     any user's interrupt handler

   tn_int_exit();  
}

//----------------------------------------------------------------------------
void UART0_IRQHandler(void)
{
   register int rc;
   register int data;
   register int i;
   unsigned int status;
   unsigned char * rx_buf;

   status = rU0IIR;  // Reset Int Request Source

   status &= (0x7 <<1);

  //----- RX --------------------------------

   if(status == RX_INT || status == TO_INT)
   {
      rc = tn_fmem_get_ipolling(&RxUART0MemPool, (void **) &rx_buf);
      if(rc != TERR_NO_ERR)
         rx_buf = NULL;

      rc = 0;
      i = UART_FIFO_SIZE;
      while((rU0LSR & 0x01) && (i--)) //-- Rx FIFO not empty
      {
         data = rU0RBR;
         if(rx_buf)
            rx_buf[rc++] = (unsigned char)data;
      }

      if(rc > 0)
      {
         //--- Pack len & addr
         data = (rc << 24) | (((unsigned int)rx_buf) & 0x00FFFFFF);
         data = tn_queue_isend_polling(&queueRxUART0, (void *) data);
         if(data != TERR_NO_ERR)
            rc = 0;
      }

      if(rc == 0 && rx_buf)
         tn_fmem_irelease(&RxUART0MemPool, (void *) rx_buf);
   }

    //----- TX --------------------------------

 //  if(status == TX_INT)
 //     tn_sem_isignal(&semFifoEmptyTxUART0);


  //-- !!!    For the Cortex CPU, this function always MUST be a last func in 
  //--     any user's interrupt handler
   tn_int_exit();
}

//----------------------------------------------------------------------------
static void tn_calc_performance(void)
{
   static int cnt = 0;

   cnt++;
   if(cnt == 1000)  //-- 1 s
   {
      cnt = 0;

      tn_curr_performance = tn_idle_count;
      tn_idle_count = 0;

   //   tn_sem_isignal(&semMeasPrf);  //-- To send result to the host
   }
}

//----------------------------------------------------------------------------
// CPU specific routines for Cortex-M3
//
//----------------------------------------------------------------------------
void tn_cpu_int_enable(void)
{
   tn_arm_enable_interrupts();
}

//----------------------------------------------------------------------------
int tn_inside_int(void)
{
   if(rNVIC_ICSR & VECTACTIVE)
      return 1;
   return 0;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------





