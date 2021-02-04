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
#include "../../../fs_common/flash_fs.h"
#include "../../../fs_common/flash_fs_cfg.h"
#include "../../../fs_common/sys_errorcodes.h"
#include "slfs_test.h"

int tn_snprintf( char *outStr, int maxLen, const char *fmt, ... );
void dump_str(unsigned char * str);

//----------------------------------------------------------------------------
void dump_di_slfs(SLFS_DISKINFO * di, unsigned char * buf)
{
   DF_DISKINFO * df_di;
   SLFS_LOGDISKDATA * ld;
   int rc;
   char sbuf[8];
   long log_flash_addr;
   long tmp;
   
   df_di = &di->df_di;

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN, "\r\nDisk: %c (%d)\r\n\r\n", 
               'A' + (df_di->disk_num-1), 
                df_di->disk_num);
   dump_str(buf);


   if(df_di->disk_sb.FileSysVersion & FS_TYPE_SFS)
   {
      tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
          "FS type:  SFS\r\n\r\n");
      dump_str(buf);
   }
   if(df_di->disk_sb.FileSysVersion & FS_TYPE_SLFS)
   {
      tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
          "FS type:  SLFS\r\n\r\n");
      dump_str(buf);
   }

   if((df_di->disk_sb.FileSysVersion & (FS_TYPE_SFS | FS_TYPE_SLFS)) == 0)
   {
      tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
                   "FS type:  unknown\r\n\r\n");
      dump_str(buf);
   }

   if((df_di->disk_sb.FileSysVersion & FS_TYPE_SLFS) == 0)
      return;

//---------------- Log
   dump_str((unsigned char*)"\r\n   Log \r\n----------------------\r\n\r\n");

   log_flash_addr = (df_di->disk_sb.LogStartSector + 1) * df_di->disk_sb.SectorSize;

   rc = sfsi_read_page(df_di, (unsigned char *)df_di->buf, log_flash_addr);
   if(rc == ERR_NO_ERR)
   {
      ld = (SLFS_LOGDISKDATA *)((unsigned char*)df_di->buf + DF_CRC_SIZE);

      tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
                  "log_wr_cnt:  %lu (avaliable capacity)\r\n",  ld->log_wr_cnt);
      dump_str(buf);

      tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
                  "tail_sector: %lu\r\n",  ld->tail_sector);
      dump_str(buf);

      tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
                  "head_sector: %lu\r\n",  ld->head_sector);
      dump_str(buf);

      tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
                  "n_read:      %lu\r\n",  ld->n_read);
      dump_str(buf);

      tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
                  "n_write:     %lu\r\n",  ld->n_write);
      dump_str(buf);

      tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
                  "n_files:     %lu\r\n",  ld->n_files);
      dump_str(buf);

      tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
                  "overflow:    %lu\r\n",  ld->overflow);
      dump_str(buf);

      switch(ld->op_mode)
      {
         case DF_APPEND:
            dump_str((unsigned char*)"mode:     DF_APPEND\r\n");
            break;
         case DF_READ:
            dump_str((unsigned char*)"mode:     DF_READ\r\n");
            break;
         default:
            tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
                     "mode:     %d\r\n",  ld->op_mode);
            dump_str(buf);
      }
      dump_str((unsigned char*)"----------------------\r\n\r\n");
   }
   else
   {
      tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
                  "Failed read slfs log. Error: %d\r\n",rc);
      dump_str(buf);
   }


//-------------------------------------------------------------------------------------------

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "FileSysVersion: %d\r\n", df_di->disk_sb.FileSysVersion & (~0xF0000000));
   dump_str(buf);

   tmp = df_di->disk_sb.magic_num;
   memcpy(sbuf, (char*)&tmp, 4);
   sbuf[4] = 0;
   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "magic_num: %s\r\n", sbuf);
   dump_str(buf);

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "SectorZero: %lu\r\n",         df_di->disk_sb.SectorZero);
   dump_str(buf);

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "TotalSectors: %lu\r\n",       df_di->disk_sb.TotalSectors);
   dump_str(buf);

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "SectorSize: %lu\r\n",         df_di->disk_sb.SectorSize);
   dump_str(buf);

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "InodesStartSector: %lu\r\n",  df_di->disk_sb.InodesStartSector);
   dump_str(buf);

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "SectorsPerInodes: %lu\r\n",   df_di->disk_sb.SectorsPerInodes);
   dump_str(buf);

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "DataStartSector: %lu\r\n",    df_di->disk_sb.DataStartSector);
   dump_str(buf);

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "SectorsPerData: %lu\r\n",     df_di->disk_sb.SectorsPerData);
   dump_str(buf);

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "LogAreaStartSector: %lu\r\n", df_di->disk_sb.LogAreaStartSector);
   dump_str(buf);

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "SectorsPerLogArea: %lu\r\n",  df_di->disk_sb.SectorsPerLogArea);
   dump_str(buf);

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "LogStartSector: %lu\r\n",     df_di->disk_sb.LogStartSector);
   dump_str(buf);

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "SectorsPerLog: %lu\r\n",      df_di->disk_sb.SectorsPerLog);
   dump_str(buf);

   if(df_di->disk_sb.PagesPerBmp == 0)
      tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "PagesPerBmp: %lu (disk is writable)\r\n", df_di->disk_sb.PagesPerBmp);
   else
      tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "PagesPerBmp: %lu (disk is read-only)\r\n", df_di->disk_sb.PagesPerBmp);

   dump_str(buf);

   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
               "SecondSBSector: %lu\r\n",     df_di->disk_sb.SecondSBSector);
   dump_str(buf);
}

//----------------------------------------------------------------------------
void dump_slfs_file_info(SLFS_FILE_HEADER * fheader,
                         unsigned __int32 curr_sector,
                         unsigned char * buf)
{
   tn_snprintf((char *)buf, MAX_TST_DUMP_STR_LEN,
           " %s        %d         %08X       0x%08X (%d)\r\n",
           fheader->file_name,
           fheader->file_len,
           fheader->attr,
           curr_sector, curr_sector);
   dump_str(buf);
}

//----------------------------------------------------------------------------
void dump_all_files_slfs(DF_FLI * fli, 
                         SLFS_FILE * hFile, 
                         int disk_letter, 
                         unsigned char * dump_buf)
{
   int rc;
   SLFS_FILEFIND hFileFind;

   rc = slfs_get_files_number(fli, disk_letter);
   if(rc < 0) // error
   {
      tn_snprintf((char *)dump_buf, MAX_TST_DUMP_STR_LEN,
                     "\r\n\r\nslfs_get_files_number() failed.Error: %d\r\n", rc);
      dump_str(dump_buf);
      return; 
   } 

   tn_snprintf((char *)dump_buf, MAX_TST_DUMP_STR_LEN,
                     "\r\n\r\nNum files: %d\r\n\r\n", rc);
   dump_str(dump_buf);

   tn_snprintf((char *)dump_buf, MAX_TST_DUMP_STR_LEN,
                     "File name          Len           Attr            Addr\r\n");
   dump_str(dump_buf);

   tn_snprintf((char *)dump_buf, MAX_TST_DUMP_STR_LEN,
                     "----------       --------     ----------    -----------------------\r\n");
   dump_str(dump_buf);

   rc = slfs_enum_files_first(fli,
                              &hFileFind,   //-- will be filled at creation
                              disk_letter);         //-- 'A', 'B', 'C', 'D'
   if(rc == ERR_FILE_NOT_FOUND)
      return;
   else
   {
      if(rc == ERR_NO_ERR)
         dump_slfs_file_info(&hFileFind.fheader, hFileFind.curr_sector, dump_buf);
   }

   for(;;)
   {
      rc = slfs_enum_files_next(&hFileFind);   //-- Result - in hSFSFile
      if(rc == ERR_LAST_PAGE)
         return;
      else
      {
         if(rc == ERR_NO_ERR)
            dump_slfs_file_info(&hFileFind.fheader, hFileFind.curr_sector, dump_buf);
         else
         {
            tn_snprintf((char *)dump_buf, MAX_TST_DUMP_STR_LEN,
                        "slfs_enum_files_next() failed. Error: %d\r\n", rc);
            dump_str(dump_buf);
            
            return;
         }
      }
   }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------










