#ifndef  _SHELL_H_
#define  _SHELL_H_

struct _SHELLINFO;

typedef int(*sh_exec_cmd_handler)(struct _SHELLINFO * sh_info);
typedef void (*sh_send_str_handler)(unsigned char * buf);
typedef void (*sh_send_char_handler)(unsigned char ch);


typedef struct _SHELLCMD
{
 	char * cmd_name;
	sh_exec_cmd_handler exec_cmd;
	
}SHELLCMD;

struct _SHELLINFO
{
   int cmd_run;
   int cl_pos; 
   int cl_size;
   unsigned char * cl_buf;
   int stop_exec;
	
   unsigned char ** argv;

   int num_cmd;
	SHELLCMD * sh_cmd_arr;

   sh_send_str_handler  sh_send_str;
   sh_send_char_handler sh_send_char;
};

typedef struct _SHELLINFO SHELLINFO;

#define  SH_EXECUTE     8
#define  SH_BREAK       3

#define  CTRLC_SYM      3
#define  BS_SYM         8
#define  LF_SYM        10
#define  CR_SYM        13
#define  ESC_SYM       27
#define  SPACE_SYM     32 
#define  DEL_SYM     0x7F 

#define  SH_MAX_ARG     4

int sh_input(SHELLINFO * sh_info, unsigned char ch);
int sh_start_cmd_exec(SHELLINFO * sh_info);
int sh_stop_cmd_exec(SHELLINFO * sh_info);
int sh_do_cmd_exec(SHELLINFO * sh_info);

int sh_echo(SHELLINFO * sh_info, unsigned char ch);
int sh_putchar(SHELLINFO * sh_info, unsigned char ch);

int sh_prompt(SHELLINFO * sh_info, int send_CR_LF);
 
#endif /*  _SHELL_H_  */
