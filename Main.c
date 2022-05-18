/****************************************************************************/
/* Main.c                                                                   */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>

#include "Ddp1438.h"
#include "CyFunction.h"
#include "common.h"
#include "CyUSBSerial.h"
#include "NPRFLightEngine.h"

int dN = 0;

void refreshDN()
{
	CyLibraryExit();
	usleep(1000);
    	if(cyAPIInit(&dN) != CY_SUCCESS)
	{
	 	printf("Init fail\n");
	}
	//else
		//printf("main's dn is %d\n",dN);
}

int main(int argc, char **agrv){
	
	byte padType = 0x00;
	char *appver;
	//int lightSensorType = -1;
	float temperature_CBM40_GEN4 = -1;	
	float temperature_CBT39 = -1;
	float temperature_CBM25X_385, temperature_CBM25X_405;
	double DW_CBT39 = 1.6;
    double DW_CBM40_GEN4 = 0.88;
	double DW_CBM25X = 1.2;
	double Vol = 4;
	double Vol_CBM25X_385, Vol_CBM25X_405;
	double DI_CBM25X = 0.0048828;
	double I;
	
	char cmd[80];
	char *func, *para;
	signal(SIGUSR1, deviceHotPlug);
	if ( cyAPIInit(&dN) != CY_SUCCESS )
	{
		printf("Failed to initialize cypress API.\n");
		return -1;
	}
	else
		printf("initialize cypress API success.\n");
		
	do{
		
		printf(">>");	
			
		fgets(cmd, 80, stdin);
		*strrchr(cmd, '\n') = '\0';
		
		func = strtok(cmd, " ");
		para = strtok(NULL, " ");
		if( func == NULL )
			continue;
		
		if( strcmp(func, "lsdev") == 0 )
		{
			printListOfDevices( TRUE );
		}
		else if( strcmp(func, "proj") == 0 )
		{
			refreshDN();
			if( strcmp(para, "off") == 0 )
				ProjectorOnOff( FALSE , dN);
			else if( strcmp(para, "on") == 0 )
			{
				ProjectorOnOff( TRUE , dN);
				padType = GetPad300xChipId(dN);
				if(padType == 0xd3)
					puts("PAD Type is PAD3000");
				else if(padType == 0xe3)
					puts("PAD Type is PAD3005");
				else
					puts("PAD Can't not detect");
				
				appver = GetAppVer(dN);
			}
				
			else
			{
				puts( "Projector On/Off: Invalid parameter." );
				continue;
			}
		}
		else if( strcmp(func, "print") == 0)
		{
			refreshDN();
			int16 frames = 0;
			frames = atoi(para);
			Print(frames, dN);
		}
		else if( strcmp(func, "led") == 0 )
		{
			refreshDN();
			if( strcmp(para, "off") == 0 )
				LedOnOff( FALSE , dN);
			else if( strcmp(para, "on") == 0 )
				LedOnOff( TRUE , dN);
			else
			{
				puts( "LED On/Off: Invalid parameter." );
			}
		}
		else if( strcmp(func, "sdac") == 0 )
		{
			refreshDN();
			int16 leveld = 0;
			if( para == NULL )
				puts( "Set Led Dac Level: Invalid parameter." );
			else
			{
				leveld = atoi(para);
				SetLedDac(leveld, dN);
			}
		}
		else if( strcmp(func, "sflip") == 0 )
		{
			refreshDN();
			if( strcmp(para, "no") == 0 )
				SetDmdFlip(NO_FLIP, dN);
			else if( strcmp(para, "h") == 0 )
				SetDmdFlip(H, dN);
			else if( strcmp(para, "v") == 0 )
				SetDmdFlip(V, dN);
			else if( strcmp(para, "hv") == 0 )
				SetDmdFlip(HV, dN);
			else
			{
				puts( "Set Dmd Flip: Invalid parameter." );
			}
		}
		else if( strcmp(func, "ssource") == 0 )
		{
			refreshDN();
			if( strcmp(para, "hdmi") == 0 )
				SetSource( HDMI_EXTERNAL , dN);
			else if( strcmp(para, "ramp") == 0 )
				SetSource( RAMP , dN);
			else if( strcmp(para, "checker") == 0 )
				SetSource( CHECKER_BOARD , dN);
			else if( strcmp(para, "solid") == 0 )
				SetSource( SOLID_FIELD , dN);
			else
				puts( "Set source: Invalid parameter." );
		}
		else if( strcmp(func, "gdac") == 0 )
		{
			refreshDN();
			int16 level = GetLedDac(dN);
			printf("Get Led Dac level = %d\n", level);
		}
		else if( strcmp(func, "gappver") == 0 )
		{
			refreshDN();
			char* appVer = GetAppVer(dN);
			printf("Get Application Version: %s\n", appVer);
			free(appVer);
		}
		else if( strcmp(func, "gledtemp") == 0 )
		{	
		    refreshDN();
			int16 Dac = GetLedDac(dN);
			
			float temperature = GetLedTemperature(dN);
			
			bool LedOnOff = GetLedOnOff(dN);
			
			I = DI_CBM25X*(Dac+1);
			
			Vol_CBM25X_385 = -0.0013 * I*I*I - 0.0621 * I*I + 0.8885 * I + 6.3739;
			Vol_CBM25X_405 = 0.0301 * I*I*I - 0.3248 * I*I + 1.5666 * I + 6.1046;
			
			//printf("Dac is %d\ntemp is %f\nLedOnOff is %d\nI is %f\nVol_385 is %f\nVol_405 is %f\n",Dac,temperature,LedOnOff,I,Vol_CBM25X_385,Vol_CBM25X_405);
			
			temperature_CBM25X_385 = (int)(temperature / 10 + (LedOnOff ? DW_CBM25X * Vol_CBM25X_385 * DI_CBM25X * (Dac + 1) : 0));
			temperature_CBM25X_405 = (int)(temperature / 10 + (LedOnOff ? DW_CBM25X * Vol_CBM25X_405 * DI_CBM25X * (Dac + 1) : 0));
			
			printf("Get Led Temperature (wavelength 405) = %.2f%s\n", temperature_CBM25X_385, "℃");
			printf("Get Led Temperature (wavelength 385) = %.2f%s\n", temperature_CBM25X_405, "℃");
		}
		else if( strcmp(func, "glight") == 0 )
		{
			refreshDN();
			int type = 2;
			char type_name[15];
			int16 light = GetLightSensor(&type, dN);
			
			if(type == 0)
			{
			  	type_name[0] = 'T';
				type_name[1] = 'S';
				type_name[2] = 'L';
				type_name[3] = '2';
				type_name[4] = '5';
				type_name[5] = '7';
				type_name[6] = '1';
				type_name[7] = '5';
				type_name[8] = '\0';
				
			}
			else if(type == 1)
			{
				type_name[0] = 'T';
				type_name[1] = 'S';
				type_name[2] = 'L';
				type_name[3] = '2';
				type_name[4] = '5';
				type_name[5] = '6';
				type_name[6] = '1';
				type_name[7] = '\0';
			}
			else
			{
				type_name[0] = 'N';
				type_name[1] = 'O';
				type_name[2] = 'N';
				type_name[3] = 'E';
				type_name[4] = '\0';
				printf("Get Type Fail\n");
			}
			
			
			
			printf("Get Light Sensor(%s) Value: %d\n", type_name, light );
		}
		else if( strcmp(func, "gflip") == 0 )
		{
			refreshDN();
			FLIP_ORIENTATION oriantation = GetDmdFlipOrientation(dN);
			if( oriantation == NO_FLIP )
				puts("The Flip Oriantation: NO_FLIP.");
			else if( oriantation == H )
				puts("The Flip Oriantation: Horizontal Flip.");
			else if( oriantation == V )
				puts("The Flip Oriantation: Vertical Flip.");
			else if( oriantation == HV )
				puts("The Flip Oriantation: Horizontal & Vertical Flip.");
		}
		else if( strcmp(func, "exit") == 0 )
		{
			CyLibraryExit();
			break;
		}
		else if( strcmp(func, "help") == 0 )
		{
			puts("[Command List]");
			puts("");
			puts("●lsdev             : List the cypress device info.");
			puts("●proj <on,off>     : Turn the projector On or Off.");
			puts("●print <para>      : Set the curing time digital value.");
			puts("                     Para(frames) must be an integer 0 to 65535.");
			puts("                     unit frames eqaul to 1/60 second, e.g. 1s = 60");
			puts("●led <on,off>      : Turn the UV LED On or Off.");
			puts("●sdac <para>       : Set the UV LED current digital value.");
			puts("                     Para must be an integer 0 or 50~1000.");
			puts("●ssource <para>    : Set the source of pattern.");
			puts("                     Para can be ramp,checker,solid.");
			puts("●sflip <para>      : Set the DMD flip oriantation.");
			puts("                     Para can be no,h,v,hv.");
			puts("                     no: no flip");
			puts("                     h: horizontal");
			puts("                     v: vertical");
			puts("                     hv: horizontal&vertical");
			puts("●gdac              : Get the UV LED current digital value.");
			puts("●gappver           : Get the F/W version.");
			puts("●gledtemp          : Get the temperature of the UV LED.");
			puts("●gflip             : Get the DMD flip oriantation.");
			puts("●glight            : Get the light sensor value.");
			puts("●exit              : To exit the program.");
		}
		else
			puts("Invalid cmd(input \"help\" to list all the valid command.)");
			
	} while(TRUE);
	
	return 0;	
}

