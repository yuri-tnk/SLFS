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

/*============================================================================
  The basic SLFS test
*===========================================================================*/

#include <string.h>
#include <stdlib.h>
#include "../../../cpu/lpc17xx/LPC176x.h"
#include "../../../TNKernel/tn.h"
#include "../../../fs_common/flash_fs.h"
#include "../../../fs_common/sys_errorcodes.h"
#include "../../../fs_common/flash_fs_cfg.h"
#include "slfs_test.h"
#include "types.h"
#include "shell.h"

//----------- Tasks ----------------------------------------------------------

#define  TASK_UART0_RX_PRIORITY      7
#define  TASK_IO_PRIORITY           12
#define  TASK_LED_BLINK_PRIORITY    14

#define  TASK_UART0_RX_STK_SIZE    256
#define  TASK_IO_STK_SIZE          256
#define  TASK_LED_BLINK_STK_SIZE   256


#if defined (__ICCARM__)    // IAR ARM
#pragma data_alignment=8
#endif
align_attr_start unsigned int task_uart0_rx_stack[TASK_UART0_RX_STK_SIZE] align_attr_end;
#if defined (__ICCARM__)    // IAR ARM
#pragma data_alignment=8
#endif
align_attr_start unsigned int task_io_stack[TASK_IO_STK_SIZE]  align_attr_end;
#if defined (__ICCARM__)    // IAR ARM
#pragma data_alignment=8
#endif
align_attr_start unsigned int task_led_blink_stack[TASK_LED_BLINK_STK_SIZE]  align_attr_end;


TN_TCB  task_uart0_rx;
TN_TCB  task_io;
TN_TCB  task_led_blink;

void task_uart0_rx_func(void * par);
void task_io_func(void * par);
void task_led_blink_func(void * par);

//-------- Semaphores -----------------------

TN_SEM  semMeasPrf;

TN_SEM  semTxUart0;
TN_SEM  semFifoEmptyTxUART0;

TN_SEM  semCmdExec;

//------- Queues ----------------------------

#define  QUEUE_RX_UART0_SIZE      4

TN_DQUE  queueRxUART0;
void     * queueRxUART0Mem[QUEUE_RX_UART0_SIZE];

//------- Memory pools ----------------------------

//--- Memory pool element size must be aligned to 4
//-- In this example, the item size (16) is already align to 4
//-- but in common case, to force right item size align, use macro MAKE_ALIG(item_type)
//---   defined in the file tn_port.h

TN_FMP RxUART0MemPool;
#define RxUART0MemPoolItemSize   (UART_FIFO_SIZE)
unsigned int memRxUART0MemPool[QUEUE_RX_UART0_SIZE * (RxUART0MemPoolItemSize/ sizeof(int))];

//------------------------------ Non OS globals ------------------------------

//------------ Shell --------------------

int sh_exec_cmd_ver(SHELLINFO * sh_info);
int sh_exec_cmd_ping(SHELLINFO * sh_info);
int sh_exec_slfs_create(SHELLINFO * sh_info);
int sh_exec_slfs_dump_disks(SHELLINFO * sh_info);
int sh_exec_slfs_dump_files(SHELLINFO * sh_info);
int sh_exec_slfs_dump_dinfo(SHELLINFO * sh_info);
int sh_exec_slfs_chk_files(SHELLINFO * sh_info);
int sh_exec_slfs_create_files(SHELLINFO * sh_info);

#define SH_CL_SIZE       96
SHELLINFO g_shell_uart;
char * g_shell_argv[SH_MAX_ARG];
unsigned char g_shell_cl_buf[SH_CL_SIZE];

const SHELLCMD g_shell_cmd_arr[] =
{
   {"ver",              sh_exec_cmd_ver},
   {"fs_create",        sh_exec_slfs_create},
   {"fs_dump",          sh_exec_slfs_dump_disks},
   {"fs_files",         sh_exec_slfs_dump_files},
   {"fs_dinfo",         sh_exec_slfs_dump_dinfo},
   {"fs_chk_files",     sh_exec_slfs_chk_files},
   {"fs_create_files",  sh_exec_slfs_create_files}
};


//------------ SLFS ---------------------

DF_FLI g_fli;
//SFS_DISKINFO  g_di_A;    //-- Disk A - SFS disk
SLFS_DISKINFO  g_di_B;   //-- Disk B - SLFS disk
SLFS_DISKINFO  g_di_C;   //-- Disk C - SLFS disk
SLFS_DISKINFO  g_di_D;   //-- Disk D - SLFS disk

SLFS_FILEFIND g_hFileFind;
SLFS_FILE     g_hSLFSFile;

unsigned char g_dump_buf[MAX_TST_DUMP_STR_LEN];
unsigned long g_big_buf[1024];
unsigned long g_wr_buf[64];

//------------ UART0 --------------------
		
UARTDRV drvUART0;
unsigned char gUART0PayloadBuf[UART0_RX_PAYLOAD_BUF_SIZE];

//-------------------------------------------
int init_fli(DF_FLI * fli);
int create_flash_fs_file(DF_FLI * fli);
void dump_both_MBR_content(DF_FLI * fli, unsigned char * dump_buf);

void  tst_1_create_files(DF_FLI * fli,
                            SLFS_FILE  * hSLFSFile,
                            int disk_letter, 
                            int max_file_len,
                            unsigned char * wr_buf,
                            int num_files,
                            unsigned char * dump_buf,
                            SHELLINFO * sh_info);
void  tst_1_chk_files(DF_FLI * fli,
                      SLFS_FILE  * hSLFSFile,
                      int disk_letter, 
                      unsigned char * wr_buf,
                      int num_files,
                      unsigned char * dump_buf,
                      SHELLINFO * sh_info);

static void rtn_dump_mbr(unsigned char * buf, 
                         DF_MBR * pMBR, 
                         int mbr_num,
                         unsigned char * dump_buf);
static void dump_fs_entries(DF_FLI * fli, 
                            unsigned char * buf, 
                            unsigned char * dump_buf);



//----------------------------------------------------------------------------
int main(void)
{
   tn_arm_disable_interrupts();

//---- Non- OS  vars init

     //-- UART0 rx drv
   drvUART0.buf          = &gUART0PayloadBuf[0];
   drvUART0.max_buf_size = UART0_RX_PAYLOAD_BUF_SIZE;
   drvUART0.pos          = 0;
  
    //---- Init test random generator

   slfs_randomize();
//------------------------

   HardwareInit();

   tn_start_system(); //-- Never returns

   return 1;
}

//----------------------------------------------------------------------------
void  tn_app_init()
{
	
		//-- Shell
	
	 memset(&g_shell_uart, 0, sizeof(SHELLINFO));

   g_shell_uart.cl_size      = SH_CL_SIZE;
   g_shell_uart.cl_buf       = &g_shell_cl_buf[0];
   g_shell_uart.argv         = (unsigned char **)&g_shell_argv[0];
   g_shell_uart.num_cmd      = sizeof(g_shell_cmd_arr)/sizeof(SHELLCMD);
   g_shell_uart.sh_cmd_arr   = (SHELLCMD *)&g_shell_cmd_arr[0];
   g_shell_uart.sh_send_str  = uart0_send_str;
   g_shell_uart.sh_send_char = uart0_send_char;
  
	
   //--- Task Rx UART0

   task_uart0_rx.id_task = 0;
   tn_task_create(&task_uart0_rx,               //-- task TCB
                 task_uart0_rx_func,            //-- task function
                 TASK_UART0_RX_PRIORITY,        //-- task priority
                 &(task_uart0_rx_stack          //-- task stack first addr in memory
                    [TASK_UART0_RX_STK_SIZE-1]),
                 TASK_UART0_RX_STK_SIZE,        //-- task stack size (in int,not bytes)
                 NULL,                          //-- task function parameter
                 TN_TASK_START_ON_CREATION);    //-- Creation option

   //--- Task IO

   task_io.id_task = 0;
   tn_task_create(&task_io,                      //-- task TCB
                 task_io_func,                   //-- task function
                 TASK_IO_PRIORITY,               //-- task priority
                 &(task_io_stack                 //-- task stack first addr in memory
                    [TASK_IO_STK_SIZE-1]),
                 TASK_IO_STK_SIZE,               //-- task stack size (in int,not bytes)
                 NULL,                           //-- task function parameter
                 TN_TASK_START_ON_CREATION);     //-- Creation option


   //--- Task LED blink

   task_led_blink.id_task = 0;
   tn_task_create(&task_led_blink,               //-- task TCB
                 task_led_blink_func,            //-- task function
                 TASK_LED_BLINK_PRIORITY,        //-- task priority
                 &(task_led_blink_stack          //-- task stack first addr in memory
                    [TASK_LED_BLINK_STK_SIZE-1]),
                 TASK_LED_BLINK_STK_SIZE,        //-- task stack size (in int,not bytes)
                 NULL,                           //-- task function parameter
                 TN_TASK_START_ON_CREATION);     //-- Creation option

  //--- Queues

   queueRxUART0.id_dque = 0;
   tn_queue_create(&queueRxUART0,       //-- Ptr to already existing TN_DQUE
                  &queueRxUART0Mem[0],  //-- Ptr to already existing array of void * to store data queue entries.Can be NULL
                  QUEUE_RX_UART0_SIZE); //-- Capacity of data queue(num entries).Can be 0

   //--- Memory pools

   RxUART0MemPool.id_fmp = 0;
   tn_fmem_create(&RxUART0MemPool,
                 (void *)&memRxUART0MemPool[0], // start_addr
                 RxUART0MemPoolItemSize,
                 QUEUE_RX_UART0_SIZE);
                 
   //--- Semaphores

   semTxUart0.id_sem = 0;
   tn_sem_create(&semTxUart0,
                           1,   //-- Start value
                           1);  //-- Max value

   semMeasPrf.id_sem = 0;
   tn_sem_create(&semMeasPrf,
                           0,   //-- Start value
                           1);  //-- Max value

   semCmdExec.id_sem = 0;
   tn_sem_create(&semCmdExec,
                           1,   //-- Start value
                           1);  //-- Max value
}

//----------------------------------------------------------------------------
void task_uart0_rx_func(void * par)
{
   int rc;
   int i;
   int len;
   unsigned int rx_block;
   unsigned int mem_addr;
   unsigned char * rx_mem;

//	 sh_prompt(&g_shell_uart, FALSE); 
	
   for(;;)
   {
      rc = tn_queue_receive(&queueRxUART0, (void **)&rx_block, TN_WAIT_INFINITE);
      if(rc == TERR_NO_ERR)
      {
           //--- Unpack len & addr

         len = ((unsigned int)rx_block) >> 24;
         mem_addr = (((unsigned int)&memRxUART0MemPool[0]) & 0xFF000000) |
                                   (((unsigned int)rx_block) & 0x00FFFFFF);
         rx_mem = (unsigned char *) mem_addr;

         for(i = 0; i < len; i++)
         {
 	          rc = sh_input(&g_shell_uart, rx_mem[i]);
            if(rc == SH_EXECUTE)
               sh_start_cmd_exec(&g_shell_uart);
            else if(rc == SH_BREAK)
               sh_stop_cmd_exec(&g_shell_uart);
         }
         tn_fmem_release(&RxUART0MemPool,(void*)rx_mem);
      }
   }
}

//----------------------------------------------------------------------------
void task_io_func(void * par)
{
	 int rc;
  
   //-- File system(s)
  
   rc = init_fli(&g_fli);
   tn_snprintf((char*)g_dump_buf, MAX_DUMP_STR_LEN, "\r\ninit_fli: %d\r\n",rc);
   dump_str(g_dump_buf);
  
   rc = df_init_disks(&g_fli);
   tn_snprintf((char*)g_dump_buf, MAX_DUMP_STR_LEN, "df_init_disks(): %d\r\n\r\n",rc);
   dump_str(g_dump_buf);
  
//------------------------------------------
   for(;;)
   {
      rc = tn_sem_acquire(&semCmdExec, TN_WAIT_INFINITE);
      if(rc == TERR_NO_ERR)
      {
         sh_do_cmd_exec(&g_shell_uart);
      }				
   }
}

//----------------------------------------------------------------------------
void task_led_blink_func(void * par)
{
   int cnt = 0;
  
   for(;;)
   {
      if(cnt & 1)
         rFIO1SET |= LED_MASK;
      else 
         rFIO1CLR |= LED_MASK;

      tn_task_sleep(167);				 
      cnt++;
   }
}

//----------------------------------------------------------------------------
void m25_rd_id(unsigned char * manufacturer_id, //-- [OUT]
                 unsigned char * mem_type,        //-- [OUT]
                 unsigned char * mem_capacity);   //-- [OUT]
//----------------------------------------------------------------------------
int sh_exec_cmd_ver(SHELLINFO * sh_info)
{
   unsigned char manufacturer_id = 0;
   unsigned char mem_type = 0;       
   unsigned char mem_capacity = 0;
   char name_buf[16];
  
   m25_rd_id(&manufacturer_id,
             &mem_type,       
             &mem_capacity);

   //--- For M25PX64  (Numonyx)

   if(manufacturer_id == 0x20 && mem_type == 0x71 && mem_capacity == 0x17)
   {
      strcpy(name_buf, "M25PX64");
   }
   else
   {
      strcpy(name_buf, "Unknown");
   }     

  
   tn_snprintf((char *)g_dump_buf, MAX_DUMP_STR_LEN,
              "\r\n id: 0x%02X type: 0x%02X capacity: 0x%02X  %s\r\n",   
               manufacturer_id,
               mem_type,       
               mem_capacity,
               name_buf);
   dump_str(g_dump_buf);
  
   return 0; 	
}

//----------------------------------------------------------------------------
//      SLFS init/dump/test functions
//----------------------------------------------------------------------------
int init_fli(DF_FLI * fli)
{
   //memset(&g_di_A, 0, sizeof(SFS_DISKINFO));
   memset(&g_di_B, 0, sizeof(SLFS_DISKINFO));
   memset(&g_di_C, 0, sizeof(SLFS_DISKINFO));
   memset(&g_di_D, 0, sizeof(SLFS_DISKINFO));

   //fli->di_arr[0] = (DF_DISKINFO *)  &g_di_A;
   fli->di_arr[1] = (DF_DISKINFO *)  &g_di_B;
   fli->di_arr[2] = (DF_DISKINFO *)  &g_di_C;
   fli->di_arr[3] = (DF_DISKINFO *)  &g_di_D;

   fli->big_buf       = (unsigned char *)&g_big_buf[0];
   fli->sector_size   = (long)DF_SECTOR_SIZE;
   fli->sectors       = DF_FLASH_SECTORS;

   fli->semSFS.id_sem = 0;
   tn_sem_create(&fli->semSFS,
                           1,   //-- Start value
                           1);  //-- Max value

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
int create_flash_fs(DF_FLI * fli)
{
   int rc;
   __int32 disk_size_sectors;

   rc = init_fli(fli);
   if(rc != ERR_NO_ERR)
     return rc;

   rc = df_create_mbr(fli, "M25PX64");
   if(rc != ERR_NO_ERR)
      return rc;

 //-- The disk letter follows the disk creation order
 //-- The disk created first will have the letter 'A', second - 'B' etc.    

 //--- Now - disk A 

   disk_size_sectors =  (unsigned long)DF_DISK_A_SECTORS;

   rc = df_create_mbr_disk_entry(fli,
                                 disk_size_sectors,
                                 0); //-- 0 - reserved space
   if(rc != ERR_NO_ERR)
      return rc;

 //--- Now - disk B

   disk_size_sectors =  (unsigned long)300;  //DF_DISK_B_SECTORS;

   rc = df_create_mbr_disk_entry(fli,
                                 disk_size_sectors,
                                 FS_TYPE_SLFS); //-- except 1st and last sector( both- MBR)
   if(rc != ERR_NO_ERR)
      return rc;

   rc = slfs_create_disk(fli,
                         'B');      //created as second disk - letter is 'B'

   if(rc != ERR_NO_ERR)
      return rc;

 //--- Now - disk C

   disk_size_sectors =  (unsigned long)400;  //DF_DISK_C_SECTORS;

   rc = df_create_mbr_disk_entry(fli,
                                 disk_size_sectors,
                                 FS_TYPE_SLFS); //-- except 1st and last sector( both- MBR)
   if(rc != ERR_NO_ERR)
      return rc;

   rc = slfs_create_disk(fli,
                         'C');      //created as disk N3 - letter is 'C'
   if(rc != ERR_NO_ERR)
      return rc;

 //--- Now - disk D

   disk_size_sectors =  (unsigned long)127;  //DF_DISK_D_SECTORS;

   rc = df_create_mbr_disk_entry(fli,
                                 disk_size_sectors,
                                 FS_TYPE_SLFS); //-- except 1st and last sector( both- MBR)
   if(rc != ERR_NO_ERR)
      return rc;

   rc = slfs_create_disk(fli,
                         'D');      //created as disk N4 - letter is 'D'

   if(rc != ERR_NO_ERR)
      return rc;

   return ERR_NO_ERR;
}


//----------------------------------------------------------------------------
void dump_both_MBR_content(DF_FLI * fli, unsigned char * dump_buf)
{
   DF_MBR * pMBR = NULL;
   unsigned char * buf;
   int rc;
 
   buf = fli->big_buf;
   pMBR = (DF_MBR *)buf;

   rc = sfsi_read_page_ex(fli, buf, DF_1ST_SECTOR);
   if(rc != ERR_NO_ERR)
   {
      dump_str((unsigned char*)"Failed read MBR 0\r\n");
      return;
   }
   rtn_dump_mbr(buf, pMBR, 0, dump_buf);

   rc = sfsi_read_page_ex(fli, buf, DF_LAST_SECTOR);
   if(rc != ERR_NO_ERR)
   {
      dump_str((unsigned char*)"Failed read MBR 1\r\n");
      return;
   }
   dump_str((unsigned char*)"\r\n");
   rtn_dump_mbr(buf, pMBR, 1, dump_buf);
   
  //-- If here do dump all FS info

   dump_fs_entries(fli, buf, dump_buf);
}

//----------------------------------------------------------------------------
static void rtn_dump_mbr(unsigned char * buf, 
                         DF_MBR * pMBR, 
                         int mbr_num,
                         unsigned char * dump_buf)
{
   unsigned __int32 crc1;

   crc1 = sfsi_chk_crc(buf + DF_CRC_SIZE,
                         DF_PAGE_SIZE - DF_CRC_SIZE);
  
   tn_snprintf((char*)dump_buf, MAX_TST_DUMP_STR_LEN, "MBR N: %d\r\n", mbr_num);
   dump_str(dump_buf);
   dump_str((unsigned char*)"---------------------------------------\r\n");
   tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "Calc CRC: %08X\r\n", crc1);
   dump_str(dump_buf);
   tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "crc:      %08X\r\n", pMBR->mbr_rec.crc);
   dump_str(dump_buf);
   tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "Name:        %s\r\n", pMBR->mbr_rec.name);
   dump_str(dump_buf);
   tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "sector_size: %d\r\n", pMBR->mbr_rec.sector_size);
   dump_str(dump_buf);
   tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "sectors:     %d\r\n", pMBR->mbr_rec.sectors);
   dump_str(dump_buf);

}
//----------------------------------------------------------------------------
static void dump_fs_entries(DF_FLI * fli, 
                            unsigned char * buf, 
                            unsigned char * dump_buf)
{
   int disk_num = 1;
   DF_MBRENTRY * pEntry = NULL;
   DF_MBR * pMBR = NULL;
   int rc;

   rc = sfsi_read_page_ex(fli, buf, DF_1ST_SECTOR);
   if(rc != ERR_NO_ERR)
   {
      dump_str((unsigned char*)("Failed to read MBR 0.\r\n"));
      return;
   }

   for(;;)
   {
      pEntry  = (DF_MBRENTRY *) (buf + DF_FIRST_DISK_ENTRY_OFFSET +
                                     sizeof(DF_MBRENTRY) * (disk_num - 1));
      if(pMBR == NULL)
      {
         pMBR = (DF_MBR *)buf;
         fli->sector_size = pMBR->mbr_rec.sector_size;
         fli->sectors     = pMBR->mbr_rec.sectors;
      }

      if(pEntry->SectorZero == DF_EMPTY_BYTES_LONG &&    //-- Empty -> last
             pEntry->TotalSectors == DF_EMPTY_BYTES_LONG)
      {
         break;
      }
      else if(pEntry->SectorZero != DF_EMPTY_BYTES_LONG &&    //-- Not empty
                 pEntry->TotalSectors != DF_EMPTY_BYTES_LONG)
      {
         if(disk_num > DF_MAX_DISKS)
         {
            dump_str((unsigned char *)"Too much disks.\r\n");
            break;
         }

         tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "\r\n    Disk: %c\r\n", 'A' + (disk_num-1));
         dump_str(dump_buf);
         dump_str((unsigned char*)"    --------------\r\n");

         tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "       SectorZero:    %d\r\n", pEntry->SectorZero);
         dump_str(dump_buf);
         tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "       TotalSectors:  %d\r\n", pEntry->TotalSectors); 
         dump_str(dump_buf);
         switch(pEntry->FSType)
         {
            case 0:

               dump_str((unsigned char*)"       FSType: 0 (no FS)\r\n");
               break;  

            case FS_TYPE_SLFS:

               tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "       FSType: %08Xh (SLFS)\r\n", pEntry->FSType);
               dump_str(dump_buf);
               break;  

            default:

               tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "       FSType: %08Xh (unknown FS)\r\n", pEntry->FSType);
               dump_str(dump_buf);
         }
         disk_num++;
         continue;   
      }
      else
      {
         dump_str((unsigned char*)"Entry corrupted.\r\n");
         break;
      }
   }
}


//----------------------------------------------------------------------------
void  tst_1_create_files(DF_FLI * fli,
                            SLFS_FILE  * hSLFSFile,
                            int disk_letter, 
                            int max_file_len,
                            unsigned char * wr_buf,
                            int num_files,
                            unsigned char * dump_buf,
                            SHELLINFO * sh_info)
{                           
   int i;
   int rc;
   char name_buf[16];

   for(i=0; i < num_files; i++)
   {
      tn_snprintf(name_buf,15,"file_%d",i);
      rc = create_random_file_slfs(fli,
                                   hSLFSFile,
                                   disk_letter, 
                                   (unsigned char*)name_buf,
                                   max_file_len,
                                   wr_buf);
      if(rc == ERR_NO_ERR)
      {
         tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "File created: %d\r\n", i);
         dump_str(dump_buf);
        
         tn_task_sleep(1);
      }        
      else
      {
         tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "Failed create random file. Error: %d File: %d\r\n", rc, i);
         dump_str(dump_buf);
         break;  
      }
      
      if(sh_info->stop_exec == TRUE)
      {
         sh_info->stop_exec = FALSE;
         dump_str((unsigned char *)"Operation canceled by user.\r\n");
         break;
      }        
   } 
}

//----------------------------------------------------------------------------
void  tst_1_chk_files(DF_FLI * fli,
                      SLFS_FILE  * hSLFSFile,
                      int disk_letter, 
                      unsigned char * wr_buf,
                      int num_files,
                      unsigned char * dump_buf,
                      SHELLINFO * sh_info)
{                           
   int i;
   int rc;
   char name_buf[24];

   for(i=0; i < num_files; i++)
   {

      rc = chk_random_file_slfs(fli, 
                                disk_letter, 
                                hSLFSFile,
                                wr_buf);
      if(rc == ERR_NO_ERR)
      {
         tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "Check OK. File num: %d\r\n", i);
         dump_str(dump_buf);
        
         tn_task_sleep(1);
      }        
      else
      {
         if(rc == ERR_DISK_EMPTY)
            strcpy(name_buf, "(Disk is empty)");
         else
            strcpy(name_buf, "");
         tn_snprintf((char*)dump_buf, MAX_DUMP_STR_LEN, "\r\nCheck failed. Error: %d %s File num: %d\r\n", 
                      rc, name_buf, i);
         dump_str(dump_buf);
         return;  
      }   
      
      if(sh_info->stop_exec == TRUE)
      {
         sh_info->stop_exec = FALSE;
         dump_str((unsigned char *)"Operation canceled by user.\r\n");
         return;
      }        
   }

   dump_str((unsigned char*)"\r\nAll checks - OK.\r\n");
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int sh_exec_slfs_create(SHELLINFO * sh_info)
{
   int rc;

   dump_str((unsigned char*)"\r\nStart formatting. It can take a few minutes...\r\n");

  //--- Format flash & Create SLFS
   rc = init_fli(&g_fli);
   tn_snprintf((char*)g_dump_buf, MAX_DUMP_STR_LEN, "init_fli: %d\r\n",rc);
   dump_str(g_dump_buf);

   rc = create_flash_fs(&g_fli);
   tn_snprintf((char*)g_dump_buf, MAX_DUMP_STR_LEN, "create_flash_fs(): %d\r\n", rc);
   dump_str(g_dump_buf);
  
   rc = df_init_disks(&g_fli);
   tn_snprintf((char*)g_dump_buf, MAX_DUMP_STR_LEN, "df_init_disks(): %d\r\n",rc);
   dump_str(g_dump_buf);
  
   return 0; 	
}

//----------------------------------------------------------------------------
int sh_exec_slfs_dump_disks(SHELLINFO * sh_info)
{
   dump_both_MBR_content(&g_fli, &g_dump_buf[0]);  
  
   return 0; 	
}
//----------------------------------------------------------------------------
int sh_exec_slfs_dump_files(SHELLINFO * sh_info)
{
   dump_all_files_slfs(&g_fli, &g_hSLFSFile, 'B', &g_dump_buf[0]);
  
   return 0; 	
}  
//----------------------------------------------------------------------------
int sh_exec_slfs_dump_dinfo(SHELLINFO * sh_info)
{
   dump_di_slfs(&g_di_B, &g_dump_buf[0]);
  
   return 0; 	
}  

//----------------------------------------------------------------------------
int sh_exec_slfs_chk_files(SHELLINFO * sh_info)
{
   int num_files = 0;  
   
   if(sh_info->argv[1])
   {
      num_files = atoi((char*)sh_info->argv[1]);
      tn_snprintf((char*)g_dump_buf, MAX_DUMP_STR_LEN, "\r\nNum files to check: %d\r\n\r\n", num_files);
      dump_str(g_dump_buf);
      if(num_files <= 0)
      {
         dump_str((unsigned char*) "Wrong num files.\r\n");
         return 0;
      }     
   }     
   else
   {
      dump_str((unsigned char*) "Num files ?\r\n");
      return 0;
   }     
  
   tst_1_chk_files(&g_fli,
                   &g_hSLFSFile,
                   'B', 
                   (unsigned char *)g_wr_buf, // wr_buf,
                   num_files,
                   &g_dump_buf[0],
                   sh_info);
   return 0; 	
}
 
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int sh_exec_slfs_create_files(SHELLINFO * sh_info)
{
   int num_files = 0;  
   // ToDo - get max_file_len from cmd line
   int max_file_len = 4096 - DF_PAGE_SIZE - sizeof(__int32); // sizeof(__int32) is CRC32 size, append to file   
   if(sh_info->argv[1])
   {
      num_files = atoi((char*)sh_info->argv[1]);
      tn_snprintf((char*)g_dump_buf, MAX_DUMP_STR_LEN, "\r\nNum files to create: %d\r\n\r\n", num_files);
      dump_str(g_dump_buf);
      if(num_files <= 0)
      {
         dump_str((unsigned char*) "Wrong num files.\r\n");
         return 0;
      }     
   }     
   else
   {
      dump_str((unsigned char*) "Num files ?\r\n");
      return 0;
   }     
  
   tst_1_create_files(&g_fli,
                      &g_hSLFSFile,
                      'B',    // disk_letter, 
                      max_file_len, 
                      (unsigned char *)g_wr_buf,
                      num_files,
                      &g_dump_buf[0],
                      sh_info);
   return 0; 	
} 
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

