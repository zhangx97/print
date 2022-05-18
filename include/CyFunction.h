/****************************************************************************/
/* CyFunction.h                                                             */
/*                                                                          */
/* Define Cypress GPIO/I2C Write/Read/Initial function.                     */
/****************************************************************************/

#ifndef _CYFUNCTION_H_
#define	_CYFUNCTION_H_

#include "CyUSBSerial.h"
#include "common.h"
#include <stdbool.h>

void deviceHotPlug ();
CY_RETURN_STATUS cyAPIInit(int *dN);
void printListOfDevices (bool isPrint);
bool isCypressDevice (int deviceNum);
CY_RETURN_STATUS GpioWrite(int deviceNumber, int interfaceNum, UINT8 gpioNumber, UINT8 value);
CY_RETURN_STATUS GpioRead(int deviceNumber, int interfaceNum, UINT8 gpioNumber, UINT8 *value);

CY_RETURN_STATUS I2CWrite(int deviceNumber, int interfaceNum, UINT8 address, int len, UINT8 *buf);
CY_RETURN_STATUS I2CRead(int deviceNumber, int interfaceNum, UINT8 address, int len, UINT8 *buf);


#endif
