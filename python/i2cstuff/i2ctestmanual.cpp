#include <iostream>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>

#define DEVICE_ID 0x48

#define CONVERSION 0x00
#define CONFIG 0x01
#define LOWTHRESH 0x02
#define HITHRESH 0x03

using namespace std;

int main()
{
	wiringPiSetup();
	pinMode(7, INPUT);
	int fd = wiringPiI2CSetup(DEVICE_ID);
	int data = 0;
	if(fd == -1)
	{
		cout << "Failed to initiate I2C communication." << endl;
		return -1;
	}
	cout << "I2C setup successful." << endl;
	wiringPiI2CWriteReg16(fd,LOWTHRESH, 0b0000000000000000);
	wiringPiI2CWriteReg16(fd,HITHRESH, 0b0000000100000000);
	wiringPiI2CWriteReg16(fd,CONFIG, 0b0000000011100000);
//	wiringPiI2CWriteReg16(fd,CONFIG, 0b1000000010100011);
	int ready;
	while(1)
	{
//		wiringPiI2CWriteReg16(fd,CONFIG,0b0000000010100000);
		ready = digitalRead(7);
		cout << ready << endl;
		if(ready == 0)
		{
			data = wiringPiI2CReadReg16(fd,CONVERSION);
			float voltage = 5.0*data/65536.0;
			cout << voltage << endl;
		}
	}
	return 0;
}
