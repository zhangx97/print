all:
	gcc -Wall -g -o NPRF_Controller_Linux *.c -lwiringPi -l cyusbserial -I ./include -W
	cp 90-cyusb.rules /etc/udev/rules.d 
	cp NPRF_Controller_Linux /usr/bin
	cp CyUSBSerial.sh /usr/bin
	chmod 777 /usr/bin/CyUSBSerial.sh
clean:
	rm -f NPRF_Controller_Linux
help:
	@echo	'make		would compile and create the library and create a link'
	@echo	'make clean	would remove the library and the soft link to the library (soname)'
