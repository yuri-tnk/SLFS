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


#ifndef  _DF_FLASH_H_
#define  _DF_FLASH_H_


#define  DF_PAGE_SIZE              256
#define  DF_SECTOR_SIZE           4096

  //----
#define  DF_SECTOR_ERASED            1
#define  DF_SECTOR_NOT_ERASED        2



//--- Commands

#define  WR_ENABLE                 0x06   //--   WREN  Write enable
#define  WR_DISABLE                0x04   //--   WRDI  Write disable
#define  RD_ID                     0x9F   //--   Read  identification
#define  RD_STATUS_REG             0x05   //--   RDSR  Read status register
#define  WR_STATUS_REG             0x01   //--   WRSR  Write status register
#define  WR_LOCK_REG               0xE5   //--   WRLR  Write to lock register
#define  RD_LOCK_REG               0xE8   //--   RDLR  Read lock register
#define  READ_DATA_BYTES           0x03   //--   READ  Read data bytes
#define  FAST_READ                 0x0B   //--   FAST_READ  Read data bytes at higher speed
#define  PAGE_WRITE                0x0A   //--   PW  Page write
#define  PAGE_PROGRAM              0x02   //--   PP  Page program
#define  PAGE_ERASE                0xDB   //--   PE  Page erase
#define  SECTOR_ERASE              0xD8   //--   SE  Sector erase
#define  SUBSECTOR_ERASE           0x20   //--   SSE Subsector erase
#define  BULK_ERASE                0xC7   //--   BE  Bulk erase
#define  DEEP_POWER_DOWN           0xB9   //--   DP  Deep power-down
#define  RELEASE_DEEP_POWER_DOWN   0xAB   //--   RDP Release from deep power-down

#define  DREAD                     0x3B   //--   Dual Mode Read

//--- Status register bits

#define  WIP      1
#define  WEL  (1<<1)
#define  BP0  (1<<2)
#define  BP1  (1<<3)
#define  BP2  (1<<4)
#define  SRWD (1<<7)

#ifdef WIN32

#define  spi_wr(data)

#define  spi_cs_0()

#define  spi_cs_1()


#else

/*
//--- SPI operations - MSP40F6638

#define  spi_wr(data) SPIB1_send_byte(data) //cpld_spi_send_byte(data) 

#define  USB1S_CS_1   P9OUT |= BIT7;
#define  USB1S_CS_0   P9OUT &= ~BIT7;

#define  spi_cs_0()  USB1S_CS_0 \
                     _NOP();

#define  spi_cs_1()  USB1S_CS_1
*/

//--- SPI operations  (LPC17xx)

int spi_wr(unsigned char data);

#define  spi_cs_0()  rFIO0CLR |= (1<<16);   

#define  spi_cs_1()  rFIO0SET |= (1<<16); 


//--- RTOS(if any) operations

#define  os_delay()  tn_task_sleep(1);

#endif /* WIN32 */

#define  M25_DUMMY   0xFF

//--- Delay for exit deep power down mode

#define DELAY_30_US  200


     //--- df_flash.c

int m25_write_page(unsigned char * buf, __int32 flash_addr);
int m25_write_page_ex(unsigned char * buf, __int32 flash_addr);
int m25_read_page(unsigned char * buf, __int32 flash_addr);
int m25_read_sector(unsigned char * buf, __int32 flash_addr);
int m25_write_sector(unsigned char * buf, __int32 flash_addr);
int m25_erase_sector(__int32 flash_addr);
int m25_chk_sector_for_erase(__int32 flash_addr);
int m25_read_bytes(unsigned char * buf, __int32 flash_addr, int nbytes);
int m25_write_bytes(unsigned char * buf, __int32 flash_addr, int nbytes);
void m25_rd_id(unsigned char * manufacturer_id, //-- [OUT]
                 unsigned char * mem_type,        //-- [OUT]
                 unsigned char * mem_capacity);    //-- [OUT]


#endif


