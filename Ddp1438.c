/****************************************************************************/
/* Ddp1438.c                                                                */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Ddp1438.h"
#include "CyUSBSerial.h"
#include "CyFunction.h"

#define CY_I2C_INTF_NUM     0
#define CY_SPI_INTF_NUM     1
#define CY_GPIO_INTF_NUM    2

#define WRITE_RESTART_READ_DELAY	100*1000
#define LIGHT_SENSOR_DELAY		   1000*1000

#define D(x) x

/***********************GPIO Interface***********************/

CY_RETURN_STATUS GpioSetProjectorOnOff( BOOL enable , int dN)
{
	CY_RETURN_STATUS rStatus;
	rStatus = GpioWrite( dN, CY_GPIO_INTF_NUM, PROJ_ON, enable );
	if( rStatus != CY_SUCCESS )
	{
		puts("## GpioSetProjectorOnOff: GpioWrite Fail ##");
		return rStatus;
	}
	return rStatus;
}


CY_RETURN_STATUS GpioSetLedOnOff( BOOL enable , int dN)
{
	CY_RETURN_STATUS rStatus;
	rStatus = GpioWrite( dN, CY_GPIO_INTF_NUM, LED_SW, enable );
	if( rStatus != CY_SUCCESS)
	{
		puts("## GpioSetLedOnOff: GpioWrite Fail ##");
		return rStatus;
	}
	return rStatus;
}

CY_RETURN_STATUS GpioGetAsicReady( BOOL *isReady , int dN)
{
	CY_RETURN_STATUS rStatus;
	rStatus = GpioRead( dN, CY_GPIO_INTF_NUM, ASIC_READY, isReady );
	if( rStatus != CY_SUCCESS )
	{
		puts("## GpioGetAsicReady: GpioRead Fail ##");
		return rStatus;
	}
	return rStatus;
}

/* es
CY_RETURN_STATUS GpioGetI2CBusy( BOOL *isBusy, int dN)
{
	CY_RETURN_STATUS rStatus;
	rStatus = GpioRead( dN, CY_GPIO_INTF_NUM, I2C_BUSY, isBusy );
	if( rStatus != CY_SUCCESS )
	{
		puts("## GpioGetI2CBusy: GpioRead Fail ##");
		return rStatus;
	}
	*isBusy = !*isBusy;
	return rStatus;
}
*/
/* es
BOOL WaitForI2cIdle(int dN)
{
	int timeout = 0;
	BOOL isBusy = TRUE;
	CY_RETURN_STATUS rStatus;
	for( ; timeout < 2000 ; timeout += 50 )
	{
		usleep(50*1000);
		rStatus = GpioGetI2CBusy( &isBusy , dN);
		if ( !isBusy )
			return TRUE;
	}
	puts("ERROR: I2c busy timeout! Try the i2c cmd later.");
	return FALSE;
}
*/

CY_RETURN_STATUS GpioGetSysrdyBusy( BOOL *isBusy, int dN)
{
	CY_RETURN_STATUS rStatus;
	rStatus = GpioRead( dN, CY_GPIO_INTF_NUM, SYS_RDY, isBusy );  //GPIO6
	if( rStatus != CY_SUCCESS )
	{
		puts("## GpioGetSysrdyBusy: GpioRead Fail ##");
		return rStatus;
	}
	*isBusy = !*isBusy;
	return rStatus;
}

CY_RETURN_STATUS GpioGetSpirdyBusy( BOOL *isBusy, int dN)
{
	CY_RETURN_STATUS rStatus;
	rStatus = GpioRead( dN, CY_GPIO_INTF_NUM, SPI_RDY, isBusy );  //GPIO5
	if( rStatus != CY_SUCCESS )
	{
		puts("## GpioGetSpirdyBusy: GpioRead Fail ##");
		return rStatus;
	}
	*isBusy = !*isBusy;
	return rStatus;
}

/*********************I2C WRITE Interface*********************/

CY_RETURN_STATUS I2CGetPadID( byte *padid , int dN)
{
	CY_RETURN_STATUS rStatus;
	
	const int08 cWRITESIZE = 6;
	const int08 cREADSIZE = 1;
	
	uint08* sendBuf;
	uint08* sendBuf2;
	uint08* readBuf;
	
	sendBuf = (uint08*)malloc(cWRITESIZE);
	sendBuf2 = (uint08*)malloc(1);
	readBuf = (uint08*)malloc(cREADSIZE);
	
	//printf("%x\n",readBuf[0]);
	
	sendBuf[0] = WRITE_EXTERNAL_PAD_ADDRESS;
	sendBuf[1] = 0x00;
	sendBuf[2] = 0x00;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x01;//dataLength = 1;
	sendBuf[5] = 0x01;//0x01 = Read Mode; 0x00 = Write Mode
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf );
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2CGetPadID: I2CWrite Fail ##");
		return rStatus;
	}
	
	sendBuf2[0] = READ_EXTERNAL_PAD_DATA;
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, 1, sendBuf2 );
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2CGetPadID: I2CWrite Fail ##");
		return rStatus;
	}
	
	rStatus = I2CRead( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cREADSIZE, readBuf);
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2CGetPadID: I2CRead Fail ##");
		return rStatus;
	}
	
	//printf("%x\n",readBuf[0]);
	
	*padid = readBuf[0];
	free(sendBuf);
	free(sendBuf2);
	free(readBuf);
	
	
	
}
//////////1438////////
CY_RETURN_STATUS I2CSetCRC16OnOff( BOOL enable, int dN)
{
	CY_RETURN_STATUS rStatus;
	
	const int08 cWRITESIZE = 2;
	uint08* sendBuf;
	
	sendBuf = (UINT8 *)malloc(cWRITESIZE);
	
	sendBuf[0] = WRITE_FPGA_CONTROL;  //CAh
	if(enable == TRUE)
	{
		sendBuf[1] = 0x04;		
	}
	else
	{
		sendBuf[1] = 0x00;
	}
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf );
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2CSetCRC16OnOff: I2CWrite Fail ##");
		return rStatus;
	}
	return rStatus;
}

CY_RETURN_STATUS I2CSetActiveBuffer( BOOL activebuf, int dN)
{
	CY_RETURN_STATUS rStatus;
	
	const int08 cWRITESIZE = 2;
	uint08* sendBuf;
	
	sendBuf = (UINT8 *)malloc(cWRITESIZE);
	
	sendBuf[0] = WRITE_ACTIVE_BUFFER;   //C5h
	if(activebuf == TRUE)
	{
		sendBuf[1] = 0x01;		
	}
	else
	{
		sendBuf[1] = 0x00;
	}
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf );
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2CSetActiveBuffer: I2CWrite Fail ##");
		return rStatus;
	}
	return rStatus;
}

CY_RETURN_STATUS I2CSetExternalPrintConfiguration(int08 para1, int08 para2 , int dN)
{
	CY_RETURN_STATUS rStatus;
	
	const int08 cWRITESIZE = 3;
	uint08* sendBuf;
	
	sendBuf = (UINT8 *)malloc(cWRITESIZE);
	
	sendBuf[0] = WRITE_EXTERNAL_PRINT_CONFIGURATION;   //A8h
	sendBuf[1] = para1;
	sendBuf[2] = para2;
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf );
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2CSetExternalPrintConfiguration: I2CWrite Fail ##");
		return rStatus;
	}
	return rStatus;
}


CY_RETURN_STATUS I2CSetParallelBuffer(int08 para, int dN)
{
	CY_RETURN_STATUS rStatus;
	
	const int08 cWRITESIZE = 2;
	uint08* sendBuf;
	
	sendBuf = (UINT8 *)malloc(cWRITESIZE);
	
	sendBuf[0] = WRITE_PARALLEL_VIDEO;   //C3h
	sendBuf[1] = para;
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf );
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2CSetParallelBuffer: I2CWrite Fail ##");
		return rStatus;
	}
	return rStatus;
}

CY_RETURN_STATUS I2CSetExternalPrintControl(int08 para1, int08 para2, int08 para3, int08 para4, int08 para5, int dN)
{
	CY_RETURN_STATUS rStatus;
	
	const int08 cWRITESIZE = 6;
	uint08* sendBuf;
	
	sendBuf = (UINT8 *)malloc(cWRITESIZE);
	
	sendBuf[0] = WRITE_EXTERNAL_PRINT_CONTROL;   //C1h
	sendBuf[1] = para1;
	sendBuf[2] = para2;
	sendBuf[3] = para3;
	sendBuf[4] = para4;
	sendBuf[5] = para5;
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf );
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2CSetExternalPrintControl: I2CWrite Fail ##");
		return rStatus;
	}
	return rStatus;
}

////end of 1438 /////////////////////

CY_RETURN_STATUS I2CSetLedOnOff( BOOL enable, int dN)
{
	CY_RETURN_STATUS rStatus;
	
	const int08 cWRITESIZE = 2;
	uint08* sendBuf;
	
	
	/*if( !WaitForI2cIdle(dN) )
		return -1;*/
	
	
	sendBuf = (UINT8 *)malloc(cWRITESIZE);
	
	sendBuf[0] = WRITE_LED_ENABLE;
	if(enable == TRUE)
	{
		sendBuf[1] = 0x07;		
	}
	else
	{
		sendBuf[1] = 0x00;
	}
	
	
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf );
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2CSetLedOnOff: I2CWrite Fail ##");
		return rStatus;
	}
	return rStatus;
}


CY_RETURN_STATUS I2cSetLedDac( int16 leveld , int dN)
{
	CY_RETURN_STATUS rStatus;
	const int08 cWRITESIZE = 7;
	uint08* sendBuf;
	
	
	/*if( !WaitForI2cIdle(dN) )
		return -1;*/
	
	
	sendBuf = (UINT8 *)malloc(cWRITESIZE);
	
	sendBuf[0] = WRITE_LED_CURRENT;
	sendBuf[1] = leveld & 0xFF;
	sendBuf[2] = (leveld >> 8) & 0xFF;
	sendBuf[3] = leveld & 0xFF;
	sendBuf[4] = (leveld >> 8) & 0xFF;
	sendBuf[5] = leveld & 0xFF;
	sendBuf[6] = (leveld >> 8) & 0xFF;
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf);
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2cSetLedDac: I2CWrite Fail ##");
		free(sendBuf);
		return rStatus;
	}
	free(sendBuf);
	return rStatus;
}

CY_RETURN_STATUS I2cSetInputSource( int08 select, int dN )
{
	CY_RETURN_STATUS rStatus;
	const int08 cWRITESIZE = 2;
	uint08* sendBuf;
	
	
	/*if( !WaitForI2cIdle(dN) )
		return -1;*/
	
	
	sendBuf = (UINT8 *)malloc(cWRITESIZE);
	
	sendBuf[0] = WRITE_INPUT_SOURCE;
	sendBuf[1] = select;
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf);
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2cSetInputSource: I2CWrite Fail ##");
		free(sendBuf);
		return rStatus;
	}
	free(sendBuf);
	return rStatus;
}

CY_RETURN_STATUS I2cSetPattern( int08 pattern , int dN )
{
	CY_RETURN_STATUS rStatus;
	const int08 cWRITESIZE = 7;
	uint08* sendBuf;
	
	
	/*if( !WaitForI2cIdle(dN) )
		return -1;*/
	
	
	sendBuf = (UINT8 *)malloc(cWRITESIZE);
	
	sendBuf[0] = WRITE_TEST_PATTERNS;
	sendBuf[1] = pattern;
	if ((pattern & 0x7F) == 0x01) //Fixed Step Horizontal Ramp
	{
		sendBuf[2] = 0x70;
		sendBuf[3] = 0x00;
		sendBuf[4] = 0xff;
		sendBuf[5] = 0x00;
		sendBuf[6] = 0x00;
	}
	else if ((pattern & 0x7F) == 0x07) //Checkerboard
	{
		sendBuf[2] = 0x70;
		sendBuf[3] = 0x04;
		sendBuf[4] = 0x00;
		sendBuf[5] = 0x04;
		sendBuf[6] = 0x00;
	}
	else
	{
		sendBuf[2] = 0x70;
		sendBuf[3] = 0x00;
		sendBuf[4] = 0x00;
		sendBuf[5] = 0x00;
		sendBuf[6] = 0x00;
	}
	
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf);
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2cSetPattern: I2CWrite Fail ##");
		free(sendBuf);
		return rStatus;
	}
	free(sendBuf);
	return rStatus;
}

CY_RETURN_STATUS I2CSetFlip(int08 flip , int dN)
{
	CY_RETURN_STATUS rStatus;
	const int08 cWRITESIZE = 2;
	uint08* sendBuf;
	
	/*if( !WaitForI2cIdle(dN) )
		return -1;*/
		
	sendBuf = (UINT8 *)malloc(cWRITESIZE);
	
	sendBuf[0] = WRITE_PROJCETOR_FLIP;
	sendBuf[1] = flip;
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf);
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2cSetFlip: I2CWrite Fail ##");
		free(sendBuf);
		return rStatus;
	}
	free(sendBuf);
	return rStatus;
}

CY_RETURN_STATUS I2CGetLedOnOff(BOOL* ledR, BOOL* ledG, BOOL* ledB, int dN)
{
	CY_RETURN_STATUS rStatus;
	const int08 cWRITESIZE = 1;
	const int08 cREADSIZE = 1;
	uint08* sendBuf,* readBuf;
	
	/*if( !WaitForI2cIdle(dN) )
		return -1;*/
		
	sendBuf = (UINT8 *)malloc(cWRITESIZE);
	readBuf = (UINT8 *)malloc(cREADSIZE);
	
	
	//D(printf("%x\n",readBuf[0]);)
	
	
	*ledR = *ledG = *ledB = false;
	
	sendBuf[0] = READ_LED_ENABLE;
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf);
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2cGetLedOnOff: I2CWrite Fail ##");
		free(sendBuf);
		return rStatus;
	}
	
	rStatus = I2CRead( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cREADSIZE, readBuf);
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2CGetLedOnOff: I2CRead Fail ##");
		return rStatus;
	}
	
	//D(printf("%x\n",readBuf[0]);)
	
	if((readBuf[0] & 0x01) == 0x01)
	{
		*ledR = true;
	}
	else
	{
		*ledR = false;
	}
	if((readBuf[0] & 0x02) == 0x02)
	{
		*ledG = true;
	}
	else
	{
		*ledG = false;
	}
	if((readBuf[0] & 0x04) == 0x04)
	{
		*ledB = true;
	}
	else
	{
		*ledB = false;
	}
	
	free(sendBuf);
	free(readBuf);
	return rStatus;
}

/*********************I2C READ Interface*********************/

CY_RETURN_STATUS I2cGetLedDac( int16 *level_R, int16 *level_G, int16 *level_B, int dN )
{
	CY_RETURN_STATUS rStatus;
	const int08 cWRITESIZE = 1;
	const int08 cREADSIZE  = 6;
	uint08* sendBuf, *readBuf;
	
	/*if( !WaitForI2cIdle(dN) )
		return -1;*/
	
	sendBuf = (uint08 *)malloc(cWRITESIZE);
	readBuf = (uint08 *)malloc(cREADSIZE);
	
	sendBuf[0] = READ_LED_CURRENT;
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf);
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2cGetLedDac: I2CWrite Fail ##");
		free(sendBuf);
		free(readBuf);
		return rStatus;
	}
	
	usleep(WRITE_RESTART_READ_DELAY);
	
	rStatus = I2CRead( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cREADSIZE, readBuf );
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2cGetLedDac: I2CRead Fail ##");
		free(sendBuf);
		free(readBuf);
		return rStatus;
	}
 
	*level_R = (readBuf[1] << 8) | readBuf[0];
	*level_G = (readBuf[3] << 8) | readBuf[2];
	*level_B = (readBuf[5] << 8) | readBuf[4];	
	
	free(sendBuf);
	free(readBuf);
	return rStatus;
}



CY_RETURN_STATUS I2cGetAppVersion( char *appVer , int dN)
{
	CY_RETURN_STATUS rStatus;
	const int08 cWRITESIZE = 1;
	const int08 cREADSIZE  = 4;
	uint08* sendBuf, *readBuf;
	
	/*if( !WaitForI2cIdle(dN) )
		return -1;*/
	
	sendBuf = (uint08 *)malloc(cWRITESIZE);
	readBuf = (uint08 *)malloc(cREADSIZE);
	
	sendBuf[0] = READ_FB_VER;
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf);
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2cGetAppVersion: I2CWrite Fail ##");
		free(sendBuf);
		free(readBuf);
		return rStatus;
	}
	
	usleep(WRITE_RESTART_READ_DELAY);
	
	rStatus = I2CRead( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cREADSIZE, readBuf );
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2cGetAppVersion: I2CRead Fail ##");
		free(sendBuf);
		free(readBuf);
		return rStatus;
	}
	
	sprintf( appVer, "%d.%d.%d", readBuf[3], readBuf[2], (readBuf[1] << 8 | readBuf[0]) );

	free(sendBuf);
	free(readBuf);
	return rStatus;
}

CY_RETURN_STATUS I2cGetLedTemperature( float *temperature , int dN )
{
	CY_RETURN_STATUS rStatus;
	const int08 cWRITESIZE = 1;
	const int08 cREADSIZE  = 2;
	uint08* sendBuf, *readBuf;
	
	/*if( !WaitForI2cIdle(dN) )
		return -1;*/
	
	sendBuf = (uint08 *)malloc(cWRITESIZE);
	readBuf = (uint08 *)malloc(cREADSIZE);
	
	sendBuf[0] = READ_TEMPERATRUE;
	
	rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cWRITESIZE, sendBuf);
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2cGetLedTemperature: I2CWrite Fail ##");
		free(sendBuf);
		free(readBuf);
		return rStatus;
	}
	
	usleep(WRITE_RESTART_READ_DELAY);
	
	rStatus = I2CRead( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cREADSIZE, readBuf );
	if( rStatus != CY_SUCCESS )
	{
		puts("## I2cGetLedTemperature: I2CRead Fail ##");
		free(sendBuf);
		free(readBuf);
		return rStatus;
	}
	
	
	*temperature = (readBuf[1] << 8) | readBuf[0]; 

	free(sendBuf);
	free(readBuf);
	return rStatus;
}

float tmp75Conversion(int temperature)
{
	if ((temperature & 0x0800) > 0) // negative
	{
		temperature = 0x1000 - temperature;
		return 0 - (((temperature & 0x0FF0) >> 4) + ((float)(temperature & 0x0F) / 16));
	}
	else
	{
		return ((temperature & 0x0FF0) >> 4) + ((float)(temperature & 0x0F) / 16);
	}
}

CY_RETURN_STATUS I2CPwrUpDwnLight(int sel, int type, int dN)
{
	usleep(500*1000);
	CY_RETURN_STATUS rStatus;
	const int08 cWRITESIZE = 2;
	uint08* sendBuf;
	sendBuf = (uint08 *)malloc(cWRITESIZE);
	
	if(type == 0)
	{
		sendBuf[0] = WRITE_PWR;
		if(sel == 1)
		{
			sendBuf[1] = 0x03;
		}
		else
		{
			sendBuf[1] = 0x00;
		}
		rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, NEWLIGHTSENSOR, cWRITESIZE, sendBuf);
		if( rStatus != CY_SUCCESS )
		{
			puts("## I2CPwrUpDwnLight(NEW): I2CWrite Fail ##");
		    free(sendBuf);
			return rStatus;
		}
		if(sel == 1)
		{
			usleep(720*1000);
		}
	}
	else
	{
		sendBuf[0] = WRITE_PWR;
		if(sel == 1)
		{
			sendBuf[1] = 0x03;
		}
		else
		{
			sendBuf[1] = 0x00;
		}
		rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, OLDLIGHTSENSOR, cWRITESIZE, sendBuf);
		if( rStatus != CY_SUCCESS )
		{
			puts("## I2CPwrUpDwnLight(OLD): I2CWrite Fail ##");
		    free(sendBuf);
			return rStatus;
		}
		if(sel == 1)
		{
			usleep(500*1000);
		}
	}
	
	free(sendBuf);
	return rStatus;
	
}

CY_RETURN_STATUS I2CGetLight( int16 **light, int type , int dN)
{
	
	usleep(500*1000);
	CY_RETURN_STATUS rStatus;
	const int08 cWRITESIZE = 1;
	const int08 cREADSIZE  = 2;
	uint08* sendBuf, *readBuf;
	sendBuf = (uint08 *)malloc(cWRITESIZE);
	readBuf = (uint08 *)malloc(cREADSIZE);
	
	**light = 0;
	
	rStatus = I2CPwrUpDwnLight(1, type, dN);
	if(rStatus != 0)
	{
		return rStatus;
	}
	if(type == 0)
	{
		sendBuf[0]=READ_UVA_DATA | COMMAND_REGISTER;
		rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, NEWLIGHTSENSOR, cWRITESIZE, sendBuf);
		if( rStatus != CY_SUCCESS )
		{
			puts("## I2CGetLight: I2CWrite Fail ##");
		    free(sendBuf);
			free(readBuf);
			return rStatus;
		}
		
		
		
		rStatus = I2CRead( dN, CY_I2C_INTF_NUM, NEWLIGHTSENSOR, cREADSIZE, readBuf );
		if( rStatus != CY_SUCCESS )
		{
			puts("## I2CGetLight: I2CRead Fail ##");
			free(sendBuf);
			free(readBuf);
			return rStatus;
		}
	}
	else
	{
		sendBuf[0] = 0xac;
		rStatus = I2CWrite( dN, CY_I2C_INTF_NUM, OLDLIGHTSENSOR, cWRITESIZE, sendBuf);
		if( rStatus != CY_SUCCESS )
		{
			puts("## I2CGetLight: I2CWrite Fail ##");
		    free(sendBuf);
			free(readBuf);
			return rStatus;
		}
		
		rStatus = I2CRead( dN, CY_I2C_INTF_NUM, OLDLIGHTSENSOR, cREADSIZE, readBuf );
		if( rStatus != CY_SUCCESS )
		{
			puts("## I2CGetLight: I2CRead Fail ##");
			free(sendBuf);
			free(readBuf);
			return rStatus;
		}
	}
	
	**light = (readBuf[1] << 8) | readBuf[0];
	
	rStatus = I2CPwrUpDwnLight(0, type, dN);
	if(rStatus != 0)
	{
		return -1;
	}
	
	return rStatus;
}

CY_RETURN_STATUS I2cGetLightSensor( int16 *light, int **type , int dN)
{
	/*
	CY_RETURN_STATUS rStatus;
	
	**type = 5;
	
	*light = 100;
	
	return (CY_RETURN_STATUS)0;
	*/
	
	
	CY_RETURN_STATUS rStatus;
	rStatus = I2CGetLight(&light, 0, dN);
	printf("rStatus is : %d\n",-(CY_RETURN_STATUS)rStatus);
	if((CY_RETURN_STATUS)rStatus == (CY_RETURN_STATUS)CY_ERROR_I2C_NAK_ERROR)
	{
		printf("In -17\n");
		rStatus = I2CGetLight(&light, 1, dN);
		if(rStatus != 0)
		{
			return -1;
		}
		else
		{
			**type = 1;
			return (CY_RETURN_STATUS)CY_SUCCESS;
		}
	}
	else if(rStatus != 0)
	{
		printf("In -17 else if\n ");
		return -1;
	}
	else
	{
		printf("In -17 else\n");
		**type = 0;		
	}
	
	return (CY_RETURN_STATUS)CY_SUCCESS;
	
}


CY_RETURN_STATUS I2CGetFlip( byte *status , int dN)
{
	CY_RETURN_STATUS rStatus;
	const int08 cWRITESIZE = 1;
	const int08 cREADSIZE = 1;
	uint08 *sendBuf,*readBuf;
	
	sendBuf = (uint08*)malloc(cWRITESIZE);
	readBuf = (uint08*)malloc(cREADSIZE);
	
	sendBuf[0] = READ_PROJCETOR_FLIP;
	
	rStatus = I2CWrite(dN,CY_I2C_INTF_NUM,DDP1438_I2C_ADDR_7BIT,cWRITESIZE,sendBuf);
	if( rStatus != CY_SUCCESS )
		{
			puts("## I2CGetFlip: I2CWrite Fail ##");
		    free(sendBuf);
			free(readBuf);
			return rStatus;
		}
		
		rStatus = I2CRead( dN, CY_I2C_INTF_NUM, DDP1438_I2C_ADDR_7BIT, cREADSIZE, readBuf );
		if( rStatus != CY_SUCCESS )
		{
			puts("## I2CGetFlip: I2CRead Fail ##");
			free(sendBuf);
			free(readBuf);
			return rStatus;
		}
		
		*status = readBuf[0];
		free(sendBuf);
		free(readBuf);
		
		return rStatus;
	
}


/*********************SPI Read Interface*********************/

CY_RETURN_STATUS cySPIWaitForIdle(CY_HANDLE cyHandle)
{
	char rd_data[2], wr_data[2];
	CY_DATA_BUFFER writeBuf, readBuf;
	int timeout = 0xFFFF;
	CY_RETURN_STATUS status;

	writeBuf.length = 2;
	writeBuf.buffer = (unsigned char *)wr_data;

	readBuf.length = 2;
	readBuf.buffer = (unsigned char *)rd_data;

	// Loop here till read data indicates SPI status is not idle
	// Condition to be checked: rd_data[1] & 0x01

	do
	{
		wr_data[0] = READ_STATUS1; /* Get SPI status */
		status = CySpiReadWrite(cyHandle, &readBuf, &writeBuf, 5000);

		if (status != CY_SUCCESS)
		{
			printf("\nFailed to send SPI status query command to device.");
			break;
		}
		timeout--;
		if (timeout == 0)
		{
			printf("\nMaximum retries completed while checking SPI status, returning with error code.");
			status = CY_ERROR_IO_TIMEOUT;
			return status;
		}

	} while (rd_data[1] & 0x03); //Check SPI Status

	return status;
}

CY_RETURN_STATUS FlashBlockRead(int deviceNumber, int interfaceNum, int address, int len, char *buf)
{
	int BlockSize = 0x1000; //Block read 4KB size
	unsigned char wbuffer[0x1004], rbuffer[0x1004];  //array size is equal to BlockSize+4
	CY_HANDLE cyHandle;
	CY_DATA_BUFFER cyDatabufferWrite, cyDatabufferRead;
	CY_RETURN_STATUS rStatus;
	int cWriteSize, cReadSize;
	int str_addr = address;

	rStatus = CyOpen(deviceNumber, interfaceNum, &cyHandle);

	if (rStatus != CY_SUCCESS){
		D(printf("SPI Device open failed.\n"));
		return rStatus;
	}

	while (len > 0)
	{
		wbuffer[0] = READ_DATA;
		wbuffer[1] = (address >> 16 & 0xff);
		wbuffer[2] = (address >> 8 & 0xff);
		wbuffer[3] = (address & 0xff);

		cWriteSize = 4;
		if (len > BlockSize)
			cReadSize = BlockSize;
		else
			cReadSize = len;

		//SPI uses a single CySpiReadWrite to perform both read and write
		//and flush operations.
		cyDatabufferWrite.buffer = wbuffer;
		cyDatabufferWrite.length = 4 + cReadSize; //4 bytes command + 256 bytes page size

		cyDatabufferRead.buffer = rbuffer;
		cyDatabufferRead.length = 4 + cReadSize;
		// As per the EEPROM datasheet we need to perform simeltanious read and write
		// to do read/write operation on EEPROM.
		// In this case cyDatabufferRead contains data pushed out by EEPROM and not real data.
		rStatus = CySpiReadWrite(cyHandle, &cyDatabufferRead, &cyDatabufferWrite, 5000);
		if (rStatus != CY_SUCCESS)
		{
			printf("Error in doing SPI data write :0x%X \n", cyDatabufferWrite.transferCount);
			CyClose(cyHandle);
			return rStatus;
		}

		int i;
		for (i = 0; i < cReadSize; i++)
			buf[address - str_addr + i] = rbuffer[cWriteSize + i];

		rStatus = cySPIWaitForIdle(cyHandle);
		if (rStatus)
		{
			printf("Error in Waiting for Flash active state:0x%X \n", rStatus);
			CyClose(cyHandle);
			return rStatus;
		}

		address += BlockSize;
		if (len > BlockSize)
			len -= BlockSize;
		else
			len = 0;
	}

	CyClose(cyHandle);
	return CY_SUCCESS;
}

CY_RETURN_STATUS SpiGetSerialNumber( char* sn, int08* len , int dN)
{
	CY_RETURN_STATUS rStatus;
	int08 index = 0;
	char *buffer = NULL;
	buffer = (char *)malloc(128);
	
	rStatus = FlashBlockRead( dN, CY_SPI_INTF_NUM, ADDR_SNUM, 128, buffer );
	if (rStatus != CY_SUCCESS)
	{
		printf("Failed to Read parameters from light engine !\n");
		return rStatus;
	}
	
	while( buffer[index] != 0 && index < 19 )
	{
		sn[index] = buffer[index];
		index ++;
	}

	sn[index] = '\0';
	*len = index+1;
	//printf("index is %d\n",index);
	return CY_SUCCESS;
}

CY_RETURN_STATUS SpiGetMask(MASK_TYPE type, char* filename, int dN)
{
	CY_RETURN_STATUS rStatus;
	int32 index = 0;
	byte header_buf[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //4bytes file length and 4bytes file check sum
	UINT32 i;
	UINT32 png_len = 0;
	UINT32 png_cksum = 0;
	int addr = 0;
	char *png_buf = NULL;
	
	if( type == NATIVE )
		addr = ADDR_NATIVE_MASK;
	else if( type == FOUR_K )
		addr = ADDR_4K_MASK;
	
	rStatus = FlashBlockRead( dN, CY_SPI_INTF_NUM, addr, 8, header_buf );
	if (rStatus != CY_SUCCESS)
	{
		printf("Failed to Read Uniformity Mask Check Sum and Size.!\n");
		return rStatus;
	}
	
	png_len = header_buf[0] & 0xff | header_buf[1] << 8 & 0xff00 | header_buf[2] << 16 & 0xff0000 | header_buf[3] << 24 & 0xff000000;
	if( png_len == 0xFFFFFFFF )
	{
		printf("Mask file maybe broken.\n");
		return -1;
	}
	png_cksum = header_buf[4] & 0xff | header_buf[5] << 8 & 0xff00 | header_buf[6] << 16 & 0xff0000 | header_buf[7] << 24 & 0xff000000;
	
	png_buf = (char*)malloc(png_len);
	rStatus = FlashBlockRead(dN, CY_SPI_INTF_NUM, addr + 8, png_len, png_buf);
	if (rStatus != CY_SUCCESS)
	{
		printf("Failed to Read Uniformity Mask.\n");
		return rStatus;
	}
	UINT32 cksum = 0;
	for ( i = 0; i < png_len; i++)
	{
		cksum += png_buf[i] & 0xff;
	}
	if (cksum==0 || cksum != png_cksum)
	{
		printf("Uniformity Mask File check sum is NG.\n");
		return -1;
	}

	FILE *pFile;
	pFile = fopen( filename, "wb");
	
	if (NULL == pFile)
	{
		printf("file open failure!\n");
		return -1;
	}
	else
	{
		fwrite(png_buf, 1, png_len, pFile);
	}
	fclose(pFile);

	free(png_buf);
	return CY_SUCCESS;

}
