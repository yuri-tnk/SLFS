#ifndef _SLFS_TEST_H_
#define _SLFS_TEST_H_

    //--- slfs_test_utilities.c

int create_random_file_slfs(DF_FLI * fli,
                            SLFS_FILE  * hSLFSFile,
                            int disk_letter, 
                            unsigned char * name,
                            long max_len, //-- [OUT]
                            unsigned char * buf);

int chk_random_file_slfs(DF_FLI * fli, 
                         int disk_letter, 
                         SLFS_FILE * hSLFSFile,
                         unsigned char * buf);

void do_slfs_test(DF_FLI * fli, int disk_letter, unsigned char * buf);

void dump_di_slfs(SLFS_DISKINFO * di, unsigned char * buf);
void dump_slfs_file_info(SLFS_FILE_HEADER * fheader,
                         unsigned __int32 curr_sector,
                         unsigned char * buf);
void dump_all_files_slfs(DF_FLI * fli, 
                         SLFS_FILE * hFile, 
                         int disk_letter, 
                         unsigned char * dump_buf);

void slfs_randomize(void);
unsigned long slfs_rand(void);

#endif
