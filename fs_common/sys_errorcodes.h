#ifndef _SYS_ERRORCODES_H_
#define _SYS_ERRORCODES_H_

#define UNEXPECTED_RC_CODE                   (-10000)

//-------------- Errors ------------------------------------------------------

#define  ERR_NO_ERR                                0

#define  ERR_INTERNAL                            (-2)
#define  ERR_OP_IN_PROGRESS                      (-5)
#define  ERR_TIMEOUT                             (-6)
#define  ERR_END_OP                              (-7)

#define  RC_BAT_LEVEL_CHANGED                   ( -9)
#define  RC_BAT_LOW                             (-10)
#define  RC_SYSBTN_LONG_PRESS                   (-11)
#define  RC_PRESS_TSCR_TIMEOUT                  (-12)
#define  RC_BTN_CHANGED_TO_PRESSED              (-13)
#define  RC_BTN_PRESSED                         (-14)
#define  RC_BTN_CHANGED_TO_RELEASED             (-15)
#define  RC_BTN_RELEASED                        (-16)
#define  RC_BTN_SHORT_PRESS                     (-17)
#define  RC_BTN_LONG_PRESS                      (-18)

#define  ERR_BAD_FILE_CONTENTS                  (-21)
#define  ERR_BAD_FLASH_INFO                     (-22)
#define  ERR_DISK_EMPTY                         (-23)   //-- SLFS only
#define  ERR_NOT_FILE_HEADER_SECTOR             (-24)   //-- SLFS only
#define  ERR_UNKNOWN_FS                         (-25)   //-- New
#define  ERR_WRONG_FS                           (-26)   //-- New
#define  ERR_WR_FINISHED                        (-27)   //-- SLFS only
#define  ERR_DISK_FULL                          (-28)
#define  ERR_EOL                                (-29)

#define  ERR_WRONG_PARAM                        (-30)
#define  ERR_ENTRY_CORRUPTED                    (-31)
#define  ERR_OVERFLOW                           (-32)
#define  ERR_BAD_BOTH_SB                        (-33)
#define  ERR_DISK_NOT_EXISTS                    (-34)
#define  ERR_OUT_OF_RANGE                       (-35)
#define  ERR_BOTH_MBR_CORRUPTED                 (-36)
#define  ERR_FILE_NOT_FOUND                     (-37)
#define  ERR_OP_MODE                            (-38)
#define  ERR_BAD_LEN                            (-39)
#define  ERR_BAD_CRC                            (-40)
#define  ERR_FILE_WR                            (-41)
#define  ERR_FILE_RD                            (-42)
#define  ERR_CANCEL                             (-43)

#define  ERR_OBJ_ALREADY_EXISTS                 (-50)
#define  ERR_OUT_OF_BMP                         (-51)
#define  ERR_BMP_ALLOC_NOT_EMPTY_SECTOR         (-52)
#define  ERR_BMP_FREE_NOT_ALLOC_SECTOR          (-53)
#define  ERR_NO_DISK_SPACE                      (-54)
#define  ERR_UNEXPECTED_CMD                     (-55)
#define  ERR_BAD_CRC8                           (-56)
#define  ERR_BAD_TSIZE                          (-57)
#define  ERR_IN_PROGRESS                        (-58)
#define  ERR_CHAR_NOT_HEX                       (-59)
#define  ERR_BAD_NAMELEN                        (-60)
#define  ERR_BAD_DATALEN                        (-61)
#define  ERR_DISK_NOW_IS_READ_ONLY              (-62)

#define  ERR_DISKINFO_NOT_ASSIGNED              (-64) 


#define  ERR_DISK_WR                            (-67)  
#define  ERR_DISK_RD                            (-68)  


#define  ERR_LAST_PAGE                       (-103)//   (-63)

//------------ SERR -----------------------------

//#define  SERR_NO_ERR                              ERR_NO_ERR
#define  ERR_RD_FILE                           (-70)
#define  ERR_FILE_CORRUPTED                    (-71)
#define  ERR_BAD_PARAM                         (-73)
#define  ERR_STR_TOO_LONG                      (-74)
#define  ERR_NO_ITEMS                          (-75)
#define  ERR_LANGINFO_NOT_EXISTS               (-76)
#define  ERR_HEX_CORRUPTED                     (-77)
#define  ERR_STR_CORRUPTED                     (-78)
#define  ERR_EMPTY_STR                         (-79)
//#define  ERR_WRONG_PARAM                       (-80)

//------------ RV_XX  --------------------------

#define  RV_NO_ERR                              ERR_NO_ERR
#define  RV_USB                                 (-90)
#define  RV_OP_ERR                              (-91)  //-- and sets global g_errorcode
#define  RV_GOTO_RESTART                        (-93)  //-- and sets global g_errorcode
#define  RV_GOTO_MAINSCR                        (-94)  //-- and sets global g_errorcode
#define  RV_GOTO_PREVSCR                        (-95)  //-- and sets global g_errorcode
#define  RV_GOTO_SLEEP_MENU                     (-96)
#define  RV_GOTO_SLEEP                          (-97)  //-- and sets global g_errorcode
#define  RV_GOTO_POWEROFF                       (-98)
#define  RV_COMFIRM_SELECT                      (-99) //-- added by amir

#endif


