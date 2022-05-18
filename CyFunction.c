/****************************************************************************/
/* CyFunction.c                                                             */
/*                                                                          */
/****************************************************************************/
#include "CyFunction.h"
#include "CyUSBSerial.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define CY_MAX_DEVICES 30
#define CY_MAX_INTERFACES 4

//#define DEBUG

#ifdef  DEBUG
	#define D(x) (x)
#else
	#define D(x)
#endif

typedef struct _CY_DEVICE_STRUCT {
    int deviceNumber;
    int interfaceFunctionality[CY_MAX_INTERFACES];
    BOOL isI2c;
    BOOL isSpi;
    BOOL isGpio;
    int numInterface; 
}CY_DEVICE_STRUCT;

CY_DEVICE_STRUCT *glDevice;
int i2cDeviceIndex[CY_MAX_DEVICES][CY_MAX_INTERFACES];
unsigned char *deviceNumber = NULL;
uint08 numDevices = 0;
int cyDevices, i2cDevices = 0;
int selectedDeviceNum = -1, selectedInterfaceNum = -1;
bool exitApp = false;
unsigned short pageAddress = -1;
short readWriteLength = -1;
bool deviceAddedRemoved = false;

void deviceHotPlug () {
    
    CY_RETURN_STATUS rStatus;
    deviceAddedRemoved = true;
    selectedDeviceNum = -1;
    selectedInterfaceNum = -1;
    printf ("Device of interest Removed/Added \n");
    rStatus = CyGetListofDevices (&numDevices);
    if (rStatus != CY_SUCCESS) {
        printf ("CY:Error in Getting List of Devices: Error NO:<%d> \n", rStatus);
        return;
    }
    usleep(100000);
    printListOfDevices (false);
}

CY_RETURN_STATUS cyAPIInit( int * dN)
{
    CY_RETURN_STATUS rStatus;
	//signal (SIGUSR1, deviceHotPlug);
    glDevice = (CY_DEVICE_STRUCT *)malloc (CY_MAX_DEVICES *sizeof (CY_DEVICE_STRUCT));
    if (glDevice == NULL){
        printf ("Memory allocation failed ...!! \n");
        return -1;
    }
    rStatus = CyLibraryInit ();
    if (rStatus != CY_SUCCESS) {
        printf ("CY:Error in Doing library init Error NO:<%d> \n", rStatus);
        return rStatus;
    }
    rStatus = CyGetListofDevices (&numDevices);
    if (rStatus != CY_SUCCESS) {
        printf ("CY:Error in Getting List of Devices: Error NO:<%d> \n", rStatus);
        return rStatus;
    }
    printListOfDevices(false);
    int DEVICE0_NUM;
    DEVICE0_NUM = glDevice[0].deviceNumber;
    *dN = DEVICE0_NUM;
    //printf("Device Number : %d\n",DEVICE0_NUM);
	return rStatus;
}

bool isCypressDevice (int deviceNum) {
    CY_HANDLE handle;
    unsigned char interfaceNum = 0;
    unsigned char sig[6];
    CY_RETURN_STATUS rStatus;
    rStatus = CyOpen (deviceNum, interfaceNum, &handle);
    if (rStatus == CY_SUCCESS){
        rStatus = CyGetSignature (handle, sig);
        if (rStatus == CY_SUCCESS){
            CyClose (handle);
            return true;
        }
        else {
            CyClose (handle);
            return false;
        }
    }
    else 
        return false;
}
void printListOfDevices (bool isPrint)
{
    int  index_i = 0, index_j = 0, i, j, countOfDevice = 0, devNum;
    int length, index = 0, numInterfaces, interfaceNum;
    bool set1 = false;

    ///unsigned char deviceID[CY_MAX_DEVICES];
    ///unsigned char functionality[64];
	char functionality[64];
    CY_DEVICE_INFO deviceInfo;
    CY_DEVICE_CLASS deviceClass[CY_MAX_INTERFACES];
    CY_DEVICE_TYPE  deviceType[CY_MAX_INTERFACES];
    CY_RETURN_STATUS rStatus;

    deviceAddedRemoved = false; 
    CyGetListofDevices (&numDevices);
    //printf ("The number of devices is %d \n", numDevices); 
    for (i = 0; i < numDevices; i++){
        for (j = 0; j< CY_MAX_INTERFACES; j++)
            glDevice[i].interfaceFunctionality[j] = -1;
    }
    if (isPrint){
        printf ("\n\n---------------------------------------------------------------------------------\n");
        printf ("Device Number | VID | PID | INTERFACE NUMBER | FUNCTIONALITY \n");
        printf ("---------------------------------------------------------------------------------\n");
    }
    cyDevices = 0;
    for (devNum = 0; devNum < numDevices; devNum++){
        rStatus = CyGetDeviceInfo (devNum, &deviceInfo);
        interfaceNum = 0;
        if (!rStatus)
        {
            if (!isCypressDevice (devNum)){
                continue;
            }
            strcpy (functionality, "NA");
            numInterfaces = deviceInfo.numInterfaces;
            glDevice[index].numInterface = numInterfaces;
            cyDevices++;
            
            while (numInterfaces){
                if (deviceInfo.deviceClass[interfaceNum] == CY_CLASS_VENDOR)
                {
                    glDevice[index].deviceNumber = devNum;
                    switch (deviceInfo.deviceType[interfaceNum]){
                        case CY_TYPE_I2C:
                            glDevice[index].interfaceFunctionality[interfaceNum] = CY_TYPE_I2C;
                            strcpy (functionality, "VENDOR_I2C");
                            glDevice[index].isI2c = true;
                            break;
                        case CY_TYPE_SPI:
                            glDevice[index].interfaceFunctionality[interfaceNum] = CY_TYPE_SPI;
                            strcpy (functionality, "VENDOR_SPI");
                            glDevice[index].isSpi = true;
                            break;
                        default:
                            strcpy (functionality, "NA");
                            break;    
                    }
                }
                else if (deviceInfo.deviceClass[interfaceNum] == CY_CLASS_CDC){
                    strcpy (functionality, "NA");
                }
                if (isPrint) {
                    printf ("%d             |%x  |%x    | %d                | %s\n",\
                            index, \
                            deviceInfo.vidPid.vid, \
                            deviceInfo.vidPid.pid,  \
                            interfaceNum, \
                            functionality \
                            );
                }
                interfaceNum++;
                numInterfaces--;
            }
            index++;
        }
    }
    if (isPrint){
        printf ("---------------------------------------------------------------------------------\n\n");
    }
}

CY_RETURN_STATUS GpioWrite(int deviceNumber, int interfaceNum, UINT8 gpioNumber, UINT8 value)
{
	CY_HANDLE cyHandle;
	CY_RETURN_STATUS rStatus;

	D(printf("Opening GPIO device with device number %d...\n", deviceNumber));

	//Open the device at deviceNumber
	rStatus = CyOpen(deviceNumber, interfaceNum, &cyHandle);

	if (rStatus != CY_SUCCESS){
		printf("GPIO Device open failed. Error NO:<%d>\n", rStatus);
		return rStatus;
	}
	
	D(printf("Performing GPIO Write operation...\n"));

	if (value > 0)
		value = 1;
	else
		value = 0;
		
	rStatus = CySetGpioValue(cyHandle, gpioNumber, value);
	if (rStatus != CY_SUCCESS){
		printf("CySetGpioValue Failed. Error NO:<%d>\n", rStatus);
		CyClose (cyHandle);
		return rStatus;
	}

	D(printf("Completed GPIO write !\n"));
	CyClose (cyHandle);
	return CY_SUCCESS;
}

CY_RETURN_STATUS GpioRead(int deviceNumber, int interfaceNum, UINT8 gpioNumber, UINT8 *value)
{
	CY_HANDLE cyHandle;
	CY_RETURN_STATUS rStatus;

	D(printf("Opening GPIO device with device number %d...\n", deviceNumber));

	//Open the device at deviceNumber
	rStatus = CyOpen(deviceNumber, interfaceNum, &cyHandle);

	if (rStatus != CY_SUCCESS){
		printf("GPIO Device open failed. Error NO:<%d>\n", rStatus);
		return rStatus;
	}


	// I2C Read/Write operations
	D(printf("Performing GPIO Read operation...\n"));
	
	rStatus = CyGetGpioValue(cyHandle, gpioNumber, value);
	if (rStatus != CY_SUCCESS){
		printf("CyGetGpioValue Failed. Error NO:<%d>\n", rStatus);
		CyClose (cyHandle);
		return rStatus;
	}

	D(printf("Completed GPIO read !\n"));
	CyClose (cyHandle);
	return CY_SUCCESS;
}

CY_RETURN_STATUS I2CWrite(int deviceNumber, int interfaceNum, UINT8 address, int len, UINT8 *buf)
{
	CY_HANDLE handle;
	CY_DATA_BUFFER cyDatabuffer;
	CY_I2C_DATA_CONFIG cyI2CDataConfig;

	CY_RETURN_STATUS rStatus;

	D(printf("Opening I2C device with device number %d...\n", deviceNumber));
	//Open the device at deviceNumber
	rStatus = CyOpen(deviceNumber, interfaceNum, &handle);

	if (rStatus != CY_SUCCESS){
		printf("I2C Device open failed. Error NO:<%d>\n", rStatus);
		return rStatus;
	}
	// I2C Read/Write operations
	D(printf("Performing I2C Write operation...\n"));

	//Initialize the CY_I2C_DATA_CONFIG variable
	cyI2CDataConfig.slaveAddress = address;
	cyI2CDataConfig.isStopBit = TRUE;
	//Initialize the CY_DATA_BUFFER variable
	cyDatabuffer.buffer = buf;
	cyDatabuffer.length = len;

	rStatus = CyI2cWrite(handle, &cyI2CDataConfig, &cyDatabuffer, 5000);
	if (rStatus != CY_SUCCESS){
		printf("CyI2cWrite Failed. Error NO:<%d>\n", rStatus);
		CyClose (handle);
		return rStatus;
	}

	D(printf("Completed I2C write transfer with %d bytes.\n", len));

	CyClose (handle);
	return rStatus;
}

CY_RETURN_STATUS I2CRead(int deviceNumber, int interfaceNum, UINT8 address, int len, UINT8 *buf)
{
	CY_HANDLE handle;
	CY_DATA_BUFFER cyDatabuffer;
	CY_I2C_DATA_CONFIG cyI2CDataConfig;

	CY_RETURN_STATUS rStatus;

	D(printf("Opening I2C device with device number %d...\n", deviceNumber));

	//Open the device at deviceNumber
	rStatus = CyOpen(deviceNumber, interfaceNum, &handle);

	if (rStatus != CY_SUCCESS){
		printf("I2C Device open failed. Error NO:<%d>\n", rStatus);
		return rStatus;
	}


	// I2C Read/Write operations
	D(printf("Performing I2C Read operation...\n"));

	//Initialize the CY_I2C_DATA_CONFIG variable
	cyI2CDataConfig.slaveAddress = address;
	cyI2CDataConfig.isStopBit = true;
	cyI2CDataConfig.isNakBit = true;
	//Initialize the CY_DATA_BUFFER variable
	cyDatabuffer.buffer = buf;
	cyDatabuffer.length = len;


	rStatus = CyI2cRead(handle, &cyI2CDataConfig, &cyDatabuffer, 5000);
	if (rStatus != CY_SUCCESS){
		printf("CyI2cRead Failed. Error NO:<%d>\n", rStatus);
		CyClose (handle);
		return rStatus;
	}
	
	D(printf("Completed I2C read successfully. Read %d bytes of data.\n", len));
	D(printf("%2X,%2X",*cyDatabuffer.buffer,*(cyDatabuffer.buffer + 1)));
	
	CyClose (handle);
	return rStatus;
}
