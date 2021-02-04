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

#include <string.h>
#include "../fs_common/sys_errorcodes.h"
#include "../fs_common/flash_fs.h"

extern unsigned char g_dump_buf[];
int tn_snprintf( char *outStr, int maxLen, const char *fmt, ... );
void dump_str(unsigned char * str);

//#define DEBUG_CONSOLE 1
#define DEBUG_LOG_SHIFT 1

//----------------------------------------------------------------------------
int slfs_prepare_log(SLFS_FILE * hFile, int mode)
{
   DF_DISKINFO * df_di;
   SLFS_DISKINFO * di;
   __int32 log_flash_addr;
   unsigned __int32 crc;
   int rc;
	
   if(mode != DF_APPEND && mode != DF_READ)
      return ERR_WRONG_PARAM;

   di = hFile->di;
   if(di == NULL)
      return ERR_WRONG_PARAM;

   df_di =&di->df_di;
   
      //--- Move log - here
   
   rc = slfs_move_log(di);
   if(rc != ERR_NO_ERR)
   {
//---------------------------------------------------        
#ifdef DEBUG_CONSOLE 
   dump_str("Main log - bad CRC\r\n");
#endif
//---------------------------------------------------        
      return rc;
   }

   //-- stage 1 - file header

   if(mode == DF_APPEND)
   {
      //-- update both file header and log info

      memcpy((unsigned char*)df_di->buf + DF_CRC_SIZE, &hFile->fheader, sizeof(SLFS_FILE_HEADER));

      di->ld.n_write++;
      di->ld.n_files++;

      di->ld.op_mode = DF_APPEND;  //-- mode = DF_READ;
   }
   else if(mode == DF_READ)
   {
      //-- Set fileheader to 0 - not used here

      memset((unsigned char*)df_di->buf + DF_CRC_SIZE, 0, sizeof(SLFS_FILE_HEADER));

      //-- update just log data

      di->ld.op_mode = DF_READ;

      if(di->ld.n_files > 0)
         di->ld.n_files--;

      di->ld.n_read++;
   }

   //-- stage 2 - log data
   if(di->ld.log_wr_cnt > 0L)
      di->ld.log_wr_cnt--;

   memcpy((unsigned char*)df_di->buf + DF_CRC_SIZE + sizeof(SLFS_FILE_HEADER),
           &di->ld,
           sizeof(SLFS_LOGDISKDATA));

   //-- stage 2 - crc

   crc = sfsi_chk_crc((unsigned char*)df_di->buf + DF_CRC_SIZE,
                       sizeof(SLFS_FILE_HEADER) + sizeof(SLFS_LOGDISKDATA));

   memcpy((unsigned char*)df_di->buf, &crc, DF_CRC_SIZE);
   
  //-- Wr log to log tmp data - log sector 0

   rc = sfsi_erase_sector(df_di, df_di->disk_sb.LogStartSector);
   if(rc != ERR_NO_ERR)
      return ERR_NO_ERR;

   log_flash_addr = df_di->disk_sb.LogStartSector * df_di->disk_sb.SectorSize;
   rc = sfsi_write_page(df_di, (unsigned char *)df_di->buf, log_flash_addr);

   return rc;
}

//----------------------------------------------------------------------------
int slfs_flush_log(SLFS_DISKINFO * di)
{
   SLFS_SYSLOG * slog;
   DF_DISKINFO * df_di;
   __int32 log_flash_addr;
   unsigned __int32 crc;
   int rc;

   df_di = &di->df_di;

//---------------------------------------------------        
#ifdef DEBUG_CONSOLE 
   tn_snprintf( (char *)g_dump_buf, MAX_DUMP_STR_LEN,
     "Pass slfs_move_log()\r\n");
   dump_str(g_tst_file_buf);
#endif
//---------------------------------------------------        
	
   //--- Log tmp data - at Log Sector 0

   log_flash_addr = df_di->disk_sb.LogStartSector * df_di->disk_sb.SectorSize;
   rc = sfsi_read_page(df_di, (unsigned char *)df_di->buf, log_flash_addr);
   if(rc != ERR_NO_ERR)
       return rc;

   slog = (SLFS_SYSLOG *)df_di->buf;

   crc = sfsi_chk_crc((unsigned char *)df_di->buf + DF_CRC_SIZE,
                                       sizeof(SLFS_SYSLOG) - DF_CRC_SIZE);
   if(crc != slog->crc)
   {
//---------------------------------------------------        
#ifdef DEBUG_CONSOLE 
   dump_str("Bad tmp log crc\r\n");
#endif
//---------------------------------------------------        

      return ERR_BAD_CRC;
   }
   //-- Flush file header(if any)

   if(slog->log_data.op_mode == DF_APPEND) //-- DF_APPEND mode
   {
      //-- Write file header

      //-- sector is already pre-erased(at slfs_file_create())

      log_flash_addr = slog->log_data.file_start_sector * df_di->disk_sb.SectorSize;

      memmove((unsigned char*)df_di->buf,
              (unsigned char*)df_di->buf + DF_CRC_SIZE,
              sizeof(SLFS_FILE_HEADER));

      rc = sfsi_write_page(df_di, (unsigned char *)df_di->buf, log_flash_addr);
      if(rc != ERR_NO_ERR)
         return rc;

      //---- Re-read log

      log_flash_addr = df_di->disk_sb.LogStartSector * df_di->disk_sb.SectorSize;
      rc = sfsi_read_page(df_di, (unsigned char *)df_di->buf, log_flash_addr);
      if(rc != ERR_NO_ERR)
         return rc;

      crc = sfsi_chk_crc((unsigned char *)df_di->buf + DF_CRC_SIZE,
                                       sizeof(SLFS_SYSLOG) - DF_CRC_SIZE);
      if(crc != slog->crc)
         return ERR_BAD_CRC;
   }
   else if(slog->log_data.op_mode == DF_READ) //-- DF_APPEND mode
   {
      log_flash_addr = slog->log_data.file_start_sector * df_di->disk_sb.SectorSize;

      rc = sfsi_set_to_zero(df_di,
                            log_flash_addr,
                            16);  //-- max nbytes
      if(rc != ERR_NO_ERR)
         return rc;
   }

   //-- Flush log data

   memmove((unsigned char*)df_di->buf + DF_CRC_SIZE,
           (unsigned char*)df_di->buf + DF_CRC_SIZE + sizeof(SLFS_FILE_HEADER),
            sizeof(SLFS_LOGDISKDATA));

   crc = sfsi_chk_crc((unsigned char *)df_di->buf + DF_CRC_SIZE,
                                       sizeof(SLFS_LOGDISKDATA));

   memcpy((unsigned char *)df_di->buf, &crc, DF_CRC_SIZE);

 //--- Erase main log sector

   rc = sfsi_erase_sector(df_di, df_di->disk_sb.LogStartSector + 1L);
   if(rc != ERR_NO_ERR)
      return ERR_NO_ERR;

 //--- Store log info to the main log location

   log_flash_addr = (df_di->disk_sb.LogStartSector + 1L) * df_di->disk_sb.SectorSize;
   rc = sfsi_write_page(df_di, (unsigned char *)df_di->buf, log_flash_addr);
   if(rc != ERR_NO_ERR)
      return rc;

   //--- If here - log is flushed;

       //-- set to 0 crc & first 4 byte of info

   log_flash_addr = df_di->disk_sb.LogStartSector * df_di->disk_sb.SectorSize;

   rc = sfsi_set_to_zero(df_di, log_flash_addr, 8);

   return rc;
}

//----------------------------------------------------------------------------
int slfs_move_log(SLFS_DISKINFO * di)
{
   DF_DISKINFO * df_di;
   SLFS_LOGDISKDATA * ld;
   unsigned __int32 crc;
   unsigned __int32 crc_dsk;
   __int32 flash_addr;
   __int32 new_log_sector;
   int rc;

   df_di = &di->df_di;

   //--- Main log Log data - at Log Sector 1

   flash_addr = (df_di->disk_sb.LogStartSector + 1L)* df_di->disk_sb.SectorSize;
   rc = sfsi_read_page(df_di, (unsigned char *)df_di->buf, flash_addr);
   if(rc != ERR_NO_ERR)
   {
//---------------------------------------------------        
#ifdef DEBUG_CONSOLE
   tn_snprintf((char *)g_dump_buf, MAX_DUMP_STR_LEN,
                "LogStartSector + 1=%lu SectorSize=%lu flash_addr=%ld rc=%d\r\n",
                df_di->disk_sb.LogStartSector + 1L,
                df_di->disk_sb.SectorSize,
                flash_addr,
                rc);
   dump_str(g_dump_buf);
#endif
//---------------------------------------------------        

       return rc;
   }

   memcpy(&crc_dsk, (unsigned char *)df_di->buf, DF_CRC_SIZE);

   crc = sfsi_chk_crc((unsigned char *)df_di->buf + DF_CRC_SIZE,
                                       sizeof(SLFS_LOGDISKDATA));
   if(crc != crc_dsk)
   {
//---------------------------------------------------        
#ifdef DEBUG_LOG_SHIFT
   dump_str((unsigned char *)"Bad main log record crc\r\n");
#endif
//---------------------------------------------------        

      return ERR_BAD_CRC;
   }

   //-- Do moving only when main log data is valid

   ld = (SLFS_LOGDISKDATA *)((unsigned char *)df_di->buf + DF_CRC_SIZE);

   if(ld->log_wr_cnt == 0) //-- Needs shift
   {
      new_log_sector = df_di->disk_sb.LogStartSector + 2;
      if(new_log_sector + 2 >=  (__int32)(df_di->disk_sb.LogAreaStartSector +
                                            df_di->disk_sb.SectorsPerLogArea))
      {
         //-- Here we leave the log at the same pos
				
				     //-- Some small value to get ability to read (only) all disk files
				
         di->ld.log_wr_cnt  = df_di->disk_sb.SectorsPerData; 
         di->ld.log_wr_cnt += di->ld.log_wr_cnt>>1;

         df_di->disk_sb.PagesPerBmp = 1L;    //-- Disk now is read-only
      }
      else  //-- Set start value(s) for the new log pos
      {
         df_di->disk_sb.LogStartSector = new_log_sector;

         di->ld.log_wr_cnt = DF_MAX_NUM_ERASING;

         //-- Sector in new Log pos is (in most cases) pre-erased

         flash_addr = (df_di->disk_sb.LogStartSector + 1) * df_di->disk_sb.SectorSize;

         rc = sfsi_erase_sector(df_di, df_di->disk_sb.LogStartSector + 1);
         if(rc != ERR_NO_ERR)
            return rc;

         rc = sfsi_write_page(df_di, (unsigned char *)df_di->buf, flash_addr);
         if(rc != ERR_NO_ERR)
            return rc;
      }

      df_di->disk_sb.Crc = sfsi_chk_crc(((unsigned char *)&df_di->disk_sb) +
                              DF_CRC_SIZE, sizeof(DF_SB) - DF_CRC_SIZE);

   //--- Makes two copies of Super Block


     //--- Wr 1st copy - at addr di->SectorZero

              //-- di->buf will be corrupted
      rc = sfsi_erase_sector(df_di, df_di->disk_sb.SectorZero);
      if(rc != ERR_NO_ERR)
         return rc;

      memset((char *)df_di->buf, 0, DF_PAGE_SIZE);  //-- set mem to 0
      memcpy((char *)df_di->buf, &df_di->disk_sb, sizeof(DF_SB));

      flash_addr = df_di->disk_sb.SectorZero * DF_SECTOR_SIZE;
      rc = sfsi_write_page(df_di, (unsigned char*)df_di->buf, flash_addr);
      if(rc != ERR_NO_ERR)
         return rc;

     //--- Wr 2nd copy - at addr (sectors - 1)

              //-- di->buf will be corrupted
      rc = sfsi_erase_sector(df_di, df_di->disk_sb.SecondSBSector);
      if(rc != ERR_NO_ERR)
         return rc;

      memset((char *)df_di->buf, 0, DF_PAGE_SIZE);  //-- set mem to 0
      memcpy((char *)df_di->buf, &df_di->disk_sb, sizeof(DF_SB));

      flash_addr = df_di->disk_sb.SecondSBSector * DF_SECTOR_SIZE;
      rc = sfsi_write_page(df_di, (unsigned char*)df_di->buf, flash_addr);
      if(rc != ERR_NO_ERR)
         return rc;

//---------------------------------------------------        
#ifdef DEBUG_LOG_SHIFT
   tn_snprintf((char *)g_dump_buf, MAX_DUMP_STR_LEN,
                "--- LOG SHIFTED -- LogStartSector: %lu \r\n",
                df_di->disk_sb.LogStartSector);
   dump_str(g_dump_buf);
#endif
//---------------------------------------------------        
   }

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------






