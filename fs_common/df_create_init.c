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
#include "sys_errorcodes.h"
#include "flash_fs.h"
#include "flash_fs_cfg.h"
#include "df_flash.h"

extern unsigned char g_tst_file_buf[];
int tn_snprintf( char *outStr, int maxLen, const char *fmt, ... );
void dump_str(unsigned char * str);

//----------------------------------------------------------------------------
//  Must be called first and once only. Actually, it is flash formating function
//  The all flash data(if any) will be destroyed 
//----------------------------------------------------------------------------
int df_create_mbr(DF_FLI * fli, char * name)     //-- Flash hame i.e. "M25PX64" etc.
{
   DF_MBR * pMBR;
   DF_DISKINFO * df_di = NULL;
   __int32 flash_addr;
   int namelen;
   int i;
   int rc;

   namelen = strlen(name);
   if(namelen >= DF_MBR_NAME_SIZE)
      return ERR_WRONG_PARAM;

   for(i=0; i < DF_NUM_DISKS; i++)
   {
      df_di = fli->di_arr[i];
      if(df_di != NULL)
         break; 
   }

   if(df_di == NULL)
      return ERR_WRONG_PARAM;

   rc = df_chk_flash_ic();
   if(rc != ERR_NO_ERR)
      return rc;


   memset(df_di, 0, sizeof(DF_DISKINFO)); //-- SectorZero will be 0

   df_di->disk_sb.TotalSectors = fli->sectors;  //-- Only for MBR creation
   df_di->disk_sb.SectorSize   = fli->sector_size;

   //--- Erase entire flash -------------

   for(i = 0; i < (int)fli->sectors; i++)
   {
      rc = sfsi_erase_sector(df_di, i);
      if(rc != ERR_NO_ERR)
         return rc;
   }
   
   //------------------------------------

   memset((unsigned char *)df_di->buf, DF_EMPTY_BYTE_VAL, DF_PAGE_SIZE);

   pMBR = (DF_MBR *)df_di->buf;

   //--- Max flash name

   memcpy((char*)pMBR->mbr_rec.name, name, namelen);
   pMBR->mbr_rec.name[namelen] = 0;

   pMBR->mbr_rec.sector_size  = fli->sector_size;
   pMBR->mbr_rec.sectors      = fli->sectors;

   //---

   pMBR->mbr_rec.crc = sfsi_chk_crc((unsigned char *)df_di->buf + DF_CRC_SIZE,
                                   DF_PAGE_SIZE - DF_CRC_SIZE);

   //--- Wr 1st copy - at addr SEF_MBR_SECTOR_NUM
   flash_addr = DF_MBR_SECTOR_NUM * DF_SECTOR_SIZE;
   rc = sfsi_write_page(df_di, (unsigned char *)df_di->buf, flash_addr);
   if(rc != ERR_NO_ERR)
      return rc;

   //--- Wr 2nd copy - at addr (sectors - 1)
   flash_addr = (fli->sectors - 1) * DF_SECTOR_SIZE;
   rc = sfsi_write_page(df_di, (unsigned char *)df_di->buf, flash_addr);
   if(rc != ERR_NO_ERR)
      return rc;

   return ERR_NO_ERR;
}

//----------------------------------------------------------------------------
int df_create_mbr_disk_entry(DF_FLI * fli,
                             __int32 size_in_sectors,
                             unsigned __int32 fs_type)
{
   DF_MBRENTRY * pEntry;
   DF_MBR * pMBR;
   DF_DISKINFO * df_di = NULL;
   unsigned __int32 total;
   unsigned __int32 len;
   unsigned __int32 aval_space;
   int rc;
   int qty = 0;

   for(rc=0; rc < DF_NUM_DISKS; rc++)
   {
      df_di = fli->di_arr[rc];
      if(df_di != NULL)
         break; 
   }

   if(df_di == NULL)
      return ERR_WRONG_PARAM;

   memset(df_di, 0, sizeof(DF_DISKINFO)); //-- SectorZero will be 0

   rc = sfsi_read_page_ex(fli, (unsigned char*)df_di->buf, DF_1ST_SECTOR);
   if(rc != ERR_NO_ERR)
      return rc;
   //-- ToDo - check crc here

   total = 1L; //-- Case first disk's SectorZero will be at FLASH sector 1
              //-- at FLASH sector 0 - 1st copy of MBR
   pMBR = (DF_MBR *)df_di->buf;
   aval_space = pMBR->mbr_rec.sectors - 2L; //-- Max possible len(1st & last sectors - MBR

   pEntry  = (DF_MBRENTRY *)((unsigned char*)df_di->buf + DF_FIRST_DISK_ENTRY_OFFSET);
   for(;;)
   {
      if(pEntry->SectorZero == DF_EMPTY_BYTES_LONG &&    //-- Empty
             pEntry->TotalSectors == DF_EMPTY_BYTES_LONG)
      {
          //-- Ok, it will be a new disk record

         len = min((__int32)aval_space, size_in_sectors);
         if(len < DF_MIN_SECTORS_IN_DISK)
            return ERR_NO_DISK_SPACE;

         pEntry->SectorZero   = total;  //--
         pEntry->TotalSectors = len;
         pEntry->FSType       = fs_type;

         total += pEntry->TotalSectors;

         //-- Sanity checking
         if(total >=  pMBR->mbr_rec.sectors)
         {
            rc = ERR_WRONG_PARAM;
            break;
         }
         if(pEntry->SectorZero + pEntry->TotalSectors >= pMBR->mbr_rec.sectors)
         {
            rc = ERR_WRONG_PARAM;
            break;
         }

         pMBR->mbr_rec.crc = sfsi_chk_crc((unsigned char*)df_di->buf + DF_CRC_SIZE,
                                                        DF_PAGE_SIZE - DF_CRC_SIZE);

              //--- Wr 1st copy - at addr SEF_MBR_SECTOR_NUM
         rc = sfsi_write_page_ex(fli, (unsigned char*)df_di->buf, DF_1ST_SECTOR);
         if(rc != ERR_NO_ERR)
            break;
              //--- Wr 2nd copy - at addr (sectors - 1)
         rc = sfsi_write_page_ex(fli, (unsigned char*)df_di->buf, DF_LAST_SECTOR);

         break; //-- Exit any case
      }
      else if(pEntry->SectorZero != DF_EMPTY_BYTES_LONG &&    //-- Not empty
                 pEntry->TotalSectors != DF_EMPTY_BYTES_LONG)
      {
         total      += pEntry->TotalSectors;
         aval_space -= pEntry->TotalSectors;
         qty++;
      }
      else
      {
         rc = ERR_ENTRY_CORRUPTED;
         break;
      }

      pEntry++;
      //qty++;
      if(qty >= DF_MAX_DISKS)
      {
         rc = ERR_OVERFLOW;
         break;
      }
   }
   return rc;
}

//----------------------------------------------------------------------------
int df_init_disks(DF_FLI * fli)
{
   DF_MBRENTRY * pEntry;
   DF_MBRENTRY  Entry;
   DF_MBR * pMBR = NULL;

   unsigned __int32 crc1;
   unsigned __int32 crc2;
   unsigned char * buf;
   int rc;
   int disk_num = 1;

   rc = df_chk_flash_ic();
   if(rc != ERR_NO_ERR)
      return rc;

   buf = fli->big_buf;

   rc = sfsi_read_page_ex(fli, buf, DF_1ST_SECTOR);
   if(rc != ERR_NO_ERR)
      return rc;

   pMBR = (DF_MBR *)buf;

   //--- Check and recovering(if needs) MBR

   crc1 = sfsi_chk_crc(buf + DF_CRC_SIZE,
                         DF_PAGE_SIZE - DF_CRC_SIZE);
   if(crc1 != pMBR->mbr_rec.crc) //-- 1st MBR bad
   {
//dump_str("1st MBR bad \r\n");

      rc = sfsi_read_page_ex(fli, buf, DF_LAST_SECTOR);
      if(rc != ERR_NO_ERR)
         return rc;

      crc2 = sfsi_chk_crc(buf + DF_CRC_SIZE,
                            DF_PAGE_SIZE - DF_CRC_SIZE);
      if(crc2 == pMBR->mbr_rec.crc) //-- 2nd MBR ok
      {
//dump_str("2nd MBR ok \r\n");
         rc = sfsi_write_page_ex(fli, buf, DF_1ST_SECTOR); //-- Recover 1st
         if(rc != ERR_NO_ERR)
            return rc;
      }
      else
         return ERR_BOTH_MBR_CORRUPTED;
   }
   else // 1st MBR ok
   {
      rc = sfsi_read_page_ex(fli, buf, DF_LAST_SECTOR);
      if(rc != ERR_NO_ERR)
         return rc;

      crc2 = sfsi_chk_crc(buf + DF_CRC_SIZE,
                          DF_PAGE_SIZE - DF_CRC_SIZE);
      if(crc2 == pMBR->mbr_rec.crc) //-- 2nd MBR ok
      {
         if(crc2 != crc1)
         {
            //1st always is fresh
            rc = sfsi_read_page_ex(fli, buf, DF_1ST_SECTOR); //-- Read 1st
            if(rc != ERR_NO_ERR)
               return rc;
            rc = sfsi_write_page_ex(fli, buf, DF_LAST_SECTOR); //-- Refresh 2nd
            if(rc != ERR_NO_ERR)
               return rc;
         }
      }
      else //-- recovering 2nd MBR
      {

//dump_str("2nd MBR bad \r\n");

         rc = sfsi_read_page_ex(fli, buf, DF_1ST_SECTOR); //-- Read 1st
         if(rc != ERR_NO_ERR)
            return rc;
         rc = sfsi_write_page_ex(fli, buf, DF_LAST_SECTOR); //-- Recover 2nd
         if(rc != ERR_NO_ERR)
            return rc;
      }
   }
//--------------------------------
   rc = sfsi_read_page_ex(fli, buf, DF_1ST_SECTOR);
   if(rc != ERR_NO_ERR)
      return rc;

   for(;;)
   {
      pEntry  = (DF_MBRENTRY *) (buf + DF_FIRST_DISK_ENTRY_OFFSET +
                                     sizeof(DF_MBRENTRY) * (disk_num - 1));
      if(pMBR == NULL) // First iteration only
      {
         pMBR = (DF_MBR *)buf;
         fli->sector_size = pMBR->mbr_rec.sector_size;
         fli->sectors     = pMBR->mbr_rec.sectors;
      }

      if(pEntry->SectorZero == DF_EMPTY_BYTES_LONG &&    //-- Empty -> last
             pEntry->TotalSectors == DF_EMPTY_BYTES_LONG)
      {
         rc = ERR_NO_ERR;
         break;
      }
      else if(pEntry->SectorZero != DF_EMPTY_BYTES_LONG &&    //-- Not empty
                 pEntry->TotalSectors != DF_EMPTY_BYTES_LONG)
      {
         if(disk_num > DF_MAX_DISKS)
         {
            rc = ERR_OVERFLOW;
            break;
         }
				 
         memcpy(&Entry, pEntry, sizeof(DF_MBRENTRY));

/*  The SFS is not relevant for the SLFS base test
         if(pEntry->FSType == FS_TYPE_SFS)
         {
            rc = sfs_init_disk(fli,
                               disk_num,
                               &Entry);
				rc = ERR_NO_ERR; //-- No file systems
            if(rc != ERR_NO_ERR)
               break;
         }
         else 
*/				
         if(pEntry->FSType == 0) //-- No file systems
         { 
				rc = ERR_NO_ERR; 
         }
         else if(pEntry->FSType == FS_TYPE_SLFS)
         {
            rc = slfs_init_disk(fli,
                                disk_num,     //-- 1,2,3..
                                &Entry);
            if(rc != ERR_NO_ERR)
               break;
         }
         else
            return ERR_UNKNOWN_FS;

         disk_num++;
         continue;   
      }
      else
      {
         rc = ERR_ENTRY_CORRUPTED;
         break;
      }
   }

   return rc;
}

//----------------------------------------------------------------------------
DF_DISKINFO * df_find_disk(DF_FLI * fli, int disk_letter)
{
   DF_DISKINFO * df_di;
   int disk_num;
   int i;

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
         return NULL;
   }

   for(i=0; i < DF_NUM_DISKS; i++)
   {
      df_di = fli->di_arr[i];
      if(df_di->disk_num == disk_num)
      {
         return df_di;
      }
   }

   return NULL;
}

//----------------------------------------------------------------------------
int df_set_disk_for_bkgnd_proc(DF_FLI * fli,
                                int disk_letter) //-- 'A', 'B' etc
{
   DF_DISKINFO * df_di;
   int disk_num;
   int i;

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
         return ERR_DISK_NOT_EXISTS;
   }

   for(i=0; i < DF_NUM_DISKS; i++)
   {
      df_di = fli->di_arr[i];
      if(df_di->disk_num == disk_num)
      {
         fli->di_bkg = df_di;
         return ERR_NO_ERR;
      }
   }

   return ERR_DISK_NOT_EXISTS;
}

//----------------------------------------------------------------------------
int df_chk_flash_ic(void)
{
#ifdef WIN32
      return ERR_NO_ERR;
#else

   int i;
   unsigned char manufacturer_id;
   unsigned char mem_type;
   unsigned char mem_capacity;

   for(i=0; i<3; i++)
   {
       m25_rd_id(&manufacturer_id, //-- [OUT]
                 &mem_type,        //-- [OUT]
                 &mem_capacity);   //-- [OUT]
   }

  //--- !!! Update the values according to the yours FLASH IC) !!!
/*
   //--- For MX25L12836, MX25L12875F, MX25L12835E  (Macronix)

   if(manufacturer_id == 0xC2 &&
       mem_type == 0x20 &&
         mem_capacity == 0x18)
      return ERR_NO_ERR;
*/
   //--- For M25PX64  (Numonyx)

   if(manufacturer_id == 0x20 &&
       mem_type == 0x71 &&
         mem_capacity == 0x17)
      return ERR_NO_ERR;


   return ERR_BAD_FLASH_INFO;
#endif
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------



