/*
 * Config.h
 * 
 * Copyright 2017 
 * 
 * This program is written by Phan Thanh Hai
 * Project C - GCC compiler
 * 
 * Last edit: 8/1/2017
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define ON TRUE
#define OFF FALSE

#define DONE 1
#define NOTDONE 0

#define MAXIMUN_NUMBER_COMMAND_HANDLE 			3U
#define MAXIMUN_NODE_CTRL		 		1
#define MAXIMUN_DEVICE				 	2
#define DEVICE_01						0
#define DEVICE_02						1
#define DEVICE_01_NAME					"Light"
#define DEVICE_02_NAME					"Fan"			

#define COMMAND_DEV_O1_ON				"*S01C1N."
#define COMMAND_DEV_O1_OFF				"*S01C1F."
#define COMMAND_DEV_O2_ON				"*S01C2N."
#define COMMAND_DEV_O2_OFF				"*S01C2F."
#define BEGIN_COMMAND					'*'
#define END_COMMAND						'.'

#define UART_RX_TIMEOUT					1

#endif
