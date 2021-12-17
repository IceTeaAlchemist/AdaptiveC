#include <iostream>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <byteswap.h>
#include <ctime>
#include <chrono>
#include "GaussNewtonSine.h"
#include "GaussNewtonSine_initialize.h"

#define DEVICE_ID 0x48

#define CONVERSION 0x00
#define CONFIG 0x01
#define LOWTHRESH 0x02
#define HITHRESH 0x03

using namespace std;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

void sampletriggered(void);

int fd;

struct reading
{
    int timestamp;
    int voltage;
};

vector<reading> data;
vector<double> x;
vector<double> y;
bool startflag = 0;
FILE *output;
int cutoff = 125;

void fit(vector<double> x, vector<double> y)
{

}

int main()
{
    wiringPiSetup();
    output = fopen("binaryoutput.bin", "wb");
    if (output == NULL)
    {
        cout << "File failed to open." << endl;
        return -1;
    }
    pinMode(2, INPUT);
    pinMode(3, OUTPUT);
    GaussNewtonSine_initialize();
    wiringPiISR(2, INT_EDGE_RISING, sampletriggered);
    fd = wiringPiI2CSetup(DEVICE_ID);
    if (fd == -1)
    {
        cout << "Failed to initiate I2C communication." << endl;
        return -1;
    }
    cout << "I2C setup successful." << endl;
    wiringPiI2CWriteReg16(fd, HITHRESH, __bswap_16(0x8000));
    wiringPiI2CWriteReg16(fd, LOWTHRESH, __bswap_16(0x7FFF));
    wiringPiI2CWriteReg16(fd, CONFIG, __bswap_16(0b0000001010101000));
    delay(10);
    double beta0[4] = {1.5, 19, 1.5, 0.05};
    double lb[4] = {0, 0, 0, -3.14159};
    double ub[4] = {30000, 500, 30000,3.14159};
    double coeff[4];
    double iter;
    while (data.size() < cutoff)
    {
        piLock(0);
        // fit(x,y);
        GaussNewtonSine(x, y, beta0, lb, ub, coeff, &iter);
        cout << coeff[0] << "   " << coeff[1] << "  " << coeff[2]  << "   " << coeff[3] << "  Iter: " << iter << endl;
        piUnlock(0);
        delay(10);

    }

    fclose(output);
    return 0;
}

void sampletriggered(void)
{
    piLock(0);
    auto millisecs = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    reading now;
    now.timestamp = millisecs;
    now.voltage = __bswap_16(wiringPiI2CReadReg16(fd, CONVERSION));
    data.push_back(now);
    x.push_back((now.timestamp - data[0].timestamp)/1000.0);
    y.push_back((now.voltage/32768.0)*4.096);
    reading *ptr = &now;
    fwrite(ptr, sizeof(reading), 1, output);
    piUnlock(0);
    if (data.size() >= cutoff)
    {
        piLock(1);
    }
}