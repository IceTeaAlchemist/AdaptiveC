#include <iostream>
#include <fstream>
#include <wiringPi.h>
#include <fstream>

using namespace std;

int main()
{
	piLock(2);
	fstream fs;
	fs.open("/home/pi/Documents/GaussNewtonBeta/phpoutput.txt", fstream::in);
	string line;
	getline(fs,line);
	string temp;
	getline(fs,temp);
	string fluortemp;
	getline(fs,fluortemp);
	cout << line << endl;
	cout << temp << endl;
	cout << fluortemp << endl;
	piUnlock(2);
	return 0;
}
