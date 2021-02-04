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


#ifndef  _FLASH_FS_H_
#define  _FLASH_FS_H_

#ifndef WIN32

#define __int32  long

#ifndef BOOL
#define BOOL int
#endif

#ifndef FALSE
#define FALSE  0
#endif

#ifndef TRUE
#define TRUE   1
#endif

#ifndef NULL
#define NULL   0
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include "../TNKernel/tn.h"

#define dump_str uart0_send_str

#else

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#endif


//----------------------------------------------------------------------------

#define  DF_CRC_SIZE                   4
#define  DF_MAX_DISKS                  4

#define  DF_PAGE_SIZE                256
#define  DF_SECTOR_SIZE             4096

#define  DF_MIN_SECTORS_IN_DISK       64



#define  DF_SECTOR_ERASED              1
#define  DF_SECTOR_NOT_ERASED          2


#define  DF_APPEND                     1
#define  DF_READ                       2
//---------------------------------------------------

   //-- Mode for wr_page_ex() func

#define  DF_1ST_SECTOR                 1
#define  DF_LAST_SECTOR                2

//---------------------------------------------------

#define  DF_EMPTY_BYTE_VAL             0xFF
#define  DF_EMPTY_BYTES_LONG           0xFFFFFFFFUL

#define  DF_FIRST_DISK_ENTRY_OFFSET   32

//-------- DF_MBRENTRY parameter 'FSType'

#define  FS_TYPE_SFS                   0x10000000UL
#define  FS_TYPE_SLFS                  0x20000000UL


#define  DF_MBR_SECTOR_NUM             0
#define  DF_MBR_NAME_SIZE             16

//-- Typically, the serial NOR flashes have num erasing = 100000 - we use 90000 here
#define  DF_MAX_NUM_ERASING        90000L //-- For test - 150L, 8000L

/*
//----------------------------------------------------------------------------
//                       SFS definition
//----------------------------------------------------------------------------

#define  SFS_NUM_BMP_PAGES              2

#define  SFS_MAX_NUM_ERASING            DF_MAX_NUM_ERASING


#define  SFS_LOG_SECTORS_NUM            3
#define  SFS_LOG_MAX_POSITIONS          8

#define  SFS_FILE_MAX_NAME_LEN          8

#define  SFS_NUM_BMP_PAGES              2
#define  SFS_MAX_NUM_ERASING            DF_MAX_NUM_ERASING

#define  SFS_MIN_SECTORS_IN_DISK       64

#define  SFS_SECTOR_SIZE               DF_SECTOR_SIZE
#define  SFS_PAGE_SIZE                 DF_PAGE_SIZE


   //-- File open modes

#define  O_CREATE                       1
#define  O_READ                         2
#define  O_ATTR                         4
#define  O_ERASE                        8


//---- Kind of file (param 'attr' when file is opened) -----------------------
  //--- Do not change the num values !!!

#define  SFS_FILE_BMP                1
#define  SFS_FILE_FONT               2
#define  SFS_FILE_VOICE              3
#define  SFS_FILE_MUSIC              4
#define  SFS_FILE_SYS                5 
#define  SFS_FILE_INFO               6 
#define  SFS_FILE_FW                 7 
#define  SFS_FILE_INFOMEM            8 

  //----
#define  SFS_SECTOR_ERASED              1
#define  SFS_SECTOR_NOT_ERASED          2


//---------------------------------------------------
#define  SFS_CRC_SIZE                   DF_CRC_SIZE

#define  SFS_EMPTY_BYTE_VAL             DF_EMPTY_BYTE_VAL


   //-- Each bmp bit -sector

#define  SFS_BITS_PER_PAGE             (SFS_PAGE_SIZE*8)
#define  SFS_BITS_PER_INT              (sizeof(int) * 8)
#define  SFS_INT_PER_PAGE              (SFS_PAGE_SIZE / sizeof(int))
*/
//----------------------------------------------------------------------------
//                       SLFS definition
//----------------------------------------------------------------------------

#define  SLFS_LOG_SECTORS_NUM             3  // 2 ????
#define  SLFS_LOG_MAX_POSITIONS           8

#define  SLFS_FILE_SIGN_SIZE              8
#define  SLFS_FILE_MAX_NAME_SIZE         15


//----------------------------------------------------------------------------
//                       Data structures
//----------------------------------------------------------------------------

#pragma  pack(push, 1)

typedef struct _DF_MBRENTRY
{
   unsigned __int32 SectorZero;   //-- > 0
   unsigned __int32 TotalSectors; //-- < 0xFFFF
   unsigned __int32 FSType;
}DF_MBRENTRY;

typedef struct _DF_MBRREC
{
   unsigned __int32 crc;
   unsigned char name[DF_MBR_NAME_SIZE];   //-- > 0
   unsigned __int32 sector_size;
   unsigned __int32 sectors;
}DF_MBRREC;

typedef struct _DF_MBR
{
   DF_MBRREC mbr_rec;
}DF_MBR;

#pragma pack(pop)

//---------------------------------------------------

struct _DF_DISKINFO;

typedef struct _SFS_FLI
{
   struct _DF_DISKINFO * di_arr[DF_MAX_DISKS];

#ifdef WIN32
   CRITICAL_SECTION cs;
   HANDLE hBkgThread;
   int fTerminate;
#else
   TN_SEM semSFS;
#endif

   struct _DF_DISKINFO * di_bkg;  //-- Current disk for bkg erasing process

   unsigned __int32 sector_size;
   unsigned __int32 sectors;
   unsigned char * big_buf;
}DF_FLI;

//------- On-disk  SuperBlock info -------------------------------------------

#pragma  pack(push, 1)

typedef struct _DF_SB
{
   unsigned __int32 Crc;
   unsigned __int32 FileSysVersion;        //-- also contains fs_type
   unsigned __int32 magic_num;             //-- _YT_

   unsigned __int32 SectorZero;
   unsigned __int32 TotalSectors;
   unsigned __int32 SectorSize;

   unsigned __int32 InodesStartSector;
   unsigned __int32 SectorsPerInodes;      //-- SLFS = 0

   unsigned __int32 DataStartSector;       //-- SLFS = InodesStartSector;
   unsigned __int32 SectorsPerData;

   unsigned __int32 LogAreaStartSector;
   unsigned __int32 SectorsPerLogArea;

   unsigned __int32 LogStartSector;
   unsigned __int32 SectorsPerLog;

   unsigned __int32 PagesPerBmp;           //-- SLFS = 0

   unsigned __int32 SecondSBSector;

}DF_SB;

#pragma pack(pop)

//----------------------------------------------------------------------------
typedef struct _DF_DISKINFO
{
   DF_FLI * fli;             //-- Link to global struct
   unsigned char disk_num;    //-- 1 - 'A', 2 - 'B', 3 - 'C' etc
   DF_SB disk_sb;         //-- on-disk record

   unsigned int buf[DF_PAGE_SIZE/sizeof(int)];

}DF_DISKINFO;

//----------------------------------------------------------------------------
//                  SLFS
//----------------------------------------------------------------------------

//-- The SFS_PAGE_SIZE is aloocated at disk for the file header

#pragma pack(push, 1)

typedef struct _SLFS_FILE_HEADER
{
   unsigned long crc;
   unsigned char header_sign[8];
   unsigned char file_name[SLFS_FILE_MAX_NAME_SIZE + 1];  //-- ASCIIZ
   unsigned long file_len;
   unsigned long attr;
   unsigned long fnum;
}SLFS_FILE_HEADER;


typedef struct _SLFS_LOGDISKDATA
{
   unsigned __int32 log_wr_cnt;                //--  4
   unsigned __int32 file_start_sector;         //--  4
   unsigned __int32 tail_sector;               //--  4
   unsigned __int32 head_sector;               //--  4
   unsigned __int32 n_read;                    //--  4
   unsigned __int32 n_write;                   //--  4
   unsigned __int32 n_files;                   //--  4
   unsigned __int32 overflow;                  //--  4
   //unsigned __int32 overflow_rd;
   unsigned __int32 op_mode;                   //--  4
}SLFS_LOGDISKDATA;

#pragma pack(pop)


typedef struct _SLFS_DISKINFO
{

   DF_DISKINFO df_di;
   SLFS_LOGDISKDATA ld;

 //--- SLFS specific

   unsigned __int32  tail_sector_pr;
//   unsigned __int32  head_sector_pr;

   __int32 start_data_flash_addr;
   __int32 end_data_flash_addr;

}SLFS_DISKINFO;

//---------- SLFS File control block

typedef struct _SLFS_FILE
{
   SLFS_DISKINFO * di;  //-- Link to 'di'; 'di' contains link to 'fli'

   SLFS_FILE_HEADER fheader;

   int open_mode;      //-- O_READ, O_CREATE
   int fWrLastPage;

   __int32 pos;
   __int32 curr_sector;

   unsigned __int32 file_addr;     //-- Points to the file data (excluded fheader)
   __int32 file_len;

}SLFS_FILE;


//----------- File find block

typedef struct _SLFS_FILEFIND
{
   SLFS_DISKINFO * di;
   SLFS_FILE_HEADER fheader;

  //---
   unsigned __int32 curr_sector;
   int n_files;

}SLFS_FILEFIND;


//----------------------------------------------------------------------------
// SLFS  Log cluster
//----------------------------------------------------------------------------

//log_curr_data       - sector 0
//disk_stored_data    - sector 1


#pragma pack (push, 1)

 //--- at sector 0 - log_curr_data(store to flush)

typedef struct _SLFS_SYSLOG
{
   unsigned __int32 crc;
   SLFS_FILE_HEADER fheader;
  //----------------------------------
   SLFS_LOGDISKDATA log_data;
}SLFS_SYSLOG;

#pragma pack (pop)

 //--- at sector 1   permanent data

  // crc + SLFS_LOGDISKDATA log_data

//----------------------------------------------------------------------------
//                             Debug
//----------------------------------------------------------------------------
#ifdef WIN32
typedef struct _DF_DBGINFO
{
   FILE * hFile;
}DF_DBGINFO;
#endif

   //--- df_disk_access.c

unsigned __int32 sfsi_chk_crc(unsigned char * buf, int len);
unsigned __int32 sfsi_update_crc(unsigned __int32 crc, unsigned char ch);
void sfs_lock(DF_FLI * fli);
void sfs_unlock(DF_FLI * fli);
int sfsi_write_page(DF_DISKINFO * di, unsigned char * buf, __int32 flash_addr);
int sfsi_read_page(DF_DISKINFO * di, unsigned char * buf, __int32 flash_addr);
int sfsi_read_page_ex(DF_FLI * fli, unsigned char * buf, int mode);
int sfsi_write_page_ex(DF_FLI * fli, unsigned char * buf, int mode);
int sfsi_read_sector(DF_DISKINFO * di, unsigned char * buf, __int32 flash_addr);
int sfsi_write_sector(DF_DISKINFO * di, unsigned char * buf, __int32 flash_addr);
int sfsi_erase_sector(DF_DISKINFO * di, int sector_num);
int sfsi_chk_sector_for_erase(DF_DISKINFO * di, __int32 sector);
int sfsi_set_to_zero(DF_DISKINFO * di, __int32 flash_addr, int nbytes);
int sfsi_write_flash_bytes(DF_DISKINFO * di,
                           __int32 flash_addr,
                           unsigned char * buf,
                           int nbytes);
int sfsi_read_flash_bytes(DF_DISKINFO * di,
                          __int32 flash_addr,
                          unsigned char * buf,
                          int nbytes);
   //--- slfs_log.c

int slfs_prepare_log(SLFS_FILE * hFile, int mode);
int slfs_flush_log(SLFS_DISKINFO * di);
int slfs_move_log(SLFS_DISKINFO * di);

   //-- df_create_init.c

int df_create_mbr(DF_FLI * fli, char * name);     //-- Flash hame i.e. "M25PE16"
int df_create_mbr_disk_entry(DF_FLI * fli,
                             __int32 size_in_sectors,
                             unsigned __int32 fs_type);

int df_init_disks(DF_FLI * fli);
DF_DISKINFO * df_find_disk(DF_FLI * fli, int disk_letter);
int df_set_disk_for_bkgnd_proc(DF_FLI * fli,
                                int disk); //-- 'A', 'B' etc

int df_chk_flash_ic(void);
/*
   //--- sfs_bkgnd.h

int df_init_bkgnd_process(DF_FLI * fli);
*/
   //--- slfs_create_init.c

int slfs_create_disk(DF_FLI * fli,
                     int disk_num);  //-- 1,2,3,4..
int slfs_init_disk(DF_FLI * fli,
                  int disk_num,     //-- 1,2,3..
                  DF_MBRENTRY * pEntry);

   //--- slfs_api.c

int slfs_file_create(DF_FLI * fli,
                     SLFS_FILE * hFile,
                     int disk_letter,   //-- 'A', 'B' etc..
                     unsigned char * file_name,
                     __int32 file_size,
                     unsigned long file_attr);
__int32 slfs_file_write(SLFS_FILE * hFile, unsigned char * buf, __int32 count);
int slfs_file_open(DF_FLI * fli,
                   SLFS_FILE * hFile,
                   int disk_letter);
__int32 slfs_file_read(SLFS_FILE * hFile, unsigned char * buf, __int32 count);
int slfs_file_close(SLFS_FILE * hFile);
int slfs_file_close_not_erase(SLFS_FILE * hFile);
int slfs_get_files_number(DF_FLI * fli,
                          int disk_letter);

int is_sector_file_hdr(DF_DISKINFO * df_di, int n_sector);

   //--- slfs_api_ex.cpp

int slfs_enum_files_first(DF_FLI * fli,
                         SLFS_FILEFIND * hFileFind, //-- will be filled at creation
                         int disk);                 //-- 'A', 'B', 'C', 'D'
int slfs_enum_files_next(SLFS_FILEFIND * hFileFind);

/*
  //--- copy_slfs_sfs.c

int copy_file_slfs_to_sfs(DF_FLI * fli,
                          SLFS_FILE * hSLFSFile, //-- SLFS file already was open
                          SFS_FILE * hSFSFile,  //-- SFS file
                          unsigned char * buf,
                          unsigned char * sfs_file_name, // SFS dst file
                          BOOL fOwerwrite);
  //--- sgl_base_func.c
*/
unsigned char cpld_spi_send_byte(const unsigned char data);

#define  MAX_TST_DUMP_STR_LEN     96
#define  MAX_DUMP_STR_LEN         (MAX_TST_DUMP_STR_LEN -1)

//----------------------------------------------------------------------------

#endif

