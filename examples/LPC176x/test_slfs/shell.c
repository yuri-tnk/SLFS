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
#include "../../../cpu/lpc17xx/LPC176x.h"
#include "../../../TNKernel/tn.h"
#include "../../../TNKernel/tn_port.h"
#include "types.h"
#include "globals.h"
#include "shell.h"

int sh_get_command_line_args(unsigned char * cmd_line, unsigned char ** argv);
void sh_cmd_end(SHELLINFO * sh_info, int use_CRLF);
int sh_send(SHELLINFO * sh_info, char * str_to_send);
int sh_stcasecmp(char * s1, char * s2);

#if defined(__CROSSWORKS_ARM)
int strcasecmp(const char *s1, const char *s2);
#endif

//----------------------------------------------------------------------------
int sh_input(SHELLINFO * sh_info, unsigned char ch)
{
	
   if(sh_info->cmd_run == FALSE)  
   {
      if(ch != CR_SYM && sh_info->cl_pos < sh_info->cl_size) // CR or Buffer is full. 
      {
         switch(ch)
         {
            case BS_SYM:
            case DEL_SYM: 
							
              if(sh_info->cl_pos > 0)
              {
                 sh_info->cl_pos--;
                 sh_echo(sh_info, BS_SYM);
                 sh_putchar(sh_info, ' ');
                 sh_putchar(sh_info, BS_SYM);
              }
              break;
							
            default:
							
              if(sh_info->cl_pos + 1 < sh_info->cl_size)
              {
                 // Only printable characters. 
                 if(ch >= SPACE_SYM && ch <= DEL_SYM)
                 {
                    sh_info->cl_buf[sh_info->cl_pos++] = (unsigned char)ch;
                    sh_echo(sh_info, ch);
                 }
              }
              break;
          }
      }
      else //-- cmd received - to process
      {
         sh_info->cl_buf[sh_info->cl_pos] = '\0';
                 
         sh_echo(sh_info, CR_SYM);
         sh_echo(sh_info, LF_SYM);

         sh_info->cmd_run = TRUE;   
         sh_start_cmd_exec(sh_info); // Just signal on  
      }
   }
   else // cmd_run == TRUE
   {
      if(ch == CTRLC_SYM)
        sh_stop_cmd_exec(sh_info); //-- // Just signal on, inside ->set cmd_run = FALSE when actually finished 
   }
	 
	 return 0;
}


//----------------------------------------------------------------------------
int sh_start_cmd_exec(SHELLINFO * sh_info)
{
	 tn_sem_signal(&semCmdExec);
	 return 0;
}
//----------------------------------------------------------------------------
int sh_do_cmd_exec(SHELLINFO * sh_info)
{
	 int i; 
   int argc;
	 SHELLCMD * cmd_ptr;
 	 
   argc = sh_get_command_line_args(sh_info->cl_buf, sh_info->argv);
   if(argc <= 0)
   {
      sh_cmd_end(sh_info, FALSE);		
      return -1;
	}			
		
   for(i=0; i < sh_info->num_cmd; i++)
	{
      cmd_ptr = &sh_info->sh_cmd_arr[i];
      if(strcasecmp((char*)sh_info->argv[0], cmd_ptr->cmd_name) == 0)
      {	 
	      //-- recognize cmd
         cmd_ptr->exec_cmd(sh_info);
	 
         sh_cmd_end(sh_info, TRUE);		
         return 0;
      }
	}
    // If here - cmd not found

   sh_send(sh_info, "Wrong command");	 
   sh_cmd_end(sh_info, TRUE);

   return -1;
}

//----------------------------------------------------------------------------
int sh_send(SHELLINFO * sh_info, char * str_to_send)
{
	 if(sh_info->sh_send_str == NULL)
       return -1;
	 sh_info->sh_send_str((unsigned char*)str_to_send);
	 
	 return 0; //-- OK
}
  
//----------------------------------------------------------------------------
int sh_prompt(SHELLINFO * sh_info, int send_CR_LF)
{
   if(sh_info->sh_send_str == NULL)
      return -1;
	 
   if(send_CR_LF)
      sh_info->sh_send_str((unsigned char*)"\r\n");
   sh_info->sh_send_str((unsigned char*)"SHELL>");

   return 0; //-- OK
}

//----------------------------------------------------------------------------
int sh_stop_cmd_exec(SHELLINFO * sh_info)
{
   sh_info->stop_exec = TRUE;

   return 0;
}

//----------------------------------------------------------------------------
int sh_echo(SHELLINFO * sh_info, unsigned char ch)
{
   if(sh_info->sh_send_char == NULL)
      return -1;
   sh_info->sh_send_char(ch);

	return 0;
}

//----------------------------------------------------------------------------
int sh_putchar(SHELLINFO * sh_info, unsigned char ch)
{
   if(sh_info->sh_send_char == NULL)
	   return -1;
   sh_info->sh_send_char(ch);

   return 0;
}

//----------------------------------------------------------------------------
void sh_cmd_end(SHELLINFO * sh_info, int use_CRLF)
{
   TN_INTSAVE_DATA

   sh_info->cl_pos = 0; 
   sh_info->cl_buf[sh_info->cl_pos] = '\0';

   sh_prompt(sh_info, use_CRLF);

   tn_disable_interrupt(); 
   sh_info->cmd_run = FALSE;   
   tn_enable_interrupt(); 
}
//----------------------------------------------------------------------------
//  Args with whitespaces does not supported !!!
//----------------------------------------------------------------------------
int sh_get_command_line_args(unsigned char * cmd_line, unsigned char ** argv)
{
   int ch;
   int argc = 0;
   int now_arg = FALSE;
   unsigned char * ptr = cmd_line;
   unsigned char * start_ptr = cmd_line;

   for(;;)
   {
      ch = *ptr;
      switch(ch)
      {
         case '\0':
         case ' ':
         case '\t':
            if(now_arg)
            {
               *ptr = '\0';  
               argv[argc] = start_ptr; 
               argc++;
               if(argc >= SH_MAX_ARG) 
                  return argc; 
    
               now_arg = FALSE; 
            }  
            if(ch == '\0')
               return argc; 

            break;

         default:
            if(now_arg == FALSE)
            {
               now_arg = TRUE;
               start_ptr = ptr;   
            }
            break;  
      }
      ptr++; 
   }
}

//----------------------------------------------------------------------------
#if 0
int sh_strcasecmp(char * s1, char * s2)
{
    char ch1, ch2;

    for(;;)
    {
       ch1 = *s1++;
       if(ch1 >= 'A' && ch1 <= 'Z')
          ch1 += 0x20;
			
       ch2 = *s2++;
       if(ch2 >= 'A' && ch2 <= 'Z')
          ch2 += 0x20;

        if(ch1 < ch2)
           return -1;
        if(ch1 > ch2)
           return 1;
        if(ch1 == 0)
           return 0;
    }
}
#endif
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
