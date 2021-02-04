/*****************************************************************************
*
*  Copyright (c) 2011 Yuri Tiomkin
*  All Rights Reserved
*
******************************************************************************
*
*  THIS SOFTWARE IS PROVIDED BY YURI TIOMKIN "AS IS" AND ANY EXPRESSED OR
*  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
*  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
*  IN NO EVENT SHALL YURI TIOMKIN OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
*  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
*  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

/* This file is not used in WIN32 emulator */

#include "../cpu/lpc17xx/LPC176x.h"
#include "../TNKernel/tn.h"

#include "sys_errorcodes.h"
#include "flash_fs.h"
#include "df_flash.h"


static void m25_wait_for_result(void);
static void m25_wr_enable(void);
static void m25_wr_disable(void);

#define USE_RTOS_DELAY 1

//----------------------------------------------------------------------------
//  No erasing here
//----------------------------------------------------------------------------
int m25_write_page(unsigned char * buf, __int32 flash_addr)
{
   union
   {
      unsigned long ul;
      unsigned char b[4];
   }ul2b;
   int nb;
   unsigned char * ptr;

   m25_wr_enable();

   spi_cs_0();

   spi_wr(PAGE_PROGRAM);

   ul2b.ul = flash_addr;
   spi_wr(ul2b.b[2]);
   spi_wr(ul2b.b[1]);
   spi_wr(ul2b.b[0]);

   ptr = buf;
   nb = DF_PAGE_SIZE;
   while(nb--)
      spi_wr(*ptr++);

   spi_cs_1();

   //--- Wait the result

   m25_wait_for_result();

   m25_wr_disable();

   return ERR_NO_ERR;
}
//----------------------------------------------------------------------------
//  Chk page for FF if not -erase the overall sector and write the page
//----------------------------------------------------------------------------
int m25_write_page_ex(unsigned char * buf, __int32 flash_addr)
{
   union
   {
      unsigned long ul;
      unsigned char b[4];
   }ul2b;

   int fErase = FALSE;
   int res;
   int nb;
   unsigned char * ptr;

   nb = DF_PAGE_SIZE;
   ul2b.ul = flash_addr;

   //--- Check the page for all FF

   spi_cs_0();

   spi_wr(FAST_READ);
   spi_wr(ul2b.b[2]);
   spi_wr(ul2b.b[1]);
   spi_wr(ul2b.b[0]);

   spi_wr(M25_DUMMY);  //-- for FAST_READ only

   while(nb--)
   {
      res = spi_wr(M25_DUMMY);
      if((res & 0xFF) != 0xFF)
      {
         fErase = TRUE;
         break;
      }
   }

   spi_cs_1();

   if(fErase) //-- Erase the overall sector
   {
      m25_wr_enable();

      spi_cs_0();

      spi_wr(SUBSECTOR_ERASE);

      ul2b.ul = flash_addr;
      spi_wr(ul2b.b[2]);
      spi_wr(ul2b.b[1]);
      spi_wr(ul2b.b[0]);

      spi_cs_1();

     //--- Wait the result

      m25_wait_for_result();

      m25_wr_disable();
   }

  //-- Write the page

   m25_wr_enable();

   spi_cs_0();

   spi_wr(PAGE_PROGRAM);

   ul2b.ul = flash_addr;
   spi_wr(ul2b.b[2]);
   spi_wr(ul2b.b[1]);
   spi_wr(ul2b.b[0]);

   ptr = buf;
   nb = DF_PAGE_SIZE;
   while(nb--)
      spi_wr(*ptr++);

   spi_cs_1();

   //--- Wait the result

   m25_wait_for_result();

   m25_wr_disable();

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
int m25_read_page(unsigned char * buf, __int32 flash_addr)
{
   union
   {
      unsigned long ul;
      unsigned char b[4];
   }ul2b;
   unsigned char * ptr;
   int nbytes;
   spi_cs_0();

   spi_wr(FAST_READ);

   ul2b.ul = flash_addr;
   spi_wr(ul2b.b[2]);
   spi_wr(ul2b.b[1]);
   spi_wr(ul2b.b[0]);

   spi_wr(M25_DUMMY);  //-- for FAST_READ only

   ptr = buf;
   nbytes = DF_PAGE_SIZE;
   while(nbytes--)
    *ptr++ = spi_wr(M25_DUMMY);

   spi_cs_1();

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
int m25_read_sector(unsigned char * buf, __int32 flash_addr)
{
   union
   {
      unsigned long ul;
      unsigned char b[4];
   }ul2b;

   int nbytes;
#ifdef WIN32
   int fErase = FALSE;
   int res;
#endif
   int nb;
   unsigned char * ptr;

   nbytes = DF_SECTOR_SIZE;
   ptr = buf;

   while(nbytes > 0)
   {
      spi_cs_0();

      spi_wr(FAST_READ);

      ul2b.ul = flash_addr;
      spi_wr(ul2b.b[2]);
      spi_wr(ul2b.b[1]);
      spi_wr(ul2b.b[0]);

      spi_wr(M25_DUMMY);  //-- for FAST_READ only

      nb = DF_PAGE_SIZE;
      while(nb--)
         *ptr++ = spi_wr(M25_DUMMY);

      spi_cs_1();

      nbytes     -= DF_PAGE_SIZE;
      flash_addr += (__int32)DF_PAGE_SIZE;
   }

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
//  Check for erasing here and erase sector if necessary
//----------------------------------------------------------------------------
int m25_write_sector(unsigned char * buf, __int32 flash_addr)
{
   union
   {
      unsigned long ul;
      unsigned char b[4];
   }ul2b;

   int nbytes;
   int fErase = FALSE;
   int res;
   int nb;
   unsigned char * ptr;

   nbytes = DF_SECTOR_SIZE;
   ul2b.ul = flash_addr;

   //--- Check sector for all FF

   spi_cs_0();

   spi_wr(FAST_READ);
   spi_wr(ul2b.b[2]);
   spi_wr(ul2b.b[1]);
   spi_wr(ul2b.b[0]);

   spi_wr(M25_DUMMY);  //-- for FAST_READ only

   while(nbytes--)
   {
      res = spi_wr(M25_DUMMY);
      if((res & 0xFF) != 0xFF)
      {
         fErase = TRUE;
         break;
      }
   }

   spi_cs_1();

   if(fErase)
   {
      m25_wr_enable();

      spi_cs_0();

      spi_wr(SUBSECTOR_ERASE);

      ul2b.ul = flash_addr;
      spi_wr(ul2b.b[2]);
      spi_wr(ul2b.b[1]);
      spi_wr(ul2b.b[0]);

      spi_cs_1();

     //--- Wait the result

      m25_wait_for_result();

      m25_wr_disable();
   }

   //--- Wr sector page by page

   nbytes = DF_SECTOR_SIZE;
   ptr = buf;

   while(nbytes > 0)
   {
      m25_wr_enable();

      spi_cs_0();

      spi_wr(PAGE_PROGRAM);

      ul2b.ul = flash_addr; //-- As micro delay also
      spi_wr(ul2b.b[2]);
      spi_wr(ul2b.b[1]);
      spi_wr(ul2b.b[0]);

      nb = DF_PAGE_SIZE;
      while(nb--)
         spi_wr(*ptr++);

      spi_cs_1();

     //--- Wait the result

      m25_wait_for_result();

      m25_wr_disable();

      nbytes     -= DF_PAGE_SIZE;
      flash_addr += DF_PAGE_SIZE;
   }


   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
//  Check for all FF and erase sector if necessary
//----------------------------------------------------------------------------
int m25_erase_sector(__int32 flash_addr)
{
   union
   {
      unsigned long ul;
      unsigned char b[4];
   }ul2b;
   int nbytes;
   int fErase = FALSE;
   int res;

   nbytes = DF_SECTOR_SIZE;

   //--- Check sector for all FF

   spi_cs_0();

   spi_wr(FAST_READ);

   ul2b.ul = flash_addr;
   spi_wr(ul2b.b[2]);
   spi_wr(ul2b.b[1]);
   spi_wr(ul2b.b[0]);

   spi_wr(M25_DUMMY);  //-- for FAST_READ only

   while(nbytes--)
   {
      res = spi_wr(M25_DUMMY);
      if((res & 0xFF) != 0xFF)
      {
         fErase = TRUE;
         break;
      }
   }
   spi_cs_1();

   if(fErase)
   {

      m25_wr_enable();

      spi_cs_0();

      spi_wr(SUBSECTOR_ERASE);

      ul2b.ul = flash_addr;
      spi_wr(ul2b.b[2]);
      spi_wr(ul2b.b[1]);
      spi_wr(ul2b.b[0]);

      spi_cs_1();

     //--- Wait the result

      m25_wait_for_result();

      m25_wr_disable();
   }

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
int m25_chk_sector_for_erase(__int32 flash_addr)
{
   union
   {
      unsigned long ul;
      unsigned char b[4];
   }ul2b;
   int nbytes;
   int fErased = TRUE;
   int res;

   nbytes = DF_SECTOR_SIZE;

   //--- Check sector for all FF

   spi_cs_0();

   spi_wr(FAST_READ);

   ul2b.ul = flash_addr;
   spi_wr(ul2b.b[2]);
   spi_wr(ul2b.b[1]);
   spi_wr(ul2b.b[0]);

   spi_wr(M25_DUMMY);  //-- for FAST_READ only

   while(nbytes--)
   {
      res = spi_wr(M25_DUMMY);
      if((res & 0xFF) != 0xFF)
      {
         fErased = FALSE;
         break;
      }
   }
   spi_cs_1();

   if(fErased == TRUE)
      return DF_SECTOR_ERASED;

   return DF_SECTOR_NOT_ERASED;
}

//----------------------------------------------------------------------------
int m25_read_bytes(unsigned char * buf, __int32 flash_addr, int nbytes)
{
   union
   {
      unsigned long ul;
      unsigned char b[4];
   }ul2b;
   unsigned char * ptr;

   spi_cs_0();

   spi_wr(READ_DATA_BYTES);

   ul2b.ul = flash_addr;
   spi_wr(ul2b.b[2]);
   spi_wr(ul2b.b[1]);
   spi_wr(ul2b.b[0]);

   ptr = buf;
   while(nbytes--)
    *ptr++ = spi_wr(M25_DUMMY);

   spi_cs_1();

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
//  flash_addr should be aligned to 256
//----------------------------------------------------------------------------
int m25_write_bytes(unsigned char * buf, __int32 flash_addr, int nbytes)
{
   union
   {
      unsigned long ul;
      unsigned char b[4];
   }ul2b;

   unsigned char * ptr;

   m25_wr_enable();

   spi_cs_0();

   spi_wr(PAGE_PROGRAM);

   ul2b.ul = flash_addr;
   spi_wr(ul2b.b[2]);
   spi_wr(ul2b.b[1]);
   spi_wr(ul2b.b[0]);

   ptr = buf;
   while(nbytes--)
      spi_wr(*ptr++);

   spi_cs_1();

 //--- Wait the result

   m25_wait_for_result();

   m25_wr_disable();

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
void m25_rd_id(unsigned char * manufacturer_id, //-- [OUT]
                 unsigned char * mem_type,        //-- [OUT]
                 unsigned char * mem_capacity)    //-- [OUT]
{
   if(manufacturer_id == 0 || mem_type == 0 || mem_capacity == 0)
      return;

   spi_cs_0();

   spi_wr(RD_ID);

   *manufacturer_id = spi_wr(M25_DUMMY);
   *mem_type        = spi_wr(M25_DUMMY);
   *mem_capacity    = spi_wr(M25_DUMMY);

   spi_cs_1();

}
//----------------------------------------------------------------------------
//  Local functions
//----------------------------------------------------------------------------
static void m25_wait_for_result(void)
{
 //--- Wait the result
   int res;
#ifndef USE_RTOS_DELAY
	volatile int dly;
#endif
	
   spi_cs_0();

   spi_wr(RD_STATUS_REG);

   for(;;)
   {
#ifdef USE_RTOS_DELAY		 
      os_delay();
#else		 
      for(dly=0; dly<500;dly++);
#endif
      res = spi_wr(M25_DUMMY);
      if((res & WIP) == 0)
         break;
   }

   spi_cs_1();
}

//----------------------------------------------------------------------------
static void m25_wr_enable(void)
{
   spi_cs_0();
   spi_wr(WR_ENABLE);
   spi_cs_1();
}

//----------------------------------------------------------------------------
static void m25_wr_disable(void)
{
   spi_cs_0();
   spi_wr(WR_DISABLE);
   spi_cs_1();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------






