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

#include  <string.h>     //-- for IAR
#include  "../fs_common/sys_errorcodes.h"
#include  "../fs_common/flash_fs.h"
#include  "../fs_common/flash_fs_cfg.h"

extern /*__no_init*/ unsigned char g_dump_buf[];
int tn_snprintf( char *outStr, int maxLen, const char *fmt, ... );
void dump_str(unsigned char * str);

//#define DEBUG_CONSOLE 1

//----------------------------------------------------------------------------
int slfs_create_disk(DF_FLI * fli,
                     int disk_letter)  //-- 'A', 'B', 'C', 'D'
{
   DF_DISKINFO * df_di;
   SLFS_DISKINFO * di;
   DF_MBRENTRY * pEntry;
   SLFS_LOGDISKDATA * ldat;

   __int32 first_sector;
   __int32 last_sector;
   __int32 flash_addr;
   unsigned __int32 tmp;
   unsigned __int32 crc;
   int rc;
   int i;
   int disk_num; 

   //-- Find disk

   //-- Now we support up to 4 disks
   switch(disk_letter)
   {
      case 'A':
      case 'a':
         disk_num = 1;
         break;
      case 'B':
      case 'b':
         disk_num = 2;
         break;
      case 'C':
      case 'c':
         disk_num = 3;
         break;
      case 'D':
      case 'd':
         disk_num = 4;
         break;
      default:
         return ERR_WRONG_PARAM;
   }

   //------ Add disk info memory block to vol  ----------

   di = (SLFS_DISKINFO *)fli->di_arr[disk_num - 1];
   if(di == NULL)
      return ERR_DISKINFO_NOT_ASSIGNED;

   memset(di, 0, sizeof(SLFS_DISKINFO)); //-- SectorZero will be 0
   df_di = &di->df_di;
   df_di->disk_num = disk_num;

 //---------- Fill on-disk struct values ---------

   //-- Get from mbr entry ZeroSector and TotalSectors

   rc = sfsi_read_page_ex(fli, (unsigned char *)df_di->buf, DF_1ST_SECTOR);
   if(rc != ERR_NO_ERR)
      return rc;

   //-- ToDo - check crc here

   pEntry = (DF_MBRENTRY *)((unsigned char *)df_di->buf +
                          DF_FIRST_DISK_ENTRY_OFFSET
                          + sizeof(DF_MBRENTRY) * (df_di->disk_num - 1));

   if(pEntry->SectorZero == DF_EMPTY_BYTES_LONG &&    //-- Empty
              pEntry->TotalSectors == DF_EMPTY_BYTES_LONG)
   {
      return ERR_ENTRY_CORRUPTED;
   }
   else if(pEntry->SectorZero != DF_EMPTY_BYTES_LONG &&    //-- Not empty
              pEntry->TotalSectors != DF_EMPTY_BYTES_LONG)
   {
      df_di->disk_sb.SectorZero   = pEntry->SectorZero;
      df_di->disk_sb.TotalSectors = pEntry->TotalSectors;
   }
   else
      return ERR_ENTRY_CORRUPTED;

   //--- Fill Super Block info
   memcpy((char*)&tmp, (char*)"_YT_", 4);
   df_di->disk_sb.magic_num = tmp;

   df_di->disk_sb.FileSysVersion     = 4001 | FS_TYPE_SLFS;

   df_di->disk_sb.SectorSize         = fli->sector_size;  //DF_SECTOR_SIZE;

   df_di->disk_sb.InodesStartSector  = df_di->disk_sb.SectorZero + 1; //-- Inodes starts from sector 1(sector 0 - 1st SB)

   df_di->disk_sb.SectorsPerInodes   =  0; //-- SLFS

   df_di->disk_sb.LogAreaStartSector = df_di->disk_sb.InodesStartSector + df_di->disk_sb.SectorsPerInodes;
   df_di->disk_sb.SectorsPerLogArea  = SLFS_LOG_SECTORS_NUM * SLFS_LOG_MAX_POSITIONS;

   df_di->disk_sb.LogStartSector  = df_di->disk_sb.LogAreaStartSector; //-- The first Log Position
   df_di->disk_sb.SectorsPerLog   = SLFS_LOG_SECTORS_NUM;

   df_di->disk_sb.DataStartSector = df_di->disk_sb.LogAreaStartSector + df_di->disk_sb.SectorsPerLogArea;
   df_di->disk_sb.SectorsPerData  = df_di->disk_sb.TotalSectors - df_di->disk_sb.SectorsPerLogArea -
                                    df_di->disk_sb.SectorsPerInodes - 2; //-- 2 here is 1st and 2nd SB

   df_di->disk_sb.PagesPerBmp = 0;   //-- SLFS

   df_di->disk_sb.SecondSBSector = df_di->disk_sb.SectorZero + df_di->disk_sb.TotalSectors - 1;

   df_di->disk_sb.Crc = sfsi_chk_crc(((unsigned char *)&df_di->disk_sb) + DF_CRC_SIZE,
                                   sizeof(DF_SB) - DF_CRC_SIZE);

   //--- Makes two copies of Super Block

   memset((char *)df_di->buf, 0, DF_PAGE_SIZE);  //-- set mem to 0
   memcpy((char *)df_di->buf, &df_di->disk_sb, sizeof(DF_SB));

   //--- Sectors must be pre-erased

     //--- Wr 1st copy - at addr di->SectorZero

   flash_addr = df_di->disk_sb.SectorZero * DF_SECTOR_SIZE;
   rc = sfsi_write_page(df_di, (unsigned char *)df_di->buf, flash_addr);
   if(rc != ERR_NO_ERR)
      return rc;
           //--- Wr 2nd copy - at addr (sectors - 1)
   flash_addr = df_di->disk_sb.SecondSBSector * DF_SECTOR_SIZE;
   rc = sfsi_write_page(df_di, (unsigned char *)df_di->buf, flash_addr);
   if(rc != ERR_NO_ERR)
      return rc;

   //--- Fill SysLog -----

      //-- Erase overall LogArea

   first_sector = df_di->disk_sb.LogAreaStartSector;
   last_sector  = df_di->disk_sb.LogAreaStartSector + df_di->disk_sb.SectorsPerLogArea;
   for(i= first_sector;i < last_sector; i++)
   {
      rc = sfsi_erase_sector(df_di, i);
      if(rc != ERR_NO_ERR)
         return rc;
   }

   //-- Set initial system vars values  and init log sys vars

   memset(df_di->buf, 0, DF_CRC_SIZE + sizeof(SLFS_LOGDISKDATA));

   ldat = (SLFS_LOGDISKDATA *)((unsigned char *)df_di->buf + DF_CRC_SIZE);

   ldat->tail_sector = df_di->disk_sb.DataStartSector;
   ldat->head_sector = df_di->disk_sb.DataStartSector;

   ldat->log_wr_cnt  = DF_MAX_NUM_ERASING;

   crc = sfsi_chk_crc((unsigned char*)df_di->buf + DF_CRC_SIZE,
                       sizeof(SLFS_LOGDISKDATA));

   memcpy((unsigned char*)df_di->buf, &crc, DF_CRC_SIZE);


      //-- Wr to log sector 1

   flash_addr = (df_di->disk_sb.LogStartSector + 1) * df_di->disk_sb.SectorSize;
   rc = sfsi_write_page(df_di, (unsigned char *)df_di->buf, flash_addr);
   if(rc != ERR_NO_ERR)
      return rc;

   //---------------------

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
int slfs_init_disk(DF_FLI * fli,
                  int disk_num,     //-- 1,2,3..
                  DF_MBRENTRY * pEntry)
{
   DF_DISKINFO * df_di;
   SLFS_DISKINFO * di;
   DF_SB * sb;
   __int32 flash_addr;
   unsigned __int32 crc;
   unsigned __int32 crc_ref;
   int rc;

   if(disk_num < 1 || disk_num > DF_NUM_DISKS)
      return ERR_WRONG_PARAM;

   di = (SLFS_DISKINFO *)fli->di_arr[disk_num - 1];
   if(di == NULL)
      return ERR_WRONG_PARAM;
   df_di = &di->df_di;

   //--- Init memory

   memset(di, 0, sizeof(SLFS_DISKINFO));

   df_di->disk_sb.SectorZero   = pEntry->SectorZero;
   df_di->disk_sb.TotalSectors = pEntry->TotalSectors;
   df_di->disk_sb.SectorSize   = fli->sector_size;

   df_di->fli = fli;


   //--- Read on-disk SuperBlock info

   flash_addr = pEntry->SectorZero * (__int32)fli->sector_size;
   rc = sfsi_read_page(df_di, (unsigned char*)df_di->buf, flash_addr);
   if(rc != ERR_NO_ERR)
      return rc;

   sb = (DF_SB *)df_di->buf;

   crc = sfsi_chk_crc((unsigned char*)df_di->buf + DF_CRC_SIZE,
                              sizeof(DF_SB) - DF_CRC_SIZE);
   if(crc != sb->Crc) //-- bad 1st sb crc
   {
     
//-------------------------------------------
#ifdef DEBUG_CONSOLE        
//tn_snprintf( (char *)g_dump_buf, MAX_TST_DUMP_STR_LEN1,
//     "LogStartSector + 1=%lu SectorSize=%lu flash_addr=%ld rc=%d\r\n",
//     df_di->disk_sb.LogStartSector + 1L,
//     df_di->disk_sb.SectorSize,
//     flash_addr,
//     rc);
  dump_str("SLFS - bad 1st sb crc\r\n");
#endif
//-------------------------------------------

       //-- 2nd sb - at last disk sector, page 0

      flash_addr = (pEntry->SectorZero + pEntry->TotalSectors - 1L) *
                                                     (__int32)fli->sector_size;
      rc = sfsi_read_page(df_di, (unsigned char*)df_di->buf, flash_addr);
      if(rc != ERR_NO_ERR)
         return rc;

      crc = sfsi_chk_crc((unsigned char*)df_di->buf + DF_CRC_SIZE,
                                         sizeof(DF_SB) - DF_CRC_SIZE);
      if(crc == sb->Crc) //-- good 2nd sb crc
      {
         //-- Copy 2nd SB to 1st SB
//-------------------------------------------
#ifdef DEBUG_CONSOLE        
   dump_str("SLFS - 2nd sb crc OK!!!\r\n");
#endif
//-------------------------------------------

         rc = sfsi_erase_sector(df_di, pEntry->SectorZero);
         if(rc != ERR_NO_ERR)
            return ERR_NO_ERR;

         flash_addr = pEntry->SectorZero * (__int32)fli->sector_size;
         rc = sfsi_write_page(df_di, (unsigned char*)df_di->buf, flash_addr);
         if(rc != ERR_NO_ERR)
            return rc;
      }
      else //-- bad 2nd sb crc
         return ERR_BAD_BOTH_SB; //-- Fatal
   }
   else //-- 1st crc - ok
   {
//-------------------------------------------
#ifdef DEBUG_CONSOLE        
   tn_snprintf((char *)g_dump_buf, MAX_DUMP_STR_LEN,
               "SLFS - 1st sb crc OK %08X\r\n", sb->Crc);
   dump_str(g_dump_buf);
#endif
//-------------------------------------------

       //-- 2nd sb - at last disk sector, page 0
      flash_addr = (pEntry->SectorZero + pEntry->TotalSectors - 1) * (__int32)fli->sector_size;
      rc = sfsi_read_page(df_di, (unsigned char*)df_di->buf, flash_addr);
      if(rc != ERR_NO_ERR)
         return rc;

      crc = sfsi_chk_crc((unsigned char*)df_di->buf + DF_CRC_SIZE,
                         sizeof(DF_SB) - DF_CRC_SIZE);
      if(crc != sb->Crc) //-- bad 2nd sb crc
      {
         __int32 sector;

//-------------------------------------------
#ifdef DEBUG_CONSOLE        
   dump_str("SLFS - bad 2nd sb crc\r\n");
#endif
//-------------------------------------------

         flash_addr = pEntry->SectorZero * (__int32)fli->sector_size;
         rc = sfsi_read_page(df_di, (unsigned char*)df_di->buf, flash_addr);
         if(rc != ERR_NO_ERR)
            return rc;

         //-- Copy 1st SB to 2nd SB

         sector = pEntry->SectorZero + pEntry->TotalSectors - 1L;

         rc = sfsi_erase_sector(df_di, sector);
         if(rc != ERR_NO_ERR)
            return ERR_NO_ERR;

              //-- 2nd SB - at last volume sector
         flash_addr = sector * (__int32)fli->sector_size;
         rc = sfsi_write_page(df_di, (unsigned char*)df_di->buf, flash_addr);
         if(rc != ERR_NO_ERR)
            return rc;
      }
//-------------------------------------------
#ifdef DEBUG_CONSOLE        
   tn_snprintf((char *)g_dump_buf, MAX_DUMP_STR_LEN,
               "SLFS - 2nd sb crc OK %08X\r\n", sb->Crc);
   dump_str(g_dump_buf);
#endif
//-------------------------------------------
   }

  //------ Calculate and fill ------

   memcpy(&df_di->disk_sb, sb, sizeof(DF_SB));
   df_di->disk_num  = disk_num;  //-- 1 - 'A', 2 - 'B', 3 - 'C' etc

   //--- Flush the Disk Log ------

   rc = slfs_flush_log(di);
   if(rc != ERR_NO_ERR)
   {  
      if(rc != ERR_BAD_CRC)
         return rc;
   }   
   //-------------------------

   di->start_data_flash_addr = df_di->disk_sb.DataStartSector * df_di->disk_sb.SectorSize;
   di->end_data_flash_addr   = (df_di->disk_sb.SecondSBSector - 1) * df_di->disk_sb.SectorSize;

   //--- Now - get Log data for local operating

   flash_addr = (df_di->disk_sb.LogStartSector + 1) * df_di->disk_sb.SectorSize;
   rc = sfsi_read_page(df_di, (unsigned char *)df_di->buf, flash_addr);
   if(rc != ERR_NO_ERR)
      return rc;

   //--ToDo chk crc here

   memcpy(&di->ld,
          (unsigned char *)df_di->buf + DF_CRC_SIZE,
          sizeof(SLFS_LOGDISKDATA));

   crc = sfsi_chk_crc((unsigned char*)&di->ld, sizeof(SLFS_LOGDISKDATA));
   memcpy(&crc_ref, (unsigned char *)df_di->buf, DF_CRC_SIZE);
   if(crc != crc_ref)
   {
//-------------------------------------------
#ifdef DEBUG_CONSOLE        
   dump_str("SLFS - bad SLFS_LOGDISKDATA crc\r\n");
#endif
//-------------------------------------------

      return ERR_BAD_CRC;
   }
   return rc;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------




