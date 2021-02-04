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

#include <string.h>       //-- for IAR
#include "../fs_common/sys_errorcodes.h"
#include "../fs_common/flash_fs.h"
#include "../fs_common/flash_fs_cfg.h"


#if defined(__CROSSWORKS_ARM) || defined (__ICCARM__) 
size_t strlcpy(char *dst, const char *src, size_t siz); 
#endif

static const unsigned char slfs_header_sign[SLFS_FILE_SIGN_SIZE] = {'S', 'L', 'F', 'S', 'f', 'h', 'd', 'r'};

static int slfs_find_disk(DF_FLI * fli,
                         SLFS_FILE * hFile,
                         int disk);   //-- 'A', 'B' etc..
static int slfsi_searh_file_for_tail(SLFS_DISKINFO * di,
                                        __int32  start_sector,
                                        __int32 * new_tail_sector); //-- [OUT]
#ifdef WIN32

extern FILE * g_hDumpFile;

size_t strlcpy (char *dst, const char *src, size_t len)
{
    size_t n;
    // Avoid problems if size_t is unsigned 

    if(len <= 0)
       return strlen (src);
 
    for(n = 0; n < len - 1; n++) 
    {
       if((dst[n] = src[n]) == '\0')
          break;
    }

    if(n >= len-1) 
    {
        // ran out of space

       dst[n] = '\0';
       while (src[n])
          n++;
    }
    return n;
}
#endif /* WIN32 */

extern unsigned char g_dump_buf[];
int tn_snprintf( char *outStr, int maxLen, const char *fmt, ... );
void dump_str(unsigned char * str);
 

//#define DEBUG_CONSOLE 1

//----------------------------------------------------------------------------
//  Create is for writting only
//----------------------------------------------------------------------------
int slfs_file_create(DF_FLI * fli,
                     SLFS_FILE * hFile,
                     int disk_letter,   //-- 'A', 'B' etc..
                     unsigned char * file_name,
                     __int32 file_size,
                     unsigned long file_attr)
{
   DF_DISKINFO * df_di;
   SLFS_DISKINFO * di;
   int rc;
   __int32 num_req_sectors;
   __int32 new_tail_sector;
   unsigned __int32 prev_head_sector;
   int fOverflow = FALSE;
   int n_cond = FALSE;

   if(fli == NULL || hFile == NULL)
      return ERR_WRONG_PARAM;

   //-- Also set contents of hFile to 0, except of 'di'

   rc = slfs_find_disk(fli, hFile, disk_letter); 
   if(rc != ERR_NO_ERR)
      return rc;

   di = hFile->di;
   if(di == NULL)  //-- Sanity checking
      return ERR_WRONG_PARAM;

   df_di = &di->df_di;

   if(df_di->disk_sb.PagesPerBmp != 0) //-- Disk now is read-only
   {
      hFile->open_mode = 0;
      return ERR_DISK_NOW_IS_READ_ONLY;
   }

   sfs_lock(fli);
                               
   hFile->open_mode  = DF_APPEND;

   memcpy(&hFile->fheader.header_sign[0], (char*)&slfs_header_sign[0], SLFS_FILE_SIGN_SIZE);
   strlcpy((char *)&hFile->fheader.file_name[0], (char *)file_name, SLFS_FILE_MAX_NAME_SIZE);
   hFile->fheader.file_len = file_size;
   hFile->fheader.attr     = file_attr;
   hFile->fheader.fnum     = di->ld.n_write;
   hFile->fheader.crc      = sfsi_chk_crc((unsigned char *)&hFile->fheader + DF_CRC_SIZE,
                                          sizeof(SLFS_FILE_HEADER) - DF_CRC_SIZE);

   hFile->file_len = file_size;
   hFile->curr_sector = -1L;

   di->ld.file_start_sector = di->ld.head_sector;


   num_req_sectors = (file_size + (__int32)DF_PAGE_SIZE + ((__int32)df_di->disk_sb.SectorSize - 1)) / (__int32)df_di->disk_sb.SectorSize;

   prev_head_sector = di->ld.head_sector;

   di->ld.head_sector += (unsigned __int32)num_req_sectors;


   if(di->ld.head_sector > df_di->disk_sb.SecondSBSector - 1)    //-- disk last sector for data
   {
      di->ld.file_start_sector = df_di->disk_sb.DataStartSector; //-- disk first sector for data
      di->ld.head_sector = df_di->disk_sb.DataStartSector + (unsigned __int32)num_req_sectors;  //-- Head - for next file  

      fOverflow = TRUE;  //-- Overflow - just right now
      di->ld.overflow++; //-- Overflow was once or more (non-volatile)
   }

   hFile->file_addr = di->ld.file_start_sector * df_di->disk_sb.SectorSize + (unsigned __int32)DF_PAGE_SIZE;

  //-- Now - set new 'tail_sector' if 'head_sector' overflow 'tail_sector'

   if(di->ld.overflow) //-- Overflow of the DiskData space was once or more (non-volatile)
   {
      if(di->ld.head_sector >= di->ld.tail_sector && di->ld.file_start_sector < di->ld.tail_sector) 
      {
        
//-------------------------------------------
#ifdef DEBUG_CONSOLE        
#ifdef WIN32

if(g_hDumpFile)
   fprintf(g_hDumpFile, "set err di->ld.overflow_rd = 1  p_wr=%d p_rd=%d \r\n",
                 di->ld.head_sector,
                 di->ld.tail_sector);
#endif
#endif // DEBUG_CONSOLE
//-------------------------------------------
         n_cond = TRUE;
      }
      else if(fOverflow) //-- Overflow - just right now
      {
         if(di->ld.head_sector >= di->ld.tail_sector)
            n_cond = TRUE;
         else if(di->ld.tail_sector == df_di->disk_sb.SecondSBSector - 1) 
         {
            if(prev_head_sector == di->ld.tail_sector) //-- FIFO was empty before WR
               di->ld.tail_sector = df_di->disk_sb.DataStartSector;
            else
               n_cond = TRUE;
         }
      }
   }

   if(n_cond)
   {
      rc = slfsi_searh_file_for_tail(di, 
                                     di->ld.head_sector,
                                     &new_tail_sector);
      if(rc != ERR_NO_ERR)
      {
         if(rc == ERR_FILE_NOT_FOUND)
         {

//-------------------------------------------
#ifdef DEBUG_CONSOLE        
#ifdef WIN32

if(g_hDumpFile)
   fprintf(g_hDumpFile, "wr err searh_file_for_tail file_not_found  p_wr=%d p_rd=%d \r\n",
           di->ld.head_sector,
           di->ld.tail_sector);
#else
tn_snprintf((char *)g_dump_buf, MAX_DUMP_STR_LEN,
             "wr err searh_file_for_tail file_not_found  p_wr=%lu p_rd=%lu \r\n",
                 di->ld.head_sector,
                 di->ld.tail_sector);
dump_str(g_dump_buf);

#endif
#endif // DEBUG_CONSOLE
//-------------------------------------------

            new_tail_sector = di->ld.head_sector + 1;
            if(new_tail_sector > (__int32)df_di->disk_sb.SecondSBSector - 1) //-- disk last sector for data
               new_tail_sector = df_di->disk_sb.DataStartSector; 
         }
         else
         { 

//-------------------------------------------
#ifdef DEBUG_CONSOLE        
#ifdef WIN32
if(g_hDumpFile)
   fprintf(g_hDumpFile, "wr err searh_file_for_tail fatal p_wr=%d p_rd=%d \r\n",
           di->ld.head_sector,
           di->ld.tail_sector);
else
tn_snprintf( (char *)g_dump_buf, MAX_DUMP_STR_LEN,
             "wr err searh_file_for_tail fatal p_wr=%lu p_rd=%lu \r\n",
             di->ld.head_sector,
             di->ld.tail_sector);
dump_str(g_dump_buf);

#endif
#endif // DEBUG_CONSOLE
//-------------------------------------------

            return rc;
         }
      }

      di->ld.tail_sector = new_tail_sector;

//-------------------------------------------
#ifdef DEBUG_CONSOLE        
#ifdef WIN32
if(g_hDumpFile)
{
   if(di->ld.head_sector == di->ld.tail_sector)
      fprintf(g_hDumpFile, "equal p_wr=%d \r\n", di->ld.head_sector);
   fprintf(g_hDumpFile, "overflow  p_wr=%d p_rd=%d \r\n",
                 di->ld.head_sector,
                 di->ld.tail_sector);
}
#else
   tn_snprintf( (char *)g_dump_buf, MAX_DUMP_STR_LEN,
                 "overflow  p_wr=%lu p_rd=%lu \r\n",
                 di->ld.head_sector,
                 di->ld.tail_sector);            
   dump_str(g_dump_buf);
#endif
#endif // DEBUG_CONSOLE
//-------------------------------------------

   }
   
//-------------------------------------------
#ifdef DEBUG_CONSOLE        
#ifdef WIN32
   if(g_hDumpFile)
      fprintf(g_hDumpFile, "wr p_fstart=%d p_wr=%d p_rd=%d n_sec=%d, len=%d, f_addr=%d\r\n",
                    di->ld.file_start_sector,
                    di->ld.head_sector,
                    di->ld.tail_sector,
                    num_req_sectors,
                    hFile->file_len,
                    hFile->file_addr);
#else
   tn_snprintf( (char *)g_dump_buf, MAX_DUMP_STR_LEN,
       "wr p_fstart=%lu p_wr=%lu p_rd=%lu n_sec=%lu, len=%lu, f_addr=%lu\r\n",
                    di->ld.file_start_sector,
                    di->ld.head_sector,
                    di->ld.tail_sector,
                    num_req_sectors,
                    hFile->file_len,
                    hFile->file_addr);
   dump_str(g_dump_buf);
#endif
#endif // DEBUG_CONSOLE
//-------------------------------------------

   return ERR_NO_ERR;

}

//----------------------------------------------------------------------------
//  File should be created with DF_APPEND flag ( by calling slfs_file_create())
//
//  The writing is only sequental. The 'lseek' operation is not
//  supported. A 'count' should be SFS_PAGE_SIZE(256 bytes); 
//  If 'count' < 256 - it will be set as a last page and the further 
//  writing is restricted.
//
//  Total 'nbytes' to write must be == 'file_size', "overflow" bytes will be cutted
//----------------------------------------------------------------------------
__int32 slfs_file_write(SLFS_FILE * hFile, unsigned char * buf, __int32 count)
{
   __int32 position;
   __int32 n_bytes;
   __int32 flash_addr;
   __int32 nb;
   __int32 rest_of_page;
   __int32 n_sector;
   unsigned char * buffer;
   int rc;

   if(hFile == NULL || buf == NULL || count > DF_PAGE_SIZE || count < 0)
      return ERR_WRONG_PARAM;

   if(hFile->open_mode != DF_APPEND)
      return ERR_OP_MODE;

 //-- New ---------------------

   if(hFile->fWrLastPage)
      return ERR_WR_FINISHED;

 //----------------------------
   n_bytes  = count;
   buffer   = buf;
   position = hFile->pos;

   while(n_bytes > 0)
   {
      nb = n_bytes;
      rest_of_page = (__int32)DF_PAGE_SIZE - position % (__int32)DF_PAGE_SIZE;

      //-- File len cannot be greater than hFile->file_len
      if(hFile->file_len < position + nb)
         nb = hFile->file_len - position;
      if(nb == 0)
         break;

      if(nb > rest_of_page)
         nb = rest_of_page;
  //-----------------

      flash_addr = hFile->file_addr + //-- Always aligned to DF_PAGE_SIZE
                 ((position/(__int32)DF_PAGE_SIZE) * (__int32)DF_PAGE_SIZE);

      n_sector = flash_addr / hFile->di->df_di.disk_sb.SectorSize;
      if(n_sector != hFile->curr_sector)
      {
         rc = sfsi_erase_sector(&hFile->di->df_di, n_sector);
         if(rc != ERR_NO_ERR)
            return (__int32)rc;
         hFile->curr_sector = n_sector;
      }

      rc = sfsi_write_page(&hFile->di->df_di, buf, flash_addr);
      if(rc != ERR_NO_ERR)
         return (__int32)rc;

      if(position % (__int32)DF_PAGE_SIZE == 0 && nb == (__int32)DF_PAGE_SIZE)
      {
         //-- do nothig here
      }
      else //-- write a partial page - a last
      {
         hFile->fWrLastPage = TRUE;
      }

      buffer   += nb;
      position += nb;
      n_bytes  -= nb;
   }

   hFile->pos  = position;

   return (count - n_bytes);
}

//----------------------------------------------------------------------------
// 'Open' is used for reading only
//----------------------------------------------------------------------------
int slfs_file_open(DF_FLI * fli,
                   SLFS_FILE * hFile,
                   int disk_letter)
{
   DF_DISKINFO * df_di;
   SLFS_DISKINFO * di;
   __int32 new_tail_sector;
   __int32 num_req_sectors;
   int rc;

   if(fli == NULL || hFile == NULL)
      return ERR_WRONG_PARAM;

   //-- Also set contents of hFile to 0, except of 'di'

   rc = slfs_find_disk(fli, hFile, disk_letter);
   if(rc != ERR_NO_ERR)
      return rc;

   di = hFile->di;
   if(di == NULL)
      return ERR_WRONG_PARAM;
   df_di = &di->df_di;

   sfs_lock(fli);

   if(di->ld.head_sector == di->ld.tail_sector)
   {
      sfs_unlock(fli);
      
//------------------------------------
#ifdef DEBUG_CONSOLE        
#ifdef WIN32

   if(g_hDumpFile)
      fprintf(g_hDumpFile, "rd err DISK EMPTY p_wr=p_rd=%d\r\n", di->ld.tail_sector);
#else

   tn_snprintf( (char *)g_dump_buf, MAX_DUMP_STR_LEN,
            "rd err DISK EMPTY p_wr=p_rd=%lu\r\n", di->ld.tail_sector);
   dump_str(g_dump_buf);

#endif
#endif // DEBUG_CONSOLE
//------------------------------------

      return ERR_DISK_EMPTY;
   }

   rc = slfsi_searh_file_for_tail(di, 
                                  di->ld.tail_sector - 1,
                                  &new_tail_sector);
   if(rc != ERR_NO_ERR)
   { 
      sfs_unlock(fli);

//------------------------------------
#ifdef DEBUG_CONSOLE        
#ifdef WIN32

   if(g_hDumpFile)
      fprintf(g_hDumpFile, "rd err searh_file_for_tail() failed \r\n");
#else
   tn_snprintf( (char *)g_dump_buf, MAX_DUMP_STR_LEN,
              "rd err searh_file_for_tail() failed \r\n");
   dump_str(g_dump_buf);
#endif
#endif // DEBUG_CONSOLE
//------------------------------------
 
      return rc;
   }
   
   di->tail_sector_pr = di->ld.tail_sector;
   di->ld.tail_sector = new_tail_sector;
   
   memcpy(&hFile->fheader, (char*)df_di->buf, sizeof(SLFS_FILE_HEADER));

   hFile->file_len  = hFile->fheader.file_len;
   hFile->open_mode = DF_READ;
   di->ld.file_start_sector = di->ld.tail_sector;

   num_req_sectors = (hFile->file_len + (__int32)DF_PAGE_SIZE + ((__int32)df_di->disk_sb.SectorSize - 1)) / (__int32)df_di->disk_sb.SectorSize;

   di->ld.tail_sector += (unsigned __int32)num_req_sectors;

   if(di->ld.tail_sector > df_di->disk_sb.SecondSBSector - 1) //-- disk last sector for data
   {
      di->ld.file_start_sector = df_di->disk_sb.DataStartSector; //-- disk first sector for data
      di->ld.tail_sector = df_di->disk_sb.DataStartSector + (unsigned __int32)num_req_sectors;  //-- Head - for next file  
   }

   hFile->file_addr = di->ld.file_start_sector * df_di->disk_sb.SectorSize + (unsigned __int32)DF_PAGE_SIZE;

//------------------------------------
#ifdef DEBUG_CONSOLE        
#ifdef WIN32
   if(g_hDumpFile)
      fprintf(g_hDumpFile, "rd f_num=%d, p_fstart=%d p_rd=%d, p_wr=%d, f_addr=%d\r\n",
                    hFile->fheader.fnum,
                    di->ld.file_start_sector,
                    di->ld.tail_sector,
                    di->ld.head_sector,
                      hFile->file_addr);
#else
   tn_snprintf( (char *)g_dump_buf, MAX_DUMP_STR_LEN,
       "rd f_num=%lu, p_fstart=%lu p_rd=%lu, p_wr=%lu, f_addr=%lu\r\n",
                    hFile->fheader.fnum,
                    di->ld.file_start_sector,
                    di->ld.tail_sector,
                    di->ld.head_sector,
                    hFile->file_addr);

   dump_str(g_dump_buf);
#endif
#endif // DEBUG_CONSOLE
//------------------------------------

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
__int32 slfs_file_read(SLFS_FILE * hFile, unsigned char * buf, __int32 count)
{
   __int32 size;
   __int32 n_bytes;
   __int32 position;
   __int32 nb;
   __int32 rest_of_page;
   __int32 flash_addr;
   unsigned char * buffer;
   int rc;

   if(hFile == NULL || buf == NULL || count > DF_PAGE_SIZE || count < 0)
      return ERR_WRONG_PARAM;

   if(hFile->open_mode != DF_READ)
      return ERR_OP_MODE;

   size     = hFile->file_len;
   n_bytes  = count;
   position = hFile->pos;
   buffer   = buf;

   while(n_bytes > 0)
   {
      nb = n_bytes;
      rest_of_page = (__int32)DF_PAGE_SIZE - position % (__int32)DF_PAGE_SIZE;
      if(size < position + nb)
         nb = size - position;
      if(nb == 0)
         break;

      if(nb > rest_of_page)
         nb = rest_of_page;

      flash_addr = hFile->file_addr + //-- Always aligned to SFS_PAGE_SIZE
                 ((position/(__int32)DF_PAGE_SIZE) * (__int32)DF_PAGE_SIZE);

      if(position % (__int32)DF_PAGE_SIZE == 0 && nb == (__int32)DF_PAGE_SIZE)
      {
         rc = sfsi_read_page(&hFile->di->df_di, buffer, flash_addr);
         if(rc != ERR_NO_ERR)
            return rc;
      }
      else //-- read a page - payload is partial page
      {
         unsigned char * ptr;
         ptr = (unsigned char*)hFile->di->df_di.buf;
         rc = sfsi_read_page(&hFile->di->df_di, ptr, flash_addr);
         if(rc != ERR_NO_ERR)
            return rc;
         memcpy(buffer, &ptr[position % (__int32)DF_PAGE_SIZE], nb);
      }

      buffer   += nb;
      position += nb;
      n_bytes  -= nb;
   }

   hFile->pos = position;

   return (count - n_bytes);
}

//----------------------------------------------------------------------------
int slfs_file_close(SLFS_FILE * hFile)
{
   SLFS_DISKINFO * di;
   int rc;

   if(hFile == NULL)
      return ERR_WRONG_PARAM;

   di = hFile->di;
   if(di == NULL)
      return ERR_WRONG_PARAM;

   if(hFile->open_mode == DF_APPEND)
   {
      if(hFile->pos != hFile->file_len) //-- Not a declaired len was written
      {
         sfs_unlock(di->df_di.fli);
         return ERR_BAD_LEN;
      }

      rc = slfs_prepare_log(hFile, DF_APPEND);
      if(rc != ERR_NO_ERR)
      {
         sfs_unlock(di->df_di.fli);
         return rc;
      }

      rc = slfs_flush_log(di);

      sfs_unlock(di->df_di.fli);

       //-- Clear the file structure to prevent operating with a closed file

      memset(hFile, 0, sizeof(SLFS_FILE));

      return rc;
   }
   else if(hFile->open_mode == DF_READ)
   {
      rc = slfs_prepare_log(hFile, DF_READ);
      if(rc != ERR_NO_ERR)
      {
         sfs_unlock(di->df_di.fli);
         return rc;
      }

      rc = slfs_flush_log(di);

      sfs_unlock(di->df_di.fli);

       //-- Clear the file structure to prevent operating with a closed file

      memset(hFile, 0, sizeof(SLFS_FILE));

      return rc;
   }

   sfs_unlock(di->df_di.fli);
   return ERR_OP_MODE;
}

//----------------------------------------------------------------------------
int slfs_file_close_not_erase(SLFS_FILE * hFile)
{
   SLFS_DISKINFO * di;

   if(hFile == NULL)
      return ERR_WRONG_PARAM;

   di = hFile->di;
   if(di == NULL)
      return ERR_WRONG_PARAM;
   if(hFile->open_mode != DF_READ)
      return ERR_OP_MODE;
     
   di->ld.tail_sector = di->tail_sector_pr;

   sfs_unlock(di->df_di.fli);

       //-- Clear the file structure to prevent operating with a closed file

   memset(hFile, 0, sizeof(SLFS_FILE));

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
int slfs_get_files_number(DF_FLI * fli,
                          int disk_letter)   //-- 'A', 'B' etc.., int disk_letter)
{
   int rc;
   int count = 0;
   SLFS_FILEFIND hFileFind;

   if(fli == NULL)
      return ERR_WRONG_PARAM;

   rc = slfs_enum_files_first(fli,
                              &hFileFind,   //-- will be filled at creation
                              disk_letter);         //-- 'A', 'B', 'C', 'D'
   if(rc == ERR_FILE_NOT_FOUND)
      return 0;
   else
   {
      if(rc == ERR_NO_ERR)
         count++;
   }

   for(;;)
   {
      rc = slfs_enum_files_next(&hFileFind);   //-- Result - in hSFSFile
      if(rc == ERR_LAST_PAGE)
         return count;
      else
      {
         if(rc == ERR_NO_ERR)
            count++;
         else
            return rc;
      }
   }
}

//----------------------------------------------------------------------------
static int slfs_find_disk(DF_FLI * fli,
                         SLFS_FILE * hFile,
                         int disk)   //-- 'A', 'B' etc..
{
   DF_DISKINFO * df_di;
   int disk_num;
   int i;

   memset(hFile, 0, sizeof(SLFS_FILE));

   //-- Find disk

   //-- Now we support up to 4 disks
   switch(disk)
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

   for(i = 0; i< DF_NUM_DISKS; i++)
   {
      df_di = fli->di_arr[i];
      if(df_di)
      {
         if(df_di->disk_num == disk_num)
         {
            if(df_di->disk_sb.FileSysVersion & FS_TYPE_SLFS)
            {
               hFile->di = (SLFS_DISKINFO *)df_di;
               break;
            }
            else
               return ERR_WRONG_FS;
         }
      }
   }

   if(hFile->di == NULL)
      return ERR_DISK_NOT_EXISTS;

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
static int slfsi_searh_file_for_tail(SLFS_DISKINFO * di,
                                        __int32  start_sector,
                                        __int32 * new_tail_sector) //-- [OUT]
{
   unsigned __int32 n_sector;
   DF_DISKINFO * df_di;
   int rc = ERR_NO_ERR;
   int count = 0;

   df_di = &di->df_di;
   n_sector = (unsigned __int32)start_sector; //di->ld.tail_sector;

   for(;;)
   {
      count++;
      n_sector++;
      if(n_sector > df_di->disk_sb.SecondSBSector - 1) //-- disk last sector for data
         n_sector = df_di->disk_sb.DataStartSector;    //-- disk first sectorr for data

      if(n_sector == di->ld.head_sector)
      {
         rc = ERR_FILE_NOT_FOUND;
         break;
      }
      rc = is_sector_file_hdr(df_di, n_sector);
      if(rc == ERR_NO_ERR)
         break;
      else if(rc == ERR_NOT_FILE_HEADER_SECTOR)
         continue;
      else
         return rc;
   }
   
//-------------------------------------------
#ifdef DEBUG_CONSOLE        
#ifdef WIN32

   if(g_hDumpFile)
      fprintf(g_hDumpFile, "count = %d\r\n", count);
#else
   tn_snprintf( (char *)g_dump_buf, MAX_DUMP_STR_LEN,
       "count = %d\r\n", count);

   dump_str(g_dump_buf);
   
#endif
#endif // DEBUG_CONSOLE
//-------------------------------------------

   *new_tail_sector = (__int32)n_sector;

   return rc;
}

//----------------------------------------------------------------------------
int is_sector_file_hdr(DF_DISKINFO * df_di, int n_sector)
{
   unsigned long crc;
   int rc;
   SLFS_FILE_HEADER * sfh;
   unsigned long flash_addr;

   flash_addr =  df_di->disk_sb.SectorSize * (unsigned long)n_sector;

   rc = sfsi_read_page(df_di, (unsigned char*)df_di->buf, flash_addr);
   if(rc != ERR_NO_ERR)
      return rc;
   if(memcmp((unsigned char*)df_di->buf + sizeof(long),
          (unsigned char *)slfs_header_sign, SLFS_FILE_SIGN_SIZE) == 0)
   {
      crc = sfsi_chk_crc((unsigned char *)df_di->buf + DF_CRC_SIZE,
                        sizeof(SLFS_FILE_HEADER) - DF_CRC_SIZE);
      sfh = (SLFS_FILE_HEADER *)df_di->buf;
      if(sfh->crc == crc)
         return ERR_NO_ERR;
   }
   return ERR_NOT_FILE_HEADER_SECTOR;
}


//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

