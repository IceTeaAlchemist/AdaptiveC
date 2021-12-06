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
//	pinMode(2, INPUT);
	int fd = wiringPiI2CSetup(DEVICE_ID);
	int resetd = wiringPiI2CSetup(RESET_ID);
	int data = 0;
	if(fd == -1)
	{
		cout << "Failed to initiate I2C communication." << endl;
		return -1;
	}
	cout << "I2C setup successful." << endl;
	cout << "Communicating with " << fd << "." <<  endl;
	wiringPiI2CWriteReg16(fd, HITHRESH, 0x8000);
	wiringPiI2CWriteReg16(fd, LOWTHRESH, 0x7FFF);
	wiringPiI2CWriteReg16(fd, CONFIG, 0x44E0);
//	wiringPiI2CWriteReg16(fd, CONFIG, 0b1000000000100011);
	int ready = wiringPiI2CReadReg16(fd,CONFIG);
	cout << "Config register: " << ready << endl;
	for(int i = 0; i < 100; i++)
	{
		data = wiringPiI2CReadReg16(fd,CONVERSION);
		ready = wiringPiI2CReadReg16(fd,CONFIG);
		// float voltage = 5.0*data/65536.0;
		cout << data << endl;
		cout << "Config register: " << ready << endl;
		usleep(4000);
	}
	return 0;
}
