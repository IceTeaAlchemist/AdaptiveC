#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>
#include <vector>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include "qiagen.h"

using namespace std;

int main()
{
	// This may need to be changed on your system depending on where the qiagen is connected.
	qiagen sens1("/dev/ttyUSB0");
	// getTemp is not implemented, for list of registers check the manual.
	string temp = sens1.readqiagen(258,2);
	cout << "Qiagen temperature reading: " << temp << endl;
	sens1.LED_on(1);
	usleep(2000000);
	sens1.LED_off(1);
	sens1.LED_on(2);
	usleep(2000000);
	sens1.LED_off(2);
	return 0;
}
	
