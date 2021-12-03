#include <iostream>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>

#define DEVICE_ID 0x48
#define RESET_ID 0x00

#define CONVERSION 0x00
#define CONFIG 0x01
#define LOWTHRESH 0x02
#define HITHRESH 0x03

using namespace std;

int main()
{
	wiringPiSetup();
	int fd = wiringPiI2CSetup(DEVICE_ID);
	int resetd = wiringPiI2CSetup(RESET_ID);
	int data = 0;
	if(fd == -1 || resetd == -1)
	{
		cout << "Failed to initiate I2C communication." << endl;
		return -1;
	}
	cout << "I2C setup successful." << endl;
	cout << "Communicating with " << fd << "." <<  endl;
	cout << "Reset on " << resetd << "." << endl;
	wiringPiI2CWrite(resetd, 0b00000110);
	wiringPiI2CWriteReg16(fd, HITHRESH, 0x7FFF);
	wiringPiI2CWriteReg16(fd, LOWTHRESH, 0x8000);
	wiringPiI2CWriteReg16(fd, CONFIG, 0b0000010110000011);
	data = wiringPiI2CReadReg16(fd, CONFIG);
	cout << "Config register: " << data << endl;
	data = wiringPiI2CReadReg16(fd, HITHRESH);
	cout << "High threshold: " << data << endl;
	data = wiringPiI2CReadReg16(fd, LOWTHRESH);
	cout << "Low threshold: " << data << endl;
	data = wiringPiI2CReadReg16(fd,CONVERSION);
	cout << "Conversion: " << data << endl;
	cout << "Reset complete." << endl;
	return 0;
}
