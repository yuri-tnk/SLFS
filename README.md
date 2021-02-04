<div id="wrap">

<div id="header">


<div id="content">

## SLFS - Small Logger File System

<div class="tabber">

<div class="tabbertab">

## Description

  The **SLFS** (**S**mall **L**ogger **F**ile **S**ystem) is a logger file system, implemented on the serial flash with unified
sector size (4 KB).

  The logger file system uses the FIFO approach - for the writing the file is added to the tail of the internal list
while for the reading the file is taken from the head of the list.
  In most cases, after the reading the file is erased from SLFS and disk space is released.
  If SLFS disk is full and new file is added, the oldest file(s) will be automatically erased.

  Files in SLFS may have an arbitrary length.

  SLFS uses the write-ahead journaling for file creating/deleting and wear leveling for the system metadata.

<span class="blue1">**SLFS RAM usage**</span>

  The SLFS uses global structure DF_FLI (52 bytes), structure DF_DISKINFO (328 bytes) per each disk and the one
global buffer with the size of the flash sector (4096 bytes).

<span class="blue1">**SLFS multi-access**</span>

  The SLFS was designed for the small embedded systems with limited RAM capacity.
   In SLFS, the only one file at time may be open - if the file is open for reading or was created for writing, no more
files can be open/created until file will be closed.

<span class="blue1">**SLFS Wear leveling**</span>

  The SLFS has a reserved area N * log (metadata) data size, where N is defined by user.
  The SLFS write capacity counter is set to the maximal erasing number value. After each file closing, the counter is decremented.  When the capacity counter reaches the zero value, the log (metadata) is relocated to the next position  in the reserved area and the capacity counter is set to the maximal erasing number value again.
  When there is no more free space available in the reserved area for the metadata relocation, the SLFS is set as read-only.

<span class="blue1">**SLFS API usage**</span>

<span class="blue1">**The flash formatting**</span>

  For the flash formatting, the function **_df_create_mbr()_** is used. Then, one or more SLFS disks may be created
by the function **_slfs_create_disk()_**.

<span class="blue1">**The SLFS initialization**</span>

  After the system restart, the file system(s) should be initiated by the function **_df_init_disks()_**.

<span class="blue1">**File writing**</span>

  File is created for the writing by function **_slfs_file_create()_**.
  The writing is performed by function **_slfs_file_write()_**.
  After finishing writing to the file, the file must be close by the function**_slfs_file_close()_**.

<span class="blue1">**File reading**</span>

  File is open for the reading by function **_slfs_file_open()_**.
  The reading is performed by function **_slfs_file_read()_**.
  After finishing reading from the file, the file must be close by the function **_slfs_file_close()_** or **_slfs_file_close_not_erase()_**.

<span class="blue1">**Files enumeration**</span>

  To get actual number of the files in the SLFS, the function **_slfs_get_files_number()_** should be used.
  To enumerate all files, the functions **_slfs_enum_files_first()/slfs_enum_files_next()_** are used.

<span class="blue1">**SLFS example**</span>

  In SLFS example, a serial flash M25PX64 (8MBytes) and NXP Cortex M3 LPC1766 CPU are used.
  The test program contains a small RS232 CLI (console). The UART is set for 115200 baud, 8 bit, 1 stop bit, no parity, no handshake.

  The CLI supports the following commands (more details in the file _main.c_):

<table cellpadding="0" cellspacing="0" border="0" id="table" class="sortable">

<tbody>

<tr>

<td>_ver_</td>

<td>shows the flash on-chip information: manufacturer id, memory type, memory capacity</td>

</tr>

<tr>

<td>_fs_create_</td>

<td>the flash formatting and test file systems creating</td>

</tr>

<tr>

<td>_fs_dump_</td>

<td>dump the MBR and disks info</td>

</tr>

<tr>

<td>_fs_files_</td>

<td>enumerate and dump actual files at SLFS disk (in example - disk 'B')</td>

</tr>

<tr>

<td>_fs_dinfo_</td>

<td>dump SLFS disk metadata (in example - disk 'B')</td>

</tr>

<tr>

<td>_fs_create_files_</td>

<td>Creates files at SLFS disk (in example - disk 'B')
Usage:  "fs_create_files N", where N - number files to create.
     (for example "fs_create_files 10")</td>

</tr>

<tr>

<td>_fs_chk_files_</td>

<td>Checks files at SLFS disk (in example - disk 'B')
Should be used after "fs_create_files" command.
Usage:  "fs_chk_files N", where N - number files to check.
     (for example "fs_chk_files 10")
The checking assumes file reading and, accordingly, the file erasing.</td>

</tr>

</tbody>

</table>

<span class="blue1">**Test configuration**</span>

  In the file _flash_fs_cfg.h_

  - the flash/disks configuration is defined - flash IC data for the file system(s) creations, the disk(s) number and size etc.

  In the file _main.c_

  - function **_init_fli()_** provides the initialization before creating and/or initialization for the flash / disks.
  - function **_create_flash_fs()_** provides the flash formatting and creates desired disks on the flash
  - functions **_init_fli()_** and **_df_init_disks()_** are doing the flash information initialization and disk(s) initialization after a restart.

  In the file _df_create_init.c_

  - function **_df_chk_flash_ic()_** provides the checking the flash IC on-chip parameters. If your project flash is
    not M25PX64, you should update the function code.

</div>

<div class="tabbertab">

## License

<table cellpadding="0" cellspacing="0" border="0" id="table_1" class="sortable">

<tbody>

<tr>
BSD
</tr>


</tbody>

</table>

</div>

</div>

</div>


<div id="footer">

© 2005, 2017 **Yuri Tiomkin**

</div>

</div>