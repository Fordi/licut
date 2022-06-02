// $Id: licut_probe.cpp 1 2011-01-28 21:55:10Z henry_groover $

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dirent.h>
#include <termios.h>
#include <linux/serial.h>

#include "licut_probe.h"

char LicutProbe::errmsg[512] = {0};


int get_usb_id(char* usbPath) {
	char propPath[256];
	int vendor = 0;
	int product = 0;
	FILE* propFile;

	sprintf(propPath, "%s/idVendor", usbPath);
	
	propFile = fopen(propPath, "r");
	if (propFile == NULL) return 0;
	fscanf(propFile, "%4x", &vendor);
	fclose(propFile);

	sprintf(propPath, "%s/idProduct", usbPath);
	propFile = fopen(propPath, "r");
	if (propFile == NULL) return 0;
	fscanf(propFile, "%4x", &product);
	fclose(propFile);
	return (vendor << 16) + product;
}

char ttyName[256];
char usbSysPath[256];


int find_attached_cricut(int verbose = 0) {
	char syspath[256];
	DIR* usbDevs = opendir(USB_DEV_ROOT);

	if (usbDevs == NULL) {
		puts("Unable to read /sys/bus/usb; are you on Linux?\n");
		return -1;
	}
	struct dirent* entry;
	char usbPath[256];
	struct stat propStat;
	while (entry = readdir(usbDevs)) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}
		sprintf(usbPath, "%s/%.128s", USB_DEV_ROOT, entry->d_name);
		int usbId = get_usb_id(usbPath);
		if (usbId == 0) continue;
		if (verbose > 0) {
			printf("Checking %s (%04x:%04x)\n", entry->d_name, usbId >> 16, usbId & 0xFFFF);
		}
		for (int i = 0; i < KNOWN_IDS_SIZE; i++) {
			if (usbId == KNOWN_IDS[i]) {
				printf("ID match, looking for tty\n");
				DIR* subDir = opendir(usbPath);
				struct dirent* sub;
				while (sub = readdir(subDir)) {
					if (strcmp(sub->d_name, ".") == 0 || strcmp(sub->d_name, "..") == 0) {
						continue;
					}
					DIR* ttys;
					struct dirent* tty;
					char ttyPath[256];

					sprintf(ttyPath, "%.96s/%.96s/tty", usbPath, sub->d_name);
					ttys = opendir(ttyPath);
					if (ttys == NULL) continue;
					while (tty = readdir(ttys)) {
						if (strcmp(tty->d_name, ".") == 0 || strcmp(tty->d_name, "..") == 0) {
							continue;
						}
						closedir(ttys);
						closedir(subDir);
						closedir(usbDevs);
						sprintf(ttyName, "%s", tty->d_name);
						sprintf(usbSysPath, "%s", usbPath);
						return 0;
					}
					closedir(ttys);
				}
				closedir(subDir);
			}
		}		
	}
	closedir(usbDevs);
	return 1;
}

int LicutProbe::Open( int verbose /*= 0*/ )
{
	char devpath[256];
	bool found_devname = false;
	if (find_attached_cricut(verbose) == 0) {
		printf("Found tty: %s in %s\n", ttyName, usbSysPath);
		found_devname = true;
		sprintf(devpath, "/dev/%.128s", ttyName);
	}
	if (!found_devname) {
		sprintf(errmsg, "Couldn't locate known Cricut device on USB.  Is it connected?\n");
		return -1;
	}
	int handle = open( devpath, O_RDWR | O_NOCTTY );
	if (handle <= 0)
	{
		sprintf( errmsg, "Failed to open %s - %d (%s)\n", devpath, errno, strerror(errno) );
		return -1;
	}

	if (verbose) printf( "Opened %s handle %d\n", devpath, handle );

        struct termios oldtio,newtio;
        
        tcgetattr( handle, &oldtio ); /* save current port settings */
    
	if (verbose) printf( "setting parameters\n" );

	bzero( &newtio, sizeof(newtio) );
	// Set custom rate to 200kbps 8N1 - we're actually sending 8N2 but get 8N1 back
	newtio.c_cflag = B38400 | /*CRTSCTS |*/ CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	
	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;
		
	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */
	
	tcflush( handle, TCIFLUSH );
	tcsetattr( handle, TCSANOW, &newtio );

	// Now use TIOCSSERIAL ioctl to set custom divisor
	// FTDI uses base_baud 24000000 so in theory a divisor
	// of 120 should give us 200000 baud...
	struct serial_struct sio; // From /usr/include/linux/serial.h
	int ioctl_res = ioctl( handle, TIOCGSERIAL, &sio );
	if (ioctl_res < 0)
	{
		sprintf( errmsg, "Failed TIOCGSERIAL ioctl: error %d (%s)\n", errno, strerror(errno) );
		close( handle );
		return -1;
	}

	if (verbose) printf( "ioctl(TIOCGSERIAL) returned %d, flags was %04x, baud_base %u\n", ioctl_res, sio.flags, sio.baud_base );
	sio.flags = ((sio.flags & ~ASYNC_SPD_MASK) | ASYNC_SPD_CUST);
	sio.custom_divisor = sio.baud_base / 200000;
	ioctl_res = ioctl( handle, TIOCSSERIAL, &sio );
	if (ioctl_res < 0)
	{
		sprintf( errmsg, "Failed TIOCSSERIAL ioctl: error %d (%s)\n", errno, strerror(errno) );
		close( handle );
		return -1;
	}
	if (verbose) printf( "ioctl(TIOCSSERIAL) returned %d, new flags %04x, new custom_divisor %u\n", ioctl_res, sio.flags, sio.custom_divisor );

	return handle;
}

void LicutProbe::Close( int handle )
{
	close( handle );
}

