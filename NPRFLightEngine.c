/****************************************************************************/
/* NPRFLightEngine.c                                                         */
/*                                                                          */
/****************************************************************************/

#include "NPRFLightEngine.h"
#include "Ddp1438.h"
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "yo_spi.h"

#define I2C_DELAY_US		1000*500

struct data_stream {
    unsigned commandop : 8;
    unsigned col_start_index : 5;
    unsigned col_end_index : 5;
    unsigned row_start_index : 11;
    unsigned rdummy : 7;             //all 0
    unsigned datacheck : 4;          //all 1
    unsigned dummy : 8;              // 1000 0000
};


static spi* spi_dev0;
static int SPIWriteData(int data_len, char *data);

BOOL ProjectorOnOff( BOOL enable , int dN )
{
	int08 ret = 0;
	ret = GpioSetProjectorOnOff ( enable , dN);
	if( ret != 0 )
	{
		puts("Projector ON/OFF Fail");
		return FALSE;
	}
	if(enable == TRUE)
		usleep(600000);
		puts("Asic Ready!");
	if(enable == FALSE)
		puts("Projector OFF OK!");
	return TRUE;
}

byte GetPad300xChipId( int dN )
{
	int08 ret = 0;
	byte PadId = 0;
	
	ret = I2CGetPadID(&PadId , dN);
	if(ret!=0)
		return -1;
	
	return PadId;
}

BOOL Print(int16 frames , int dN)
{
	int08 ret = 0;
	BOOL flagbuf = FALSE;
	int08 icycle = 0;
	uint16 counter =0;
	uint08 framesLow =0;
	uint08 framesHigh =0;
	uint32 curingtime =0;
	
	spi_dev0 = yo_spi_init(0, 0);
    spi* spi_dev;
    spi_dev = spi_dev0;
	yo_spi_set_mode(spi_dev, 0);
	yo_spi_set_bits_per_word(spi_dev, 8);
    yo_spi_set_speed(spi_dev, 48000000); //48MHz
	
	///step 1
	ret = I2CSetCRC16OnOff( TRUE , dN);
	if( ret !=0 )
		return FALSE;

    ///step 2
		ret = I2CSetActiveBuffer( flagbuf , dN);  //set active buffer 0(Odd Print Layer)
		flagbuf = !flagbuf;
		if( ret !=0 )
			return FALSE;

		usleep(1000);
	///step 3-1
		ret = I2cSetInputSource( 0xFF , dN);  //set standby mode
		if( ret !=0 )
			return FALSE;
	
		usleep(500000);  //500ms
    ///step 3-2
		ret = I2CSetExternalPrintConfiguration( 0x00, 0x04 , dN);  //set LED3 enable
		if( ret !=0 )
			return FALSE;
	
		usleep(5000);  //5ms
	///step 4
	    ++icycle;
		ret = yo_spidatastream_write( icycle , dN);  //SPI data stream transmission, send the data and waiting for the SPI_RDY pull high
		if( ret !=0 )
			return FALSE;

		if (icycle > 3) icycle = 0;
		
		Check_SPI_RDY_Busy(dN);   //waiting for the SPI_RDY get ready(high)

	///step 5
		ret = I2CSetActiveBuffer( flagbuf , dN);   //change active buffer to another buffer(Even print layer)
		flagbuf = !flagbuf;
		if( ret !=0 )
			return FALSE;
		
		usleep(5000);  //5ms
    ///step 6
		ret = I2CSetParallelBuffer( 0x01 , dN);  //read and send buffer
		if( ret !=0 )
			return FALSE;	
	
		usleep(5000);  //5ms
	///step 7
		ret = I2cSetInputSource( 0x06 , dN);  //set External Print mode, and waiting for the SYS_RDY pull high
		if( ret !=0 )
			return FALSE;

		usleep(5000);  //5ms
	    Check_SYS_RDY_Busy(dN);  //waiting for the SYS_RDY get ready(high)

	    framesLow = (uint08)(frames & 0xff);
		framesHigh = (uint08)((frames >>8) & 0xff);
		if((frames / 60.0) > 0.9)  //0.9 is spi image data transmission time, it's changed depand on SPI speed setting 
		{
		    curingtime = (uint32)(((frames / 60.0)-0.9)* 1000000.0);	
		}
		else
		{
			curingtime = (uint32)((0.9-(frames / 60.0))* 1000000.0);	
		}

	while(TRUE){	
		
		puts(" ");
		++counter;
		printf("## 3D Print - Layer : %d \n",counter);
		printf("The curing time is  %.2f s.(%d/60 s)\n ",frames/60.0, frames);
		
	///step 8 or step 11
		ret = I2CSetExternalPrintControl( 0x00, 0x05, 0x00, framesLow, framesHigh, dN);  //set External Print Layer Control and start print
		if( ret !=0 )
			return FALSE;
	
        if (counter > 9)  //just print 10 layers , and quit for the 3D printig
		{
			usleep((uint32)((frames / 60.0)* 1000000.0));
			ret = I2cSetInputSource( 0xFF , dN);  //set standby mode
			if( ret !=0 )
				return FALSE;			
			return TRUE;
		}	
	
	///step 9
	    ++icycle;
		ret = yo_spidatastream_write( icycle , dN);  //SPI data stream transmission
		if( ret !=0 )
			return FALSE;

		if (icycle > 3) icycle = 0;
    
    /////!!!!!!!  Waiting for the end of curing time  !!!!!!/// 
	    if((frames / 60.0) > 0.9)   //when curing time is larger than SPI image data transmission time 
     	   usleep(curingtime);
        else 
		  Check_SPI_RDY_Busy(dN);   //waiting for the SPI_RDY get ready(high)
	  
	    usleep(3*1000000);  //off time(this unit is micro second) setting between layer to layer 
	  
    ///step 10
		ret = I2CSetActiveBuffer( flagbuf , dN);  //set active buffer (change between Odd and Even layer)
		flagbuf = !flagbuf;
		if( ret !=0 )
			return FALSE;

		usleep(1000);

	} //while loop

	return TRUE;
}

int yo_spidatastream_write(int08 imageselect , int dN)
{
    //1440P 2560x1440=3687478, bmp file include header(1078bytes) and image data(3686400).
    int sdata_size;
    unsigned char *pStream_data;
    FILE *fp;
    ///int idx;
	
	if (imageselect == 0x01){
	   fp = fopen("pattern/white.bmp", "rb");
	   puts("## White Image ##");
	}
	else if (imageselect == 0x02){
	   fp = fopen("pattern/gray128.bmp", "rb");
	   puts("## Gray-128 Image ##");
	}
	else if (imageselect == 0x03){
	   fp = fopen("pattern/black.bmp", "rb");
	   puts("## Black Image ##");
	}
	else{
	   fp = fopen("pattern/young.bmp", "rb");
	   puts("## YoungOptics Image ##");
	}
	
    if (fp == NULL) {
        return -1;
    }
    else {   //write data stream to fpga
        sdata_size = 3687478; //(int *)sdata;
        pStream_data = (unsigned char *)malloc((unsigned long)sdata_size);
        if (pStream_data == NULL) {
            fclose(fp);
			free(pStream_data);
            return -1;
        }
        else {
            fread(pStream_data, 1, (unsigned long)sdata_size, fp);

            if ((SPIWriteData(sdata_size, (char *)pStream_data)) != 0) {
                 puts("SPIWriteData fail");
				 free(pStream_data);
                 return 1;
            }
            fclose(fp);
        }
    }
  free(pStream_data);
  return 0;
}

static int SPIWriteData(int data_len, char *data)
{
    int buffer_size = 1600; //1600
    int rStatus;
    unsigned char wbuffer[buffer_size+14];
    int i;
    int buffer_tmp =0;
    int bmpoffset = 1078; //bmp file header 1078; from 0x436
    struct data_stream ds;

    spi* spi_dev;
    spi_dev = spi_dev0;

    ds.commandop = 0x04;
    ds.col_start_index = 0;  //bit 12~8 : 00000
    ds.col_end_index = 19;    //bit 17~13 : 10011
    ds.row_start_index = 0; //bit 28~18 ;
    ds.rdummy = 0b0000000;//bit 35~29 ;
    ds.datacheck = 0b1111; //bit 39~36

    ds.dummy = 0x00;

    wbuffer[0] = ds.commandop;
    wbuffer[1] = (ds.col_end_index << 5) | (ds.col_start_index); // 3 + 5
    wbuffer[2] = (ds.row_start_index << 2) | (ds.col_end_index >> 3) ; // 6 + 2
    wbuffer[3] = ((ds.row_start_index >> 3) & 0xf1) | ((ds.rdummy) & 0x07); // 5 + 3
    wbuffer[4] =  (ds.datacheck << 4) | (ds.rdummy >> 3); // 4 + 4
    //wbuffer[1] = 0x60;    //wbuffer[2] = 0x02;    //wbuffer[3] = 0x00;    //wbuffer[4] = 0xF1;

    wbuffer[5] = ds.dummy;

    wbuffer[6] = (data_len - bmpoffset) & 0xff;     // little endian --> low first
    wbuffer[7] = (data_len - bmpoffset)>>8 & 0xff;
    wbuffer[8] = (data_len - bmpoffset)>>16 & 0xff;
    wbuffer[9] = (data_len - bmpoffset)>>24 & 0xff;

    for(buffer_tmp=0;buffer_tmp<(data_len - bmpoffset);buffer_tmp += buffer_size)
    {
        if(buffer_tmp == 0)
        {
			memcpy(wbuffer+10, data+bmpoffset, buffer_size);

            if(buffer_size < (data_len - bmpoffset))
            {
                 rStatus = yo_spi_write(spi_dev, wbuffer, (buffer_size+10));
                 if (rStatus != 0){
					 puts("Error in doing SPI data program!");
                     return rStatus;
                 }
            }
            else
            {
                wbuffer[buffer_size+10] = 0x12 ; //crc16[0];
                wbuffer[buffer_size+11] = 0x34 ; //crc16[1];
                wbuffer[buffer_size+12] = 0x00 ; //dummy byte
                wbuffer[buffer_size+13] = 0x00 ; //dummy byte

                rStatus = yo_spi_write(spi_dev, wbuffer, (buffer_size+14));
                if (rStatus != 0){
					puts("Error in doing SPI data program!");
                    return rStatus;
                }
            }
        }
        else if(buffer_tmp < (data_len-bmpoffset-buffer_size))
        {
           memcpy(wbuffer+6, data+bmpoffset+buffer_tmp, buffer_size);

           rStatus = yo_spi_write(spi_dev, wbuffer, (buffer_size+6));
           if (rStatus != 0){
			   puts("Error in doing SPI data program!");
               return rStatus;
           }

        }
        else //when last buffer_size
        {
			buffer_size = data_len - bmpoffset - buffer_tmp;
            memcpy(wbuffer+6, data+bmpoffset+buffer_tmp,buffer_size);

            wbuffer[buffer_size+6] = 0x12 ; //crc16[0];
            wbuffer[buffer_size+7] = 0x34 ; //crc16[1];
            wbuffer[buffer_size+8] = 0x00 ; //dummy byte
            wbuffer[buffer_size+9] = 0x00 ; //dummy byte

            rStatus = yo_spi_write(spi_dev, wbuffer, (buffer_size+10));
            if (rStatus != 0){
                puts("Error in doing SPI data program!");
                return rStatus;
            }
        }
    }

    return 0;
}


BOOL LedOnOff( BOOL enable , int dN)
{
	int08 ret = 0;
	
	ret = I2CSetLedOnOff( enable , dN);
	if( ret !=0 )
		return FALSE;
	
	return TRUE;
}

BOOL GpioLedOnOff( BOOL enable , int dN)
{
	int08 ret = 0;
	
	ret = GpioSetLedOnOff( enable , dN);
	if( ret != 0 )
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SetLedDac( int16 leveld , int dN)
{
	int08 ret = 0;
	if( leveld > 1023 || (leveld < 91 && leveld != 0 ) )
	{
		puts("dac level is not in the range(0,91-1023).");
		puts("Set dac failed.");
		return FALSE;
	}
	ret = I2cSetLedDac(leveld , dN);
	if( ret != 0 )
		return FALSE;
	return TRUE;
}

BOOL SetSource( INPUT_SOURCE source , int dN)
{
	int08 ret = 0;
	
	if( source == HDMI_EXTERNAL )
	{
		ret = I2cSetInputSource( 0 , dN);
		if( ret != 0 )
			return FALSE;
	}
	else if( source == SOLID_FIELD )
	{
		ret = I2cSetInputSource( 1 , dN);
		if( ret != 0 )
			return FALSE;
		ret = I2cSetPattern( 0 , dN);
		if( ret != 0 )
			return FALSE;
	}
	else if( source == CHECKER_BOARD )
	{
		ret = I2cSetInputSource( 1 , dN);
		if( ret != 0 )
			return FALSE;
		ret = I2cSetPattern( 7 , dN);
		if( ret != 0 )
			return FALSE;
	}
	else if( source == RAMP )
	{
		ret = I2cSetInputSource( 1 , dN);
		if( ret != 0 )
			return FALSE;
		ret = I2cSetPattern( 1 , dN);
		if( ret != 0 )
			return FALSE;
	}
	
	return TRUE;
}

BOOL SetDmdFlip( FLIP_ORIENTATION orientation , int dN)
{
	int08 ret = 0;
	
	if( orientation == NO_FLIP )
	{
		ret = I2CSetFlip( 0 , dN);
		if( ret != 0 )
			return FALSE;
	}
	else if( orientation == V )
	{
		ret = I2CSetFlip( 4 , dN);
		if( ret != 0 )
			return FALSE;
	}
	else if( orientation == H )
	{
		ret = I2CSetFlip( 2 , dN);
		if( ret != 0 )
			return FALSE;
	}
	else if( orientation == HV )
	{
		ret = I2CSetFlip( 6 , dN);
		if( ret != 0 )
			return FALSE;
	}
	
	return TRUE;
}

int16 GetLedDac( int dN)
{
	int08 ret = 0;
	int16 level_R = -1;
	int16 level_G = -1;
	int16 level_B = -1;
	ret = I2cGetLedDac( &level_R,&level_G,&level_B , dN );
	if( ret != 0 )
		return -1;
		
	return level_B;
}


char* GetAppVer(int dN)
{
	int08 ret = 0;
	char* version = (char* )malloc(12);
	
	ret = I2cGetAppVersion( version , dN);
	if( ret != 0 )
		return "NULL";

	return version;
}


BOOL GetLedOnOff( int dN)
{
	int08 ret = 0;
    BOOL ledEnableR, ledEnableG, ledEnableB;
	
	ret = I2CGetLedOnOff(&ledEnableR, &ledEnableG, &ledEnableB , dN);
	if( ret != 0 )
		return -1;
	
	return ledEnableB;
}

float GetLedTemperature( int dN)
{	
	int08 ret = 0;
	float temperature = -1;

	
	ret = I2cGetLedTemperature( &temperature , dN);
	if( ret != 0 )
		return -1;

	return temperature;
}



int16 GetLightSensor( int* type ,int dN )
{
	int08 ret = 0;
	int16 light = -1;
	ret = I2cGetLightSensor( &light, &type, dN );
	//*type = 1;
	if( ret != 0 )
		return -1;
	return light;
}

FLIP_ORIENTATION GetDmdFlipOrientation( int dN)
{
	FLIP_ORIENTATION orientation;
	byte status, horizontal, vertical;
	int08 ret = 0;
	ret = I2CGetFlip( &status , dN);
	
	horizontal = status & 0x02;
	vertical = status & 0x04;
	
	if(horizontal == 0x02 && vertical == 0x04)
	{
		orientation = HV;
	}
	else if(horizontal == 0x00 && vertical == 0x00)
	{
		orientation = NO_FLIP;
	}
	else if(horizontal == 0x00 && vertical == 0x04)
	{
		orientation = V;
	}
	else
	{
		orientation = H;
	}
	
	return orientation;
}


char* GetSerialNumber( int dN )
{
	int08 ret = 0;
	char* serialNumTemp = (char* )malloc(24);
	int08 snLength = 0;
	ret = SpiGetSerialNumber( serialNumTemp, &snLength, dN );
	if( ret != 0 )
		return "NULL";

	char* serialNum = (char* )malloc(snLength);
	strncpy( serialNum, serialNumTemp, snLength );
	return serialNum;
}

void GetUniformityMask( int dN )
{
	int08 ret = 0;
	char fileName[30];
	char* serialNum = GetSerialNumber(dN);
	
	sprintf( fileName, "%s.png", serialNum );
	ret = SpiGetMask( NATIVE, fileName, dN );
	if( ret != 0 )
		return;
	puts("Save native resolution uniformity mask finished.");
	free(serialNum);
}

void Check_SYS_RDY_Busy( int dN )
{
	BOOL isBusy = TRUE;
	while(TRUE)
	{
		GpioGetSysrdyBusy(&isBusy, dN);
		if(isBusy == FALSE)
		{
			///puts("SYS_RDY Get Ready ! ");
			break;
		}	
	}
}

void Check_SPI_RDY_Busy( int dN )
{
	BOOL isBusy = TRUE;
	while(TRUE)
	{
		GpioGetSpirdyBusy(&isBusy, dN);
		if(isBusy == FALSE)
		{
			///puts("SPI_RDY Get Ready ! ");
			break;
		}	
	}
}
//New Add
/* es
void CheckI2CBusy( int dN )
{
	BOOL isBusy = TRUE;
	while(TRUE)
	{
		GpioGetI2CBusy(&isBusy, dN);
		if(isBusy == FALSE)
		{
			break;
		}	
	}
}
//New Add
*/