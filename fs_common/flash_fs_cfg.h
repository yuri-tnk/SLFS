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


#ifndef  _FLASH_FS_CFG_H_
#define  _FLASH_FS_CFG_H_

//--------- For the project -------------------------

  //--- Flash data for the file systems creations

  // Now external serial flash is a M25PX64 - 8 MBytes
  // The flash contains 2048 sectors 4096 bytes each.

#define DF_FLASH_SECTORS          2096  //-- M25PX64 - 8 MBytes

  //--- Disks - num and size 

   //  Max num disks = 4 

#define  DF_NUM_DISKS                4  // 2 4 

    //-- disk A - reserved space

#define  DF_DISK_A_SECTORS        1024 // half of the flash

    //-- disk B - SLFS

    //-- Desired value. Actual value generated automatically and will be lower

#define  DF_DISK_B_SECTORS       (DF_FLASH_SECTORS - DF_DISK_A_SECTORS)


#endif


