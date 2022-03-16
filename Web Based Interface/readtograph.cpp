#include <iostream>
#include <fstream>
#include <wiringPi.h>
#include <fstream>

using namespace std;

int main()
{
	piLock(2);
	fstream fs;
	fs.open("/var/www/html/phpoutput.txt", fstream::in);
	double y;
	fs >> y;
	fs.close();
	int fluor = y;
	cout << y << endl;
	piUnlock(2);
	return y;
}
