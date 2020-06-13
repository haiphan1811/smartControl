/*
 * Communication.h
 * 
 * Copyright 2017 
 * 
 * This program is written by Phan Thanh Hai
 * Project C - GCC compiler
 * 
 * Last edit: 7/1/2017
 */
 
 
#ifndef UART_COMMUNICATION_
#define UART_COMMUNICATION_

#include "StdTypes.h"

void ProcessCommand(void);
void* MainProcessCommunicationUART(void *Arg);

#endif
