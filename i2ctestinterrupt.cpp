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

void sampletriggered(void);

int fd;
int main()
{
	wiringPiSetup();
	pinMode(2, INPUT);
	pinMode(3, OUTPUT);
	wiringPiISR(2, INT_EDGE_FALLING, sampletriggered);
	fd = wiringPiI2CSetup(DEVICE_ID);
	if(fd == -1)
	{
		cout << "Failed to initiate I2C communication." << endl;
		return -1;
	}
	cout << "I2C setup successful." << endl;
	wiringPiI2CWriteReg16(fd,HITHRESH, 0x8000);
	wiringPiI2CWriteReg16(fd,LOWTHRESH, 0x7FFF);
	wiringPiI2CWriteReg16(fd,CONFIG, 0b1100010010100000);
	delay(10);
	int ready = wiringPiI2CReadReg16 (fd,CONFIG);
	cout <<"Config register: " <<  ready << endl;
	int data = wiringPiI2CReadReg16(fd,CONVERSION);
//	cout << data << endl;
	while(1)
	{
		digitalWrite(3,HIGH);
		usleep(500000);
		digitalWrite(3,LOW);
		usleep(500000);
	}
	return 0;
}

void sampletriggered(void)
{
	int data = wiringPiI2CReadReg16(fd,CONVERSION);
	cout << data << endl;
}
