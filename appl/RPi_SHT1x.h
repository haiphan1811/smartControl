/*
Raspberry Pi SHT1x communication library.
By:      John Burns (www.john.geek.nz)
Date:    14 August 2012
License: CC BY-SA v3.0 - http://creativecommons.org/licenses/by-sa/3.0/

This is a derivative work based on
	Name: Nice Guy SHT11 library
	By: Daesung Kim
	Date: 04/04/2011
	Source: http://www.theniceguy.net/2722
*/
// Compile with: gcc -o testSHT1x ./../bcm2835-1.3/src/bcm2835.c ./RPi_SHT1x.c testSHT1x.c
/*
Raspberry Pi SHT1x communication library.
By:      John Burns (www.john.geek.nz)
Date:    14 August 2012
License: CC BY-SA v3.0 - http://creativecommons.org/licenses/by-sa/3.0/

This is a derivative work based on
	Name: Nice Guy SHT11 library
	By: Daesung Kim
	Date: 04/04/2011
	Source: http://www.theniceguy.net/2722

Dependencies:
	BCM2835 Raspberry Pi GPIO Library - http://www.open.com.au/mikem/bcm2835/

Sensor:
	Sensirion SHT11 Temperature and Humidity Sensor interfaced to Raspberry Pi GPIO port

SHT pins:
	1. GND  - Connected to GPIO Port P1-06 (Ground)
	2. DATA - Connected via a 10k pullup resistor to GPIO Port P1-01 (3V3 Power)
	2. DATA - Connected to GPIO Port P1-18 (GPIO 24)
	3. SCK  - Connected to GPIO Port P1-16 (GPIO 23)
	4. VDD  - Connected to GPIO Port P1-01 (3V3 Power)

Note:
	GPIO Pins can be changed in the Defines of RPi_SHT1x.h
*/
#ifndef RPI_SHT1x_H_
#define	RPI_SHT1x_H_

// Includes
#include <bcm2835.h>
#include <unistd.h>
#include <stdio.h>

// Defines
//#define	TRUE	1
//#define	FALSE	0
#define SHT1x_DELAY usleep(2)

// Define the Raspberry Pi GPIO Pins for the SHT1x
#define RPI_GPIO_SHT1x_SCK RPI_V2_GPIO_P1_35
#define RPI_GPIO_SHT1x_DATA RPI_V2_GPIO_P1_37

/* Macros to toggle port state of SCK line. */
#define SHT1x_SCK_LO	bcm2835_gpio_write(RPI_GPIO_SHT1x_SCK, LOW)
#define SHT1x_SCK_HI	bcm2835_gpio_write(RPI_GPIO_SHT1x_SCK, HIGH)
#define SHT1x_DATA_LO 	bcm2835_gpio_write(RPI_GPIO_SHT1x_DATA, LOW);\
						bcm2835_gpio_fsel(RPI_GPIO_SHT1x_DATA, BCM2835_GPIO_FSEL_OUTP)
#define	SHT1x_DATA_HI 	bcm2835_gpio_fsel(RPI_GPIO_SHT1x_DATA, BCM2835_GPIO_FSEL_INPT)
#define SHT1x_GET_BIT 	bcm2835_gpio_lev(RPI_GPIO_SHT1x_DATA)

/* Definitions of all known SHT1x commands */
#define SHT1x_MEAS_T	0x03			// Start measuring of temperature.
#define SHT1x_MEAS_RH	0x05			// Start measuring of humidity.
#define SHT1x_STATUS_R	0x07			// Read status register.
#define SHT1x_STATUS_W	0x06			// Write status register.
#define SHT1x_RESET	0x1E			// Perform a sensor soft reset.

/* Enum to select between temperature and humidity measuring */
typedef enum _SHT1xMeasureType {
	SHT1xMeaT	= SHT1x_MEAS_T,		// Temperature
	SHT1xMeaRh	= SHT1x_MEAS_RH		// Humidity
} SHT1xMeasureType;

typedef union 
{
	unsigned short int i;
	float f;
} value;

/* Public Functions ----------------------------------------------------------- */
void SHT1x_Transmission_Start( void );
unsigned char SHT1x_Readbyte( unsigned char sendAck );
unsigned char SHT1x_Sendbyte( unsigned char value );
int SHT1x_InitPins( void );
unsigned char SHT1x_Measure_Start( SHT1xMeasureType type );
unsigned char SHT1x_Get_Measure_Value(unsigned short int * value );
void SHT1x_Reset();
unsigned char SHT1x_Mirrorbyte(unsigned char value);
void SHT1x_Xrc_check(unsigned char value);
void SHT1x_Calc(float *p_humidity ,float *p_temperature);


void ReadTempandHumid(float *Temp,float *Humid);
#endif

