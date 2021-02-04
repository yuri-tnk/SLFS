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

//----------------------------------------------------------------------------
int slfs_enum_files_first(DF_FLI * fli,
                         SLFS_FILEFIND * hFileFind, //-- will be filled inside the function
                         int disk_letter)          //-- 'A', 'B', 'C', 'D'
{
   SLFS_DISKINFO * di;
   DF_DISKINFO * df_di;
   int disk_num;
   int rc;
   int i;

   if(fli == NULL || hFileFind == NULL)
      return ERR_WRONG_PARAM;

   //-- Find disk (now we support up to 4 disks)

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
         return ERR_DISK_NOT_EXISTS;
   }

   di = NULL;
   for(i=0; i < DF_NUM_DISKS; i++)
   {
      df_di = fli->di_arr[i];
      if(df_di)
      { 
         if(df_di->disk_num == disk_num)
         {
            if(df_di->disk_sb.FileSysVersion & FS_TYPE_SLFS)
            {
               memset(hFileFind, 0, sizeof(SLFS_FILEFIND));
               di = (SLFS_DISKINFO *)df_di;
               break;
            }
         }
      } 
   }

   hFileFind->di = di;
   if(hFileFind->di == NULL)
      return ERR_DISK_NOT_EXISTS;

   df_di = &di->df_di;

   if(di->ld.head_sector != di->ld.tail_sector) //-- FS not empty
   {
      rc = is_sector_file_hdr(df_di, di->ld.tail_sector);
      if(rc != ERR_NO_ERR)
         return rc;

      memcpy(&hFileFind->fheader, (unsigned char*)df_di->buf, sizeof(SLFS_FILE_HEADER));

      hFileFind->curr_sector = di->ld.tail_sector;
      hFileFind->n_files     = 1; 

      return ERR_NO_ERR;
   }
   return ERR_FILE_NOT_FOUND;
}


//----------------------------------------------------------------------------
int slfs_enum_files_next(SLFS_FILEFIND * hFileFind)
{
   unsigned __int32 n_sector;
   DF_DISKINFO * df_di;
   int rc;

   if(hFileFind == NULL)
      return ERR_WRONG_PARAM;

   if(hFileFind->di == NULL)
      return ERR_WRONG_PARAM;

   df_di = &hFileFind->di->df_di; 

   n_sector = hFileFind->curr_sector;

   for(;;)
   {
      n_sector++;
      if(n_sector > df_di->disk_sb.SecondSBSector - 1)  //-- disk last sector for data
         n_sector = df_di->disk_sb.DataStartSector;     //-- disk first sector for data

      if(n_sector == hFileFind->di->ld.head_sector)
         return  ERR_LAST_PAGE;

      rc = is_sector_file_hdr(df_di, n_sector);
      if(rc == ERR_NO_ERR)
         break;
      else if(rc == ERR_NOT_FILE_HEADER_SECTOR)
         continue;
      else
         return rc;

   }

   //-- If we are hear - the file header was found

   memcpy(&hFileFind->fheader, (unsigned char*)df_di->buf, sizeof(SLFS_FILE_HEADER));

   hFileFind->curr_sector = n_sector;
   hFileFind->n_files++;

   return rc;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
