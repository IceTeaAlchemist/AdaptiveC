#include <iostream>
#include <errno.h>
#include <wiringPiI2C.h>
#include <ads1115.h>
#include <unistd.h>

using namespace std;

int main()
{
	int ads,result;
	ads = ads1115Setup(2,0x48);
	cout << "Init result: " << ads << endl;

	for(int i = 0; i < 1000; i++)
	{
		result = myAnalogRead(ads,0);
		cout << result <<endl;
		usleep(1000000);
	}
}
