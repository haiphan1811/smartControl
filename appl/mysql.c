#include <my_global.h>
#include <mysql.h>
#include <string.h>
#include <stdlib.h>

#include <wiringPi.h>
#include <stdio.h>
#include "RPi_SHT1x.h"
#include "RPi_SHT1x.c"
#include <time.h>

#define SERVER_NAME "localhost"
#define USER_NAME 	"root"
#define PASSWORD 	"thanhtu140897"
#define DATABASE 	"home"
#define TABLE 		"temphumid"
#define TEMP_UPDATE_QUERY		"UPDATE temphumid SET value= WHERE id=1"
#define HUMID_UPDATE_QUERY		"UPDATE temphumid SET value= WHERE id=2"

#define LIGHT_UPDATE_QUERY		"UPDATE device SET state= WHERE id=23"
#define FAN_UPDATE_QUERY		"UPDATE device SET state= WHERE id=25"

void finish_with_error(MYSQL *connection);
void UpdateTempHumid(float Temp_Value, float Humid_Value);
void UpdateStatusDevice(int DeviceNo, unsigned char Status);
void* MonitoringTempandHumid(void *Arg);

void finish_with_error(MYSQL *connection)
{
  fprintf(stderr, "%s\n", mysql_error(connection));
  mysql_close(connection);
  exit(1);
}

void UpdateStatusDevice(int DeviceNo, unsigned char Status)
{   	// Insert value in string form to string query
	char *LightFormQuery = LIGHT_UPDATE_QUERY;
	char *FanFormQuery = FAN_UPDATE_QUERY;
	char Light_Query_Command[50];
	char Fan_Query_Command[50];
	
	MYSQL *conn = mysql_init(NULL);
	if (conn == NULL) 
	{
	  fprintf(stderr, "%s\n", mysql_error(conn));
	  exit(1);
	}

	if (mysql_real_connect(conn, SERVER_NAME, USER_NAME, PASSWORD, 
		  DATABASE, 0, NULL, 0) == NULL) 
	{
		finish_with_error(conn);
	}  
	
	if (DeviceNo == DEVICE_01)
	{
		//Temp first
		strncpy(Light_Query_Command, LightFormQuery ,24);
		Light_Query_Command[24] = '\0';
		if (Status == TRUE)strcat(Light_Query_Command,"1");
		else strcat(Light_Query_Command,"0");
		strcat(Light_Query_Command, LightFormQuery+24);
		printf("%s\n",Light_Query_Command);
		if (mysql_query(conn, Light_Query_Command)) 
		{      
			finish_with_error(conn);
		}
	}
	
	else if (DeviceNo == DEVICE_02)
	{
			//Humid next
		strncpy(Fan_Query_Command, FanFormQuery ,24);
		Fan_Query_Command[24] = '\0';
		if (Status == TRUE)strcat(Fan_Query_Command,"1");
		else strcat(Fan_Query_Command,"0");
		strcat(Fan_Query_Command, FanFormQuery + 24);
		printf("%s\n",Fan_Query_Command);
		if (mysql_query(conn, Fan_Query_Command)) 
		{      
			finish_with_error(conn);
		}
	}

	  mysql_close(conn);
	  //exit(0);
}

void UpdateTempHumid(float Temp_Value, float Humid_Value)
{
	
	char *TempValue_StringForm_pu8 = malloc(4);
	char *HumidValue_StringForm_pu8 = malloc(4);

	// Convert floant to string
	if (Temp_Value > 0)
	{
		snprintf(TempValue_StringForm_pu8,5,"%.1f",Temp_Value);
		printf("Temperature: %s \n", TempValue_StringForm_pu8);
	}
	if (Humid_Value >0)
	{
		snprintf(HumidValue_StringForm_pu8,5,"%.1f",Humid_Value);
		printf("Humidity: %s \n", HumidValue_StringForm_pu8);
	}

	// Insert value in string form to string query
	char *TempFormQuery = TEMP_UPDATE_QUERY;
	char *HumidFormQuery = HUMID_UPDATE_QUERY;
	char Temp_Query_Command[50];
	char Humid_Query_Command[50];

	//Temp first
	strncpy(Temp_Query_Command, TempFormQuery ,27);
	Temp_Query_Command[27] = '\0';
	strcat(Temp_Query_Command,TempValue_StringForm_pu8);
	strcat(Temp_Query_Command, TempFormQuery+27);
	printf("%s\n",Temp_Query_Command);

	//Humid next
	strncpy(Humid_Query_Command, HumidFormQuery ,27);
	Humid_Query_Command[27] = '\0';
	strcat(Humid_Query_Command,HumidValue_StringForm_pu8);
	strcat(Humid_Query_Command, HumidFormQuery + 27);
	printf("%s\n",Humid_Query_Command);
    
    free(TempValue_StringForm_pu8);
    free(HumidValue_StringForm_pu8);
    
	MYSQL *conn = mysql_init(NULL);

	if (conn == NULL) 
	{
	  fprintf(stderr, "%s\n", mysql_error(conn));
	  exit(1);
	}

	if (mysql_real_connect(conn, SERVER_NAME, USER_NAME, PASSWORD, 
		  DATABASE, 0, NULL, 0) == NULL) 
	{
		finish_with_error(conn);
	}  
	
	if (mysql_query(conn, Temp_Query_Command)) 
	{      
		finish_with_error(conn);
	}
	if (mysql_query(conn, Humid_Query_Command)) 
	{      
		finish_with_error(conn);
	}

	  mysql_close(conn);
	  //exit(0);
}
