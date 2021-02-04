#ifndef _GLOBALS_H_
#define _GLOBALS_H_


extern TN_DQUE  queueRxUART0;

extern  TN_FMP  RxUART0MemPool;

extern  TN_SEM  semTxUart0;
extern  TN_SEM  semMeasPrf;
extern  TN_SEM  semFifoEmptyTxUART0;

extern  TN_SEM  semCmdExec;
#endif
