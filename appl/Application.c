/*
 * Application.c
 * 
 * Copyright 2017 
 * 
 * This program is written by Phan Thanh Hai
 * Project C - GCC compiler
 * 
 * Last edit: 7/1/2017
 */

#include <string.h>
#include <strings.h>
#include "TCP-IP.h"
#include "TCP-IP.c"
#include <stdbool.h>    // for bool type
#include <stdlib.h> // for itoa() call
#include <time.h>


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <pthread.h>
#include <sys/types.h>

#include "Config.h"
#include "StdTypes.h"
#include "Communication.h"
#include "mysql.c"

#define localhost "127.0.0.1"
#define SET "OK"

char *IdDevice[4][3] = { {"Living Room Light", "L1", NULL},
                    {"Toilet Light", "L2", NULL},
                    {"Bed Room Light", "L3", NULL},
                    {"Font Door", "D1", NULL},
};

void ServerCommand(char *str);
bool IdentifyDevice(int clntSock, char *str);
// Function : handle client
void *HandleThreadClient(void *socket_desc);
// send a command to device 
bool SendCommandToDevice(int index, char *str);
// sizeof(IdDevice)/sizeof(IdDevice[0][0])/2 : the number of row
#define SIZE_OF_ARRY2(array2) (sizeof(array2)/sizeof(array2[0][0])/3)

int servSock;   // global variable

pthread_mutex_t lock;

Std_ReturnType SendCommand(char *Command_pu8);
Std_ReturnType ControlDevice(unsigned char DeviceNo_u8, unsigned char Acction);

typedef enum 
{
	INIT_E = 0x01,
	WAIT_TRIGGER,
	WAIT_TO_SEND_COM_E,
	WAIT_RX_E,
	PROCESS_E,
	SYNC_PROCESS_E,
	UPDATE_E,
	ERROR_E,
} Uart_Receive_state;

typedef struct
{
	unsigned char Device_Number_u8;
	unsigned char NodeControl_u8;
	unsigned char	*Device_Name_pu8;
	unsigned char Status;
} Device_Info_tst;

Device_Info_tst	DeviceHandler_ast[MAXIMUN_DEVICE] = {
	{
	.Device_Number_u8 = DEVICE_01,
	.NodeControl_u8 = 1,
	.Device_Name_pu8 = (unsigned char*) DEVICE_01_NAME,
	.Status = OFF,
	},
	{
	.Device_Number_u8 = DEVICE_02,
	.NodeControl_u8 = 1,
	.Device_Name_pu8 = (unsigned char*) DEVICE_02_NAME,
	.Status = OFF,
	}
};

struct CommandHadler
{
	const char	*Smarthome_Command_apu8[MAXIMUN_NUMBER_COMMAND_HANDLE];
	const char	*Retry_Command_apu8[MAXIMUN_NUMBER_COMMAND_HANDLE];
	unsigned char stCommandStatus[MAXIMUN_NUMBER_COMMAND_HANDLE];
	unsigned char cntCommandNo_u8;
	unsigned char PendingCommand_u8;
	unsigned char Acknowledgement_u8[MAXIMUN_NUMBER_COMMAND_HANDLE];
};
struct CommandHadler CommandHandler_st = 
{ 
	.Smarthome_Command_apu8 = {NULL},
	.Retry_Command_apu8 = {NULL},
	.stCommandStatus = {DONE,DONE,DONE},
	.cntCommandNo_u8 = 0,
	.PendingCommand_u8 = 0,
	.Acknowledgement_u8 = {1,1,1},
};

Uart_Receive_state State_UART_RX = INIT_E;

unsigned char ReceiptionRX_b = FALSE;
unsigned char flag_OutWaitingRX_b = FALSE;
unsigned char flag_TriggerSendCommand_b = FALSE;

void ProcessCommandfromServer(char *command)
{	// Command string: Room Light:State=0 Fan:State=1
	if (*(command+5) == 'L')
	{
		if (*(command+17) == '1') ControlDevice(DEVICE_01, ON);
		else if(*(command+17)  == '0') ControlDevice(DEVICE_01, OFF);
		
	}
	else if (*command  == 'F')
	{
		if (*(command+10)  == '1') ControlDevice(DEVICE_02, ON);
		else if(*(command+10) == '0') ControlDevice(DEVICE_02, OFF);
	}
}

Std_ReturnType SyncDevice_Info(char c)
{
	Std_ReturnType RetVal = E_NOT_OK;
	int i = 0;
	if (c == '1') // *01SS.
	{
		for( i = 0; i < MAXIMUN_DEVICE; i++)
		{
			fprintf(stdout, "D %d status %d \n",DeviceHandler_ast[i].Device_Number_u8,DeviceHandler_ast[i].Status);
			ControlDevice(DeviceHandler_ast[i].Device_Number_u8, DeviceHandler_ast[i].Status);
		}
		RetVal = E_OK;
	}
	else if ( c == '2')
	{
		
	}
	else
	{
		
	}
	return RetVal;
}

Std_ReturnType SendCommand(char *Command_pu8)
{
	Std_ReturnType RetVal = E_NOT_OK;
	int i = 0;
	if ( NULL != Command_pu8 )
	{
		fprintf(stdout, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		for (i = 0; i < MAXIMUN_NUMBER_COMMAND_HANDLE ; i++)
		{
			fprintf(stdout, "Handler command index: %d -> Available: %d \n", i, CommandHandler_st.stCommandStatus[i]);
			if ((CommandHandler_st.Smarthome_Command_apu8[i] == NULL) && (CommandHandler_st.stCommandStatus[i] == DONE))
			{
				
				pthread_mutex_lock(&lock);
				CommandHandler_st.Smarthome_Command_apu8[i] = (const char *)Command_pu8;
				CommandHandler_st.stCommandStatus[i] = NOTDONE;
				CommandHandler_st.Retry_Command_apu8[i] = (const char *)Command_pu8;
				fprintf(stdout, "Command order %d : %s was registed into Command Handler at index: %d \n", i + 1, CommandHandler_st.Smarthome_Command_apu8[i], i);
				CommandHandler_st.PendingCommand_u8++;
				flag_TriggerSendCommand_b = TRUE;
				RetVal = E_OK;
				pthread_mutex_unlock(&lock);
				
				break;
			}
			else
			{
				RetVal = E_NOT_OK;
			}
		}
	}
	return RetVal;
}

Std_ReturnType ControlDevice(unsigned char DeviceNo_u8, unsigned char Acction)
{
	Std_ReturnType RetVal = E_NOT_OK;
	if( DeviceNo_u8 == DEVICE_01)
	{
		if ( Acction == ON )
		{fprintf(stdout, "CONTROL DEVICE 1 ON\n");
			SendCommand(COMMAND_DEV_O1_ON);
			RetVal = E_OK;
		}
		else if ( Acction == OFF)
		{
			SendCommand(COMMAND_DEV_O1_OFF);
			RetVal = E_OK;
		}
	}
	else if (DeviceNo_u8 == DEVICE_02)
	{
		if ( Acction == ON )
		{
			fprintf(stdout, "CONTROL DEVICE 2 ON\n");
			SendCommand(COMMAND_DEV_O2_ON);
			RetVal = E_OK;
		}
		else if ( Acction == OFF)
		{
			SendCommand(COMMAND_DEV_O2_OFF);
			RetVal = E_OK;
		}
	}		
	return RetVal;
}

void ProcessCommand(void)
{
	int fd;
	time_t Time_WaitingforRX_Instance; // in second
	struct timespec spec;
	unsigned int i = 0;
	unsigned char UartRX_Buffer_u8[20];
	switch (State_UART_RX)
	{
		case INIT_E:
			printf("Uart Polling is initialized.... \n");
			if((fd = serialOpen ("/dev/ttyAMA0", 115200)) < 0 )
			{
				fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno));
				State_UART_RX = ERROR_E;
			}
			else
			{
				fflush (stdout);
				serialFlush(fd);
				ControlDevice(DEVICE_01, ON);
				ControlDevice(DEVICE_02, ON);
				printf("\nUART state is changed to WAIT_TO_SEND_COM_E \n");
				State_UART_RX = WAIT_TRIGGER;
			}
		break;
		
		case WAIT_TRIGGER:
			usleep(30);
			if ( TRUE == flag_TriggerSendCommand_b)
			{
				printf("\nUART state is changed to WAIT_TO_SEND_COM_E \n");
				State_UART_RX = WAIT_TO_SEND_COM_E;
			}
			if ( 5 < serialDataAvail(fd))
			{
				if ( 7 < serialDataAvail(fd))
				{
					while(serialDataAvail(fd) > 0)
					{
						fprintf(stdout, "i = %d : ",i);
						UartRX_Buffer_u8[i] = serialGetchar(fd);
						fprintf(stdout, "%c \n",UartRX_Buffer_u8[i]);
						if (UartRX_Buffer_u8[0] == BEGIN_COMMAND)
						{
							if (UartRX_Buffer_u8[i] == END_COMMAND)
							{
								fprintf(stdout, "OUT While loop for update command\n");
								UartRX_Buffer_u8[i+1] = '\0';
								if (i == 6) serialFlush(fd);
								fprintf(stdout, "Buffer Update Request Data: %s \n", UartRX_Buffer_u8);
								fprintf(stdout, "\nUART state is changed to UPDATE_E after Recieving a Update Request\n");
								State_UART_RX = UPDATE_E;
								break;			
							}
							i++;
						}
					}
				}
				else if ( 6 == serialDataAvail(fd))
				{
					while(serialDataAvail(fd) > 0)
					{
						fprintf(stdout, "i = %d : ",i);
						UartRX_Buffer_u8[i] = serialGetchar(fd);
						fprintf(stdout, "%c \n",UartRX_Buffer_u8[i]);
						if (UartRX_Buffer_u8[0] == BEGIN_COMMAND)
						{
							if (UartRX_Buffer_u8[i] == END_COMMAND)
							{
								fprintf(stdout, "OUT While loop for recieved sync command\n");
								UartRX_Buffer_u8[i+1] = '\0';
								if (i == 6) serialFlush(fd);
								fprintf(stdout, "Buffer Sync Request Data: %s \n", UartRX_Buffer_u8);
								fprintf(stdout, "\nUART state is changed to SYNC_PROCESS_E after Recieving a Sync Request\n");
								State_UART_RX = SYNC_PROCESS_E;
								break;			
							}
							i++;
						}
					}
				}
				else
				{
					fprintf(stdout, "Buffer Bytes: %d\n", serialDataAvail(fd));
					serialFlush(fd);
					fflush (stdout);
				}
				
			}
			
		break;
		
		case WAIT_TO_SEND_COM_E: /* ------------------------WAIT_TO_SEND_COM_E--------------------------------------------------------------------------------------------- */
				fprintf(stdout, "======================================\n");
				fprintf(stdout, "\nCommand Pending is needs to send %d\n", CommandHandler_st.PendingCommand_u8);
				//fprintf(stdout, "\nIndex:0 - Command:%s - ACK: %d\n", CommandHandler_st.Smarthome_Command_apu8[0],CommandHandler_st.Acknowledgement_u8[0]);
				//fprintf(stdout, "Index:1 - Command:%s - ACK: %d\n", CommandHandler_st.Smarthome_Command_apu8[1],CommandHandler_st.Acknowledgement_u8[1]);
				//fprintf(stdout, "Index:2 - Command:%s - ACK: %d\n\n", CommandHandler_st.Smarthome_Command_apu8[2],CommandHandler_st.Acknowledgement_u8[2]);
				if ((CommandHandler_st.PendingCommand_u8 > 0) && (CommandHandler_st.Acknowledgement_u8[CommandHandler_st.cntCommandNo_u8] ==TRUE))
				{
					//fprintf(stdout, "Number of Pending Command : %d \n", CommandHandler_st.PendingCommand_u8);
					//fprintf(stdout, "Index of cntCommandNo_u8 : %d \n", CommandHandler_st.cntCommandNo_u8 );
					//fprintf(stdout, "\nIndex:0 - Command:%s - Done or Not: %d\n", CommandHandler_st.Smarthome_Command_apu8[0],CommandHandler_st.stCommandStatus[0]);
					//fprintf(stdout, "Index:1 - Command:%s - Done or Not: %d\n", CommandHandler_st.Smarthome_Command_apu8[1],CommandHandler_st.stCommandStatus[1]);
					//fprintf(stdout, "Index:2 - Command:%s - Done or Not: %d\n\n", CommandHandler_st.Smarthome_Command_apu8[2],CommandHandler_st.stCommandStatus[2]);

					for (i = 0; i < MAXIMUN_NUMBER_COMMAND_HANDLE; i++)
					{
						if ((CommandHandler_st.Smarthome_Command_apu8[i] != NULL) 
								&& (CommandHandler_st.stCommandStatus[i] == NOTDONE))
						{
							CommandHandler_st.cntCommandNo_u8 = i;
							serialPuts(fd, CommandHandler_st.Smarthome_Command_apu8[CommandHandler_st.cntCommandNo_u8]);
							serialFlush(fd);
							fprintf(stdout, "Command %d: %s is sent.\n", CommandHandler_st.cntCommandNo_u8+1, CommandHandler_st.Smarthome_Command_apu8[CommandHandler_st.cntCommandNo_u8]);
							fflush(stdout);
							CommandHandler_st.Acknowledgement_u8[CommandHandler_st.cntCommandNo_u8] = FALSE;
							CommandHandler_st.Smarthome_Command_apu8[CommandHandler_st.cntCommandNo_u8] = NULL;
							//fprintf(stdout, "The number of command pending for next turn: %d\n", CommandHandler_st.PendingCommand_u8);
							fprintf(stdout, "\nUART state is changed to WAIT_RX_E \n");
							State_UART_RX = WAIT_RX_E;
							break;
						}
					}
				}
				else if (CommandHandler_st.Acknowledgement_u8[CommandHandler_st.cntCommandNo_u8] == FALSE)
				{
					fprintf(stdout, "Resend the Command \n");
					fprintf(stdout, "Number of Pending Command : %d \n", CommandHandler_st.PendingCommand_u8);
					fprintf(stdout, "Index cntCommandNo_u8 : %d \n", CommandHandler_st.cntCommandNo_u8 );
					if( CommandHandler_st.Retry_Command_apu8[CommandHandler_st.cntCommandNo_u8])
					{ 	fprintf(stdout, "Resend the Command CONTENT %s\n",CommandHandler_st.Retry_Command_apu8[CommandHandler_st.cntCommandNo_u8]);
						serialPuts(fd, CommandHandler_st.Retry_Command_apu8[CommandHandler_st.cntCommandNo_u8]);
						serialFlush(fd);
						fprintf(stdout, "ReSend Command : %s is sent.\n", CommandHandler_st.Retry_Command_apu8[CommandHandler_st.cntCommandNo_u8]);
						fflush(stdout);
						fprintf(stdout, "\nUART state is changed to WAIT_RX_E after Resending Command\n");
						State_UART_RX = WAIT_RX_E;
					}
				}	

				if (CommandHandler_st.PendingCommand_u8 == 0) 
				{
					pthread_mutex_lock(&lock);
					flag_TriggerSendCommand_b = FALSE;
					fprintf(stdout, "\nUART state is changed to WAIT_TRIGGER.\n");
					State_UART_RX = WAIT_TRIGGER;
					fprintf(stdout, "THERE IS NO COMMAND !!!!!!\n================================================================================\n");
					CommandHandler_st.cntCommandNo_u8 = 0;
					pthread_mutex_unlock(&lock);

				}
			
			break;
			
		case WAIT_RX_E:  /* ------------------------WAIT_RX_E--------------------------------------------------------------------------------------------- */
			clock_gettime(CLOCK_MONOTONIC, &spec);
			Time_WaitingforRX_Instance = spec.tv_sec;
			while(1)
			{
				while(serialDataAvail(fd) > 0)
				{
					fprintf(stdout, "i = %d : ",i);
					if (i == 7) ReceiptionRX_b = TRUE;
					UartRX_Buffer_u8[i] = serialGetchar(fd);
					fprintf(stdout, "%c \n",UartRX_Buffer_u8[i]);
					if (UartRX_Buffer_u8[0] == BEGIN_COMMAND)
					{
						if (UartRX_Buffer_u8[i] == END_COMMAND)
						{
							UartRX_Buffer_u8[i+1] = '\0';
							fprintf(stdout, "OUT While loop\n");
							flag_OutWaitingRX_b = TRUE;	
							break;			
						}
						i++;
					}
				}
				
				if (flag_OutWaitingRX_b == TRUE)
				{
					fprintf(stdout, "Buffer of Recieved Command: %s \n",UartRX_Buffer_u8);
					flag_OutWaitingRX_b = FALSE;
					ReceiptionRX_b = FALSE;
					fprintf(stdout, "\nUART state is changed to PROCESS_E \n");
					State_UART_RX = PROCESS_E;	
					break;
				}
				else
				{
					// Do nothing
				}
				
				if (ReceiptionRX_b == TRUE)   // Rarely 
				{
					fprintf(stdout, "Buffer Failed: %s \n",UartRX_Buffer_u8);
					fprintf(stdout, "FLUSH BUFFER AFTER FAILED REVIEVING COMMAND\n");
					serialFlush(fd);
					fflush (stdout);
					if(i < 7)
					{
						fprintf(stdout, "\nUART state is changed to WAIT_TO_SEND_COM_E for ResendCommand \n");
						State_UART_RX = WAIT_TO_SEND_COM_E;	
					}
					i = 0;
					ReceiptionRX_b = FALSE;
				}
				else 
				{
					
				}
				clock_gettime(CLOCK_MONOTONIC, &spec);
				if ( (UART_RX_TIMEOUT + Time_WaitingforRX_Instance) < spec.tv_sec )
				{
					fprintf(stdout, "\nUART state is changed to WAIT_TO_SEND_COM_E Because TIME_OUT \n");
					State_UART_RX = WAIT_TO_SEND_COM_E;
					break;
				}
				
			}
			break;
		
		case PROCESS_E: /* ------------------------PROCESS_E--------------------------------------------------------------------------------------------- */
			// *O1SR1N.
			// *01SR1F.
			// *01SR1E.
			fprintf(stdout, "FROM node: %c - To node: %c \n", UartRX_Buffer_u8[1], UartRX_Buffer_u8[3]);
			
			if (UartRX_Buffer_u8[4] == 'R')
			{
				if (UartRX_Buffer_u8[3] == 'S')
				{
					pthread_mutex_lock(&lock);
					CommandHandler_st.PendingCommand_u8--;
					CommandHandler_st.Acknowledgement_u8[CommandHandler_st.cntCommandNo_u8] = TRUE;
					CommandHandler_st.stCommandStatus[CommandHandler_st.cntCommandNo_u8] = DONE;
					fprintf(stdout, "DONE: %d - Index command:%d\n", CommandHandler_st.stCommandStatus[CommandHandler_st.cntCommandNo_u8],CommandHandler_st.cntCommandNo_u8);
					pthread_mutex_unlock(&lock);
					if (UartRX_Buffer_u8[2] == '1')
					{
						fprintf(stdout, "Node 2 is response with message: %s \n", UartRX_Buffer_u8);
						if(UartRX_Buffer_u8[5] == '1') // Response from Node1 about D1
						{
							if (UartRX_Buffer_u8[6] == 'N')
							{
								DeviceHandler_ast[DEVICE_01].Status = ON;
								UpdateStatusDevice(DEVICE_01, ON);
								fprintf(stdout, "Updated Status for %s is ON \n",DeviceHandler_ast[DEVICE_01].Device_Name_pu8);
							}
							else if (UartRX_Buffer_u8[6] == 'F')
							{
								DeviceHandler_ast[DEVICE_01].Status = OFF;
								UpdateStatusDevice(DEVICE_01, OFF);
								fprintf(stdout, "Updated Status for %s is OFF \n",DeviceHandler_ast[DEVICE_01].Device_Name_pu8);
							}
							else if (UartRX_Buffer_u8[6] == 'E')
							{
								printf("\nUART state is changed to ERROR_E \n");
								State_UART_RX = ERROR_E;
							}
							else
							{
								
							}
						}
						else if (UartRX_Buffer_u8[5] == '2')  // Response from Node1 about D2
						{
							if (UartRX_Buffer_u8[6] == 'N')
							{
								DeviceHandler_ast[DEVICE_02].Status = ON;
								UpdateStatusDevice(DEVICE_02, ON);
								printf("Updated Status for %s is ON \n",DeviceHandler_ast[DEVICE_02].Device_Name_pu8);
							}
							else if (UartRX_Buffer_u8[6] == 'F')
							{
								DeviceHandler_ast[DEVICE_02].Status = OFF;
								UpdateStatusDevice(DEVICE_02, OFF);
								printf("Updated Status for %s is OFF \n",DeviceHandler_ast[DEVICE_02].Device_Name_pu8);

							}
							else if (UartRX_Buffer_u8[6] == 'E')
							{
								printf("ERROR\n");
								printf("\nUART state is changed to ERROR_E \n");
								State_UART_RX = ERROR_E;
							}
							else
							{
								
							}
						}
					}
					
					printf("\nUART state is changed to WAIT_TO_SEND_COM_E \n");
					State_UART_RX = WAIT_TO_SEND_COM_E;
				}
				else
				{
					printf("Ignore message! \n");
					printf("\nUART state is changed to WAIT_TO_SEND_COM_E \n");
					State_UART_RX = WAIT_TO_SEND_COM_E;
				}
			}
			else
			{
					fprintf(stdout, "\nUART state is changed to WAIT_TO_SEND_COM_E after Response Error \n");
					State_UART_RX = WAIT_TO_SEND_COM_E;
			}
		break;
		
		case SYNC_PROCESS_E: /* ------------------------SYNC_PROCESS_E--------------------------------------------------------------------------------------------- */
				/* Sync Request Form: *01SS.  */
				if ((UartRX_Buffer_u8[3] == 'S') && (UartRX_Buffer_u8[4] == 'S'))
				{
					if (E_OK == SyncDevice_Info(UartRX_Buffer_u8[2]))
					{
						fprintf(stdout, "\nUART state is changed to WAIT_TRIGGER after Sync Data \n");
						State_UART_RX = WAIT_TRIGGER;
					}
					else
					{
						
					}
				}
				else
				{
					
				}	
		break;
		
		case UPDATE_E: /* ------------------------UPDATE_E--------------------------------------------------------------------------------------------- */
				/* Update Form: *01SU1N.  */
				if ((UartRX_Buffer_u8[3] == 'S') && (UartRX_Buffer_u8[4] == 'U'))
				{
					if (UartRX_Buffer_u8[5] == '1')
					{
						if (UartRX_Buffer_u8[6] == 'N')	UpdateStatusDevice(DEVICE_01,ON);
						else if (UartRX_Buffer_u8[6] == 'F')	UpdateStatusDevice(DEVICE_01,OFF);
						fprintf(stdout, "\nUART state is changed to WAIT_TRIGGER after Update State of D1\n");
						State_UART_RX = WAIT_TRIGGER;
					}
					else if (UartRX_Buffer_u8[5] == '2')
					{
						if (UartRX_Buffer_u8[6] == 'N')	UpdateStatusDevice(DEVICE_02,ON);
						else if (UartRX_Buffer_u8[6] == 'F')	UpdateStatusDevice(DEVICE_02,OFF);
						fprintf(stdout, "\nUART state is changed to WAIT_TRIGGER after Update State of D2\n");
						State_UART_RX = WAIT_TRIGGER;
					}
				}
				else
				{
					
				}	
		break;
		
		case ERROR_E: /* ------------------------ERROR_E--------------------------------------------------------------------------------------------- */

		break;
		
		default: /* ------------------------default--------------------------------------------------------------------------------------------- */
		break;
	}
}

void* MonitoringTempandHumid(void *Arg)
{
	float Temperature;
	float Humidity;
	// Set up the SHT1x Data and Clock Pins
	SHT1x_InitPins();
	while(1)
	{
		ReadTempandHumid( &Temperature, &Humidity);
		UpdateTempHumid(Temperature, Humidity);
		sleep(50);
	}
	return NULL;
}

void* MainProcessCommunicationUART(void *Arg)
{
	while(1)
	{   
		ProcessCommand();
	}
	return NULL;
}

void* CreateClient_Thread(void* Arg)
{
	int clntSock;
    
    struct sockaddr_in cli_addr;
    pthread_t threadID;              /* Thread ID from pthread_create() */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    struct ThreadArgs *threadArgs;   /* Pointer to argument structure for thread */
    unsigned int clntLen;            /* Length of client address data structure */

    clntLen = sizeof(cli_addr);
	fprintf(stdout, "Before CreateTCPServeerSocket\n");
    servSock = CreateTCPServerSocket(PORT);
	fprintf(stdout, "After CreateTCPServeerSocket\n");
    while(1) 
    {
		fprintf(stdout, "Wait for client connect ....\n");
        clntSock = AcceptTCPConnection(servSock);

		fprintf(stdout, "Wait for data socket RX ...\n");
        getpeername(clntSock, (struct sockaddr *) &cli_addr, &clntLen);
		
        /* Create separate memory for client argument */
        if ((threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs))) == NULL)
            error("malloc() failed");

        threadArgs -> clntSock = clntSock;
        threadArgs -> addr = inet_ntoa(cli_addr.sin_addr);
		fprintf(stdout, "Before Pthreadcreate\n");
        if (pthread_create(&threadID, &attr, HandleThreadClient, (void *) threadArgs) != 0)
            error("pthread_create() failed");
         fprintf(stdout, "Before Pthreadcreate\n");
	}
        // printf("\n+ New client[%d][Addr:%s]\n\n", 
        //     clntSock, inet_ntoa(cli_addr.sin_addr));
    close(servSock);
    return 0; 
}

int main(int argc, char *argv[])
{
    pthread_t thread[3];              /* Thread ID from pthread_create() */

     if (pthread_create(&thread[0], NULL, CreateClient_Thread, NULL) != 0)
        fprintf(stdout, "Can't create thread \n");
    else
        fprintf(stdout, "Thread CreateClient_Thread was created successfully\n");

        
    if (pthread_create(&thread[1], NULL, MainProcessCommunicationUART, NULL) != 0)
        fprintf(stdout, "Can't create thread \n");
    else
        fprintf(stdout, "Thread MainProcessCommunicationUART was created successfully\n");
        
    if (pthread_create(&thread[2], NULL, MonitoringTempandHumid, NULL) != 0)
        fprintf(stdout, "Can't create thread \n");
    else
        fprintf(stdout, "Thread MonitoringTempandHumid was created successfully\n");
        
        pthread_join(thread[0], NULL);
        fprintf(stdout, "Thread Communication was joined successfully\n");
       // pthread_join(thread[0], NULL);
        fprintf(stdout, "Thread CreateClient_Thread was joined successfully\n");
	
    return 0; 
}

void *HandleThreadClient(void *threadArgs){
    int clntSock;
    int recvMsgSize;
    char buffer[BUFFSIZE];
    bzero(buffer,BUFFSIZE);

    clntSock = ((struct ThreadArgs *) threadArgs) -> clntSock;
    
    while(1)
    {
		recvMsgSize = recv(clntSock,buffer,BUFFSIZE,0);
		fprintf(stdout, "Recieving message from socket.\n");
        if (recvMsgSize < 0) {
            printf("ERROR reading from socket\n");
        }
        else if(recvMsgSize>0){
            
            if( !strcmp(((struct ThreadArgs *) threadArgs) -> addr, localhost) ) // if data from server 
            {
                printf(". Web server: %s\n", buffer);
                //ServerCommand(buffer);
                ProcessCommandfromServer(buffer);
            }
            else 
            {
                printf(". Device: %s\n", buffer);
                // Check command of device
                char *header = strtok(buffer, ":");
    			char *content = strtok(NULL, ":");
                if(strcmp(header,"RESULT")==0) { // result command
                    //response from device
                    printf(". Address[%s]: %s\n", ((struct ThreadArgs *) threadArgs) -> addr, content);
                } else if (strcmp(header,"INIT")==0) {    // Init a new device command
                    IdentifyDevice(clntSock, content);
                } else {
                    // other command
                }
            }
            bzero(buffer,strlen(buffer));
        }
        else {
            if( !strcmp(((struct ThreadArgs *) threadArgs) -> addr, localhost) ) {
                // printf("- Web server disconnected\n");
            }
            else {
                printf("- Address[%s]: disconnected !\n", ((struct ThreadArgs *) threadArgs) -> addr);
                // u can handle with client-disconnected event
                // code here
            }
            
            break;
        }     
    }
    close(clntSock);
    return NULL;
}

void ServerCommand(char *str) {

    char *header = strtok(str, ":");
    char *content = strtok(NULL, ":");
    int i = 0;
    for (i = 0; i < SIZE_OF_ARRY2(IdDevice); ++i) { 
        if(IdDevice[i][0]) // check if IdDevice[i][0] is not NULL
            if(strcmp(header, IdDevice[i][0])==0) {
                // printf("header: %s, IdDevice: %s, content: %s\n", header, IdDevice[i][1], content);
                if(IdDevice[i][2]!=NULL)    // check if has device
                    SendCommandToDevice(i, content);
                else
                    printf("- There is no device: \"%s\"\n", IdDevice[i][0]);
                // break;       // break when u just wanna send to one device
            }
    }
}

bool IdentifyDevice(int clntSock, char *str) {

    int i = 0;
    for (i = 0; i < SIZE_OF_ARRY2(IdDevice); ++i) { 
        if(IdDevice[i][0]) // check if IdDevice[i][0] is not NULL
            if(strcmp(str, IdDevice[i][1])==0) {
                printf("+ Detecting a new device: %s, ID:%s\n\n", IdDevice[i][0], IdDevice[i][1]);
                if ( send(clntSock, SET,strlen(SET),0) < 0) { // send "OK"
                    printf("- Sending \"OK\" to device: \"%s\" false\n", IdDevice[i][0]);
                    return false;
                }
                char clientAddr[4]; // save client socket as string, max is "9999"
                sprintf(clientAddr, "%d", clntSock);    // convert to string
                IdDevice[i][2] = strdup(clientAddr);    // save to IdDevice
                return true;
            }
    }
    return false;
}

bool SendCommandToDevice(int index, char *str) {

    int clntSock = atoi(IdDevice[index][2]);
    if ( send(clntSock, str,strlen(str),0) < 0) {
        printf("- Sending to device: \"%s\" failed\n", IdDevice[index][0]);
        return false;
    }
    printf(". Sending to device: \"%s\", content: %s\n", IdDevice[index][0], str);

    return false;
}
