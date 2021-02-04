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

//#pragma warning (disable: 4996)

//#define USE_TEXT 1
//#define USE_LOCAL_WR_CR 1

#ifdef USE_TEXT

const char chk_text_slfs1[] =
"FEATURES:\r\n"
"MCU: MSP430F149 with 60K Bytes Program Flash, 256 Bytes data Flash, 2K Bytes RAM\r\n"
"JTAG connector\r\n"
"JTAG Power_In and Power_Out jumpers\r\n"
"32 768 Hz oscillator crystal\r\n"
"optional high frequency crystal (socket)\r\n"
"TEST pin pull-down\r\n"
"RST/NMI pin pull-up\r\n"
"power supply filtering capacitor\r\n"
"extension headers for each uC pin\r\n"
"PCB: FR-4, 1.5 mm (0,062\"), green soldermask, white silkscreen component print\r\n"
"Dimensions: 48x48 mm (1.9x1.9\")\r\n"
"space between the pin rows: 43.18 mm (1.7)\r\n"
"DOCUMENTS:\r\n"
"\r\n"
"MSP430-Hxxx.pdf\r\n"
"MSP430-Hxx-dimensions.gif\r\n"
"HARDWARE:\r\n"
"\r\n"
"MSP430-Hxxx schematic\r\n"
"MSP430-JTAG connector (top)\r\n"
"\r\n"
"SOFTWARE:\r\n"
"\r\n"
"n/a. \r\n"
"ORDERING CODES: \r\n"
"                MSP430-H149 Fully assembled and tested with MSP430F149 microcontroller \r\n"
"Click here for our PRICELIST and shipping options, and click here to learn HOW TO ORDER\r\n"
;

const char chk_text_slfs[] =
"4 Operating features\r\n"
"\r\n"
"4.1 Page programming\r\n"
"\r\n"
"To program one data byte, two instructions are required: write enable (WREN), which is one\r\n"
"byte, and a page program (PP) sequence, which consists of four bytes plus data. This is\r\n"
"followed by the internal program cycle (of duration tPP).\r\n"
"To spread this overhead, the page program (PP) instruction allows up to 256 bytes to be\r\n"
"programmed at a time (changing bits from ‘1’ to ‘0’), provided that they lie in consecutive\r\n"
"addresses on the same page of memory.\r\n"
"For optimized timings, it is recommended to use the page program (PP) instruction to\r\n"
"program all consecutive targeted bytes in a single sequence versus using several page\r\n"
"program (PP) sequences with each containing only a few bytes (see Page program (PP)\r\n"
"and Table 18: AC characteristics).\r\n"
"\r\n"
"4.2 Dual input fast program\r\n"
"\r\n"
"The dual input fast program (DIFP) instruction makes it possible to program up to 256 bytes\r\n"
"using two input pins at the same time (by changing bits from ‘1’ to ‘0’).\r\n"
"For optimized timings, it is recommended to use the dual input fast program (DIFP)\r\n"
"instruction to program all consecutive targeted bytes in a single sequence rather to using\r\n"
"several dual input fast program (DIFP) sequences each containing only a few bytes (see\r\n"
"Section 6.12: Dual input fast program (DIFP)).\r\n"
"\r\n"
"4.3 Subsector erase, sector erase and bulk erase\r\n"
"\r\n"
"The page program (PP) instruction allows bits to be reset from ‘1’ to ’0’. Before this can be\r\n"
"applied, the bytes of memory need to have been erased to all 1s (FFh). This can be\r\n"
"achieved either a subsector at a time, using the subsector erase (SSE) instruction, a sector\r\n"
"at a time, using the sector erase (SE) instruction, or throughout the entire memory, using the\r\n"
"bulk erase (BE) instruction. This starts an internal erase cycle (of duration tSSE, tSE or tBE).\r\n"
"The erase instruction must be preceded by a write enable (WREN) instruction.\r\n"
"\r\n"
"4.4 Polling during a write, program or erase cycle\r\n"
"\r\n"
"A further improvement in the time to write status register (WRSR), program OTP (POTP),\r\n"
"program (PP), dual input fast program (DIFP) or erase (SSE, SE or BE) can be achieved by\r\n"
"not waiting for the worst case delay (tW, tPP, tSSE, tSE, or tBE). The write in progress (WIP) bit\r\n"
"is provided in the status register so that the application program can monitor its value,\r\n"
"polling it to establish when the previous write cycle, program cycle or erase cycle is\r\n"
"complete.\r\n"
"\r\n"
"4.5 Active power, standby power and deep power-down modes\r\n"
"\r\n"
"When Chip Select (S) is Low, the device is selected, and in the active power mode.\r\n"
"When Chip Select (S) is High, the device is deselected, but could remain in the active power\r\n"
"mode until all internal cycles have completed (program, erase, write status register). The\r\n"
"device then goes in to the standby power mode. The device consumption drops to ICC1.\r\n"
"The deep power-down mode is entered when the specific instruction (the deep power-down\r\n"
"(DP) instruction) is executed. The device consumption drops further to ICC2. The device\r\n"
"remains in this mode until another specific instruction (the release from deep power-down\r\n"
"(RDP) instruction) is executed.\r\n"
"While in the deep power-down mode, the device ignores all write, program and erase\r\n"
"instructions (see Section 6.18: Deep power-down (DP)), this can be used as an extra\r\n"
"software protection mechanism, when the device is not in active use, to protect the device\r\n"
"from inadvertent write, program or erase instructions.\r\n"
"\r\n"
"4.6 Status register\r\n"
"\r\n"
"The status register contains a number of status and control bits that can be read or set (as\r\n"
"appropriate) by specific instructions. See Section 6.4: Read status register (RDSR) for a\r\n"
"detailed description of the status register bits.\r\n"
"\r\n"
"4.7 Protection modes\r\n"
"\r\n"
"There are protocol-related and specific hardware and software protection modes. They are\r\n"
"described below.\r\n"
"\r\n"
"4.7.1 Protocol-related protections\r\n"
;

#endif

#define  MAX_TEST_FILE_SIZE  (1024 * 24)

//#define  CURR_MAX_TEST_FILE_SIZE    (4096*3)  //  (1024 * 6)  //(1024 * 256)

static const unsigned char fc_1[] = { 0xA5 };
static const unsigned char fc_2[] = { 0x5A, 0x3E };
static const unsigned char fc_3[] = { 0x78, 0x14, 0xD3 };
static const unsigned char fc_4[] = { 0x2D, 0xEA, 0xC9, 0x4B };


//----------------------------------------------------------------------------
int create_random_file_slfs(DF_FLI * fli,
                            SLFS_FILE  * hSLFSFile,
                            int disk_letter, 
                            unsigned char * name,
                            long max_len, 
                            unsigned char * buf)
{
   int rc;
   __int32 res;
   union
   {
      unsigned __int32  file_crc;
      unsigned char crc_buf[4];
   }ucrc;
   __int32 flen;
   int len = 0;

#ifdef USE_TEXT
  int txt_ind = 0;
#endif

   while(len == 0)
      len = slfs_rand() % max_len;

  // len = 3909;
  // len = 1;
  //  len = 2;
  // len = 3;
  // len = 4;

#ifdef USE_TEXT
   len = strlen(chk_text_slfs);
#endif

   if(len > 4)
      flen = len + 4;
   else
      flen = len;

   rc = slfs_file_create(fli,
                     hSLFSFile,
                     disk_letter,   //-- 'A', 'B' etc..
                     name,
                     flen,
                     (unsigned __int32)flen);   // file_attr

   if(rc != ERR_NO_ERR)
      return rc;

   if(len <= 4)
   {
      rc = ERR_INTERNAL;
      switch(len)
      {
         case 1:

            buf[0] = fc_1[0];
            res = slfs_file_write(hSLFSFile, buf, 1L);
            if(res == 1L) //-- ok
               rc = ERR_NO_ERR;
            else
               rc = ERR_FILE_WR;

            break;

         case 2:

            memcpy(buf, fc_2, 2);
            res = slfs_file_write(hSLFSFile, buf, 2L);
            if(res == 2L) //-- ok
               rc = ERR_NO_ERR;
            else
               rc = ERR_FILE_WR;

            break;

         case 3:

            memcpy(buf, fc_3, 3);
            res = slfs_file_write(hSLFSFile, buf, 3L);
            if(res == 3L) //-- ok
               rc = ERR_NO_ERR;
            else
               rc = ERR_FILE_WR;

            break;

         case 4:

            memcpy(buf, fc_4, 4);
            res = slfs_file_write(hSLFSFile, buf, 4L);
            if(res == 4L) //-- ok
               rc = ERR_NO_ERR;
            else
               rc = ERR_FILE_WR;
            break;
      }

      slfs_file_close(hSLFSFile);
      return rc;
   }
   else //-- len > 4
   {
      int i;
      __int32 nbytes;
      int wr_bytes;
      int fLast;

      nbytes = len;  // - 4;
      ucrc.file_crc = 0xffffffffUL;

      fLast = FALSE;
      while(nbytes > 0L)
      {
         if(nbytes < (__int32)DF_PAGE_SIZE)       //-- Last page
         {
            wr_bytes = (int)nbytes;
            fLast = TRUE;
         }
         else if(nbytes == (__int32)DF_PAGE_SIZE) //-- Last page
         {
            wr_bytes = DF_PAGE_SIZE;
            fLast = TRUE;
         }
         else //-- >
            wr_bytes = DF_PAGE_SIZE;

      //--- Fill file by random data & update crc

         for(i = 0; i < wr_bytes; i++)
         {
#ifdef USE_TEXT
            buf[i] =  (unsigned char) chk_text_slfs[txt_ind++];
#else
            buf[i] =  (unsigned char)(slfs_rand() % 256UL);
#endif
            ucrc.file_crc = sfsi_update_crc(ucrc.file_crc, buf[i]);
         }

         if(fLast)
         {
            ucrc.file_crc ^= 0xffffffffUL;

            if(DF_PAGE_SIZE - wr_bytes > 0) //-- We have a room at the end of 256 bytes page 
            {
               i = min(DF_PAGE_SIZE - wr_bytes, DF_CRC_SIZE);
               memcpy(buf + wr_bytes, &ucrc.crc_buf[0], i); //-- Store CRC at file end
               wr_bytes += i;
               i = DF_CRC_SIZE - i;
            } 
            else
               i = DF_CRC_SIZE;
         }

         res = slfs_file_write(hSLFSFile, buf, (__int32)wr_bytes);
         memset(buf + wr_bytes, 0, DF_PAGE_SIZE - wr_bytes);

         if(res != (__int32)wr_bytes)
         {
            slfs_file_close(hSLFSFile);
            if(res < 0)
               return (int)res;
            return ERR_FILE_WR;
         }
         else
         { 
            if(fLast)
            {
               if(i > 0 && wr_bytes == DF_PAGE_SIZE)
               {
                  res = slfs_file_write(hSLFSFile, &ucrc.crc_buf[DF_CRC_SIZE - i], (__int32)i);

                  slfs_file_close(hSLFSFile);

                  if(res < 0)
                     return (int)res;

                  if(res == (__int32)i)
                     return ERR_NO_ERR;
                  else
                     return ERR_FILE_WR;
               }

               slfs_file_close(hSLFSFile);
               return ERR_NO_ERR;
            }
         }
         nbytes -= res;
      }
   }
   return ERR_INTERNAL;
}

//----------------------------------------------------------------------------
int chk_random_file_slfs(DF_FLI * fli, 
                         int disk_letter, 
                         SLFS_FILE * hSLFSFile,
                         unsigned char * buf)
{
   union
   {
      unsigned __int32  file_crc;
      unsigned char crc_buf[4];
   }ucrc;
   unsigned __int32  crc;
   int rc;
   __int32 nb;
   int i;
   __int32 res;
   __int32 len = 0;
   __int32 nbytes;

   rc = slfs_file_open(fli,
                       hSLFSFile,
                       disk_letter);
   if(rc != ERR_NO_ERR)
      return rc;

   len = (__int32)hSLFSFile->fheader.file_len;

   if(len <= 4L)
   {
      res = slfs_file_read(hSLFSFile, buf, len);
      if(res != len)
      {
         if(res < 0L)
            return (int)res;
         else
            return ERR_DISK_RD;
      }

      rc = ERR_BAD_FILE_CONTENTS;
      switch(len)
      {
         case 1:

            if(buf[0] == fc_1[0])
               rc = ERR_NO_ERR;

            break;

         case 2:

            if(memcmp(buf, fc_2, 2) == 0)
               rc = ERR_NO_ERR;

            break;

         case 3:

            if(memcmp(buf, fc_3, 3) == 0)
               rc = ERR_NO_ERR;

            break;

         case 4:

            if(memcmp(buf, fc_4, 4) == 0)
               rc = ERR_NO_ERR;

            break;
      }

      slfs_file_close(hSLFSFile);
      return rc;
   }
   else
   {
      nbytes = len - DF_CRC_SIZE;
      crc = 0xffffffffUL;
      while(nbytes > 0)
      {
         if(nbytes < (__int32)DF_PAGE_SIZE)
            nb = nbytes;
         else
            nb = (__int32)DF_PAGE_SIZE;
         
         memset(buf, 0, DF_PAGE_SIZE);

         res = slfs_file_read(hSLFSFile, buf, nb);
         if(res != nb)
         {
            if(res < 0L)
               return (int)res;
            else
               return ERR_FILE_RD;
         }

         for(i = 0; i < (int)nb; i++)
            crc = sfsi_update_crc(crc, buf[i]);
         nbytes -= res;
      }

      crc ^= 0xffffffffUL;


      res = slfs_file_read(hSLFSFile, &ucrc.crc_buf[0], DF_CRC_SIZE);
      if(res != DF_CRC_SIZE)
      {
         if(res < 0L)
            return (int)res;
         else
            return ERR_FILE_RD;
      }
      slfs_file_close(hSLFSFile);
      if(crc != ucrc.file_crc)
         return ERR_BAD_CRC;
      return ERR_NO_ERR;
   }
#ifdef WIN32   
   return ERR_INTERNAL;
#endif   
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------





