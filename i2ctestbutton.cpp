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

int fd;

void buttonPressed(void);

int main()
{
	wiringPiSetup();
	pinMode(2, INPUT);
	pinMode(3, OUTPUT);
	wiringPiISR(2, INT_EDGE_FALLING, buttonPressed);
	fd = wiringPiI2CSetup(DEVICE_ID);
	int data = 0;
	if(fd == -1)
	{
		cout << "Failed to initiate I2C communication." << endl;
		return -1;
	}
	cout << "I2C setup successful." << endl;
	wiringPiI2CWriteReg16(fd,LOWTHRESH, 0x0000);
	wiringPiI2CWriteReg16(fd,HITHRESH, 0x8000);
//	wiringPiI2CWriteReg16(fd,CONFIG, 0b1000000010100011);
	int ready;
	while(1)
	{
		digitalWrite(3,HIGH);
		delay(500);
		digitalWrite(3,LOW);
		delay(500);
	}
	return 0;
}

void buttonPressed(void)
{
	wiringPiI2CWriteReg16(fd,CONFIG, 0b1000000110100000);
	delay(20);
	int data = wiringPiI2CReadReg16(fd,CONVERSION);
//	float voltage = 5.0*data/65536.0;
	cout << data <<endl;
}
