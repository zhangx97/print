/****************************************************************************/
/* NPRFLightEngine.h                                                         */
/*                                                                          */
/* Define the LightEngine function.					                        */
/****************************************************************************/
#ifndef _NPRFLIGHTENGINE_H
#define _NPRFLIGHTENGINE_H

#include "common.h"
#include "CyUSBSerial.h"
#include "Ddp1438.h"
#include "yo_spi.h"

BOOL ProjectorOnOff				   	  ( BOOL enable , int dN);
BOOL Print                            ( int16 frames , int dN);
int yo_spidatastream_write            ( int08 imageselect , int dN);
BOOL LedOnOff					   	  ( BOOL enable , int dN);
BOOL SetLedDac					   	  ( int16 leveld , int dN);
BOOL SetSource					   	  ( INPUT_SOURCE source , int dN);
BOOL SetDmdFlip					   	  ( FLIP_ORIENTATION orientation, int dN );
int16 GetLedDac					   	  ( int dN );
BOOL GetLedOnOff                      ( int dN );
BOOL GpioLedOnOff                  ( BOOL enable , int dN);					
char* GetAppVer					   	  ( int dN );

float GetLedTemperature			   	  ( int dN );

int16 GetLightSensor			   	  ( int* type , int dN);
FLIP_ORIENTATION GetDmdFlipOrientation( int dN );
char* GetSerialNumber				  ( int dN );
void GetUniformityMask				  ( int dN );
//New Add
//void CheckI2CBusy                     ( int dN );
void Check_SYS_RDY_Busy               ( int dN );
void Check_SPI_RDY_Busy               ( int dN );
//New Add
byte GetPad300xChipId                 ( int dN );
#endif
