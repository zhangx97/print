/****************************************************************************/
/* Ddp1438.h                                                                */
/*                                                                          */
/* Define Ddp1438 GPIO/I2C/SPI function.                                    */
/****************************************************************************/

#ifndef _DDP1438_H
#define _DDP1438_H

#define DDP1438_I2C_ADDR_7BIT	0x1B
#define NEWLIGHTSENSOR			0x29
#define OLDLIGHTSENSOR    		0x39

#include "CyUSBSerial.h"
#include "common.h"

enum CommandId {
	WRITE_LED_ENABLE            = 0x52,
	WRITE_LED_CURRENT           = 0x54,
	LED_CURRENT_DAC             = 0xD1,         //Control the Current of LED
	TEMPERATURE_SENSOR          = 0x9C,         //Read DMD Temperature
	WRITE_INPUT_SOURCE          = 0x05,
	READ_INPUT_SOURCE           = 0x06,
	WRITE_TEST_PATTERNS         = 0x0b,
	READ_LED_CURRENT            = 0x55,
	READ_FB_VER                 = 0xd9,
	READ_TEMPERATRUE            = 0xd6,
	WRITE_EXTERNAL_PAD_ADDRESS  = 0xeb,
	READ_EXTERNAL_PAD_DATA      = 0xed,
	READ_UVA_DATA               = 0x14,
	WRITE_PROJCETOR_FLIP        = 0x14,
	READ_PROJCETOR_FLIP			= 0x15,
	READ_LED_ENABLE             = 0x53,
	WRITE_PWR                   = 0xa0,
	COMMAND_REGISTER            = 0xA0,
	READ_LIGHT                  = 0xac,
	
	            //3D Print Commands
            WRITE_TRIGGER_OUT_CONFIGURATION = 0x92,
            READ_TRIGGER_OUT_CONFIGURATION = 0x93,
            READ_LIGHT_CONTROL_SEQUENCE_VERSION = 0x9B,
            WRITE_EXTERNAL_PRINT_CONFIGURATION = 0xA8,
            READ_EXTERNAL_PRINT_CONFIGURATION = 0xA9,
            WRITE_EXTERNAL_PRINT_CONTROL = 0xC1,
            READ_EXTERNAL_PRINT_CONTROL = 0xC2,
            WRITE_PARALLEL_VIDEO = 0xC3,
            READ_PARALLEL_VIDEO = 0xC4,
            WRITE_ACTIVE_BUFFER = 0xC5,
            READ_ACTIVE_BUFFER = 0xC6,
            WRITE_FPGA_CONTROL = 0xCA,
            READ_FPGA_CONTROL = 0xCB,
            WRITE_FPGA_TGP = 0x67,
            //3D Print Commands

};

enum CY_GPIO_NUM {
	PROJ_ON		 =  7,  ///2
	///I2C_BUSY	 =  1,
	LED_SW		 =  1,  ///14 
	ASIC_READY	 = 15,  ///Host IRQ
    SPI_RDY      =  5,
	SYS_RDY      =  6,
};

enum Flash_CommandCode
{
	READ_STATUS1 = 0x05,
	READ_STATUS2 = 0x35,
	READ_DATA 	 = 0x03,
};

enum Flash_ADDR
{
	ADDR_SNUM			 = 0x00,
	ADDR_NATIVE_MASK	 = 0x10000,
	ADDR_4K_MASK		 = 0xA0000,
	
};

typedef enum _MASK_TYPE
{
	NATIVE,
	FOUR_K,
} MASK_TYPE;

typedef enum _INPUT_SOURCE {
	HDMI_EXTERNAL = 0,
	SOLID_FIELD,
	CHECKER_BOARD,
	RAMP,
} INPUT_SOURCE;

typedef enum _FLIP_ORIENTATION {
	H,
	V,
	HV,
	NO_FLIP,
} FLIP_ORIENTATION;

float 			 tmp75Conversion				( int temperature );
///BOOL 			 WaitForI2cIdle					( int dN);
CY_RETURN_STATUS GpioSetProjectorOnOff			( BOOL enable , int dN);
CY_RETURN_STATUS GpioGetAsicReady				( BOOL *isReady , int dN);
///CY_RETURN_STATUS GpioGetI2CBusy					( BOOL *isBusy , int dN);
CY_RETURN_STATUS GpioGetSysrdyBusy				( BOOL *isBusy , int dN);
CY_RETURN_STATUS GpioGetSpirdyBusy				( BOOL *isBusy , int dN);
CY_RETURN_STATUS I2CGetPadID                    ( byte *padid , int dN);

CY_RETURN_STATUS I2CSetCRC16OnOff		        ( BOOL enable , int dN);
CY_RETURN_STATUS I2CSetActiveBuffer		        ( BOOL activebuf , int dN);
CY_RETURN_STATUS I2CSetExternalPrintConfiguration   ( int08 para1, int08 para2 , int dN);
CY_RETURN_STATUS I2CSetParallelBuffer           ( int08 para, int dN);
CY_RETURN_STATUS I2CSetExternalPrintControl     ( int08 para1, int08 para2, int08 para3, int08 para4, int08 para5, int dN);

CY_RETURN_STATUS I2CSetLedOnOff				    ( BOOL enable , int dN);
CY_RETURN_STATUS GpioSetLedOnOff                ( BOOL enable , int dN);
CY_RETURN_STATUS I2cSetLedDac					( int16 leveld , int dN);
CY_RETURN_STATUS I2cSetInputSource				( int08 select , int dN);
CY_RETURN_STATUS I2cSetPattern					( int08 pattern , int dN);
CY_RETURN_STATUS I2CSetFlip                     (int08 flip, int dN);

CY_RETURN_STATUS I2cGetLedDac					( int16 *level_R, int16 *level_G, int16 *level_B, int dN );
CY_RETURN_STATUS I2CGetLedOnOff                 (BOOL* ledR, BOOL* ledG, BOOL* ledB, int dN);


CY_RETURN_STATUS I2cGetAppVersion				( char *appVer , int dN);
CY_RETURN_STATUS I2cGetLedTemperature			( float *temperature , int dN);

CY_RETURN_STATUS I2CGetLight                    ( int16 **light, int type, int dN);
CY_RETURN_STATUS I2cGetLightSensor				( int16 *light, int **type , int dN);

CY_RETURN_STATUS I2CGetFlip						( byte *status , int dN);


CY_RETURN_STATUS SpiGetSerialNumber				( char *sn, int08* len , int dN);
CY_RETURN_STATUS SpiGetMask						( MASK_TYPE type, char* filename, int dN);

#endif
