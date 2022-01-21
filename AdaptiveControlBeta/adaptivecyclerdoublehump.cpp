// gaussnewtonv3.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <byteswap.h>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <deque>

#include "ADC.h"
#include "GaussNewton3.h"

#define DEVICE_ID 0x48
#define HEATER_PIN 21
#define FAN_PIN 22
#define SMOOTHING 25

using namespace std;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;


void sampletriggered(void);

//Declare reading struct.
struct reading
{
    int timestamp;
    int voltage;
};

//Declare global variables/variables that need to persist in memory. There might be a better way, but with threading this is the best I've come up with.
adc D2(DEVICE_ID, 0x8000, 0x7FFF);
vector<reading> data;
vector<double> x;
vector<double> y;
vector<double> derivs = {0};
deque<double>deriveaverage(SMOOTHING,0);

// FILE *output;
int datapoints = 0;
double run_start = 0;

//Takes a deque of doubles and returns the average.
double mean(const deque<double> queue)
{
    double sum;
    for(int i = 0; i < queue.size(); i++)
    {
        sum += queue[i];
    }
    sum = sum/queue.size();
    return sum;
}


//This function is called when the control algorithm is reset. It empties the vectors that the fitting algorithm targets. 
void controltriggered()
{
    x.clear();
    y.clear();
    derivs.clear();
}

// Shifts the mode between heating and cooling. Takes the current mode as an argument, returns the new one.
bool modeshift(bool state)
{
    if(state == true)
    {
        digitalWrite(HEATER_PIN, LOW);
        digitalWrite(FAN_PIN, HIGH);
        return false;
    }
    else
    {
        digitalWrite(HEATER_PIN, HIGH);
        digitalWrite(FAN_PIN,LOW);
        return true;
    }
}

// Delays to a point based on the fitted coefficients and the value of the derivative, depending on a fraction of peak threshold. Returns the time in milliseconds after program start.
int delaytopointBill(const double coeff[3], double thresh)
{
    while(abs(derivs[derivs.size()-1]) > abs(coeff[0] * thresh))
    {
        delay(10);
    }
    cout << "Absolute value of " << derivs[derivs.size()-1] << " is less than " << thresh*coeff[0] << endl;
    return data[data.size()-1].timestamp - data[0].timestamp;
}
// Delays to a point based on the fitted coefficients and the timestamp, depending on a standard deviation from the peak threshold. Returns the time in milliseconds after program start.
int delaytopointNick(const double coeff[3], double thresh)
{
    while(x[x.size()-1] > coeff[1] + coeff[2] * thresh)
    {
        delay(10);
    }
    return data[data.size()-1].timestamp - data[0].timestamp;
}

int main()
{
 /*   output = fopen("binaryoutput.bin","wb");
	if(output == NULL)
	{
		cout << "File failed to open." << endl;
		return -1;
	} */
    cout << "Single(1) or double(2) hump functionality?" << endl;
    int mode;
    cin >> mode;
    bool doublehump;
    if(mode == 2)
    {
        doublehump = true;
    }
    else
    {
        doublehump = false;
    }
    bool dtrigger = false;
    wiringPiSetup();
	pinMode(2,INPUT);
	pinMode(3,OUTPUT);
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    digitalWrite(HEATER_PIN, HIGH);
    digitalWrite(FAN_PIN,LOW);
    bool state = true;
	D2.SetGain(1);
	D2.SetMode(0);
	D2.SetSPS(5);
	D2.SetCompPol(1);
	D2.SetCompQueue(0);
    wiringPiISR(2,INT_EDGE_RISING,sampletriggered);
    double beta0[3] = {10, 10, 2};
    double lb[3] = {0, 0, 0};
    double ub[3] = {500, 40, 25};
    double coeff[3];
    double iter;
    double coeffprev[3];
    double coeffdouble[3];
    double thresh = 0.05;
    double dthreshheat = 0.1;
    double dthreshcool = 0.5;
    coeffprev[0] = 0;
    coeffprev[1] = 0;
    coeffprev[2] = 0;
    int cutoff = 150000;
    int triggertime;
    delay(5000);
    while (data.size() < cutoff)
    {
        piLock(0);
        if(state == true)
        {
            auto maxlocation = max_element(begin(derivs), end(derivs));
            beta0[0] = *maxlocation;
            beta0[1] = x[distance(derivs.begin(), maxlocation)];
        }
        else
        {
            auto minlocation = min_element(begin(derivs), end(derivs));
            beta0[0] = *minlocation;
            beta0[1] = x[distance(derivs.begin(), minlocation)];
        }
        GaussNewton3(x, derivs, beta0, lb, ub, coeff, &iter);
        piUnlock(0);
        if (iter < 24 && abs(coeff[0]) > 1 && coeff[1] > 1) 
        {
            if (abs(coeffdouble[0] - coeff[0]) < 0.5 && abs(coeffdouble[1] - coeff[1]) < 0.5 && abs(coeffdouble[2] - coeff[2]) < 0.5)
            {
                cout << coeff[0] << "   " << coeff[1] << "  " << coeff[2]  << "   " << "  Iter: " << iter << endl;
                if(doublehump == false)
                {
                    triggertime = delaytopointBill(coeff,thresh);
                    cout << "Control triggered at " << triggertime << " milliseconds after run initiation." << endl;
                    controltriggered();
                    if(state == true)
                    {
                        lb[0] = -100;
                        ub[0] = 0;
                    }
                    else
                    {
                        lb[0] = 0;
                        ub[0] = 100;
                    }
                    state = modeshift(state);
                }
                else
                {
                    if(dtrigger == false)
                    {
                        dtrigger = true;
                        if(state == true)
                        {
                            triggertime = delaytopointBill(coeff,dthreshheat);
                        }
                        else
                        {
                            triggertime = delaytopointBill(coeff,dthreshcool);
                        }
                    }
                    else
                    {
                        if(state == true)
                        {
                            lb[0] = -100;
                            ub[0] = 0;
                        }
                        else
                        {
                            lb[0] = 0;
                            ub[0] = 100;
                        }
                        triggertime = delaytopointBill(coeff,thresh);
                        state = modeshift(state);
                        dtrigger = false;
                    }
                    cout << "Control triggered at " << triggertime << " milliseconds after run initiation." << endl;
                    controltriggered();
                }
            }
            coeffdouble[0] = coeffprev[0];
            coeffdouble[1] = coeffprev[1];
            coeffdouble[2] = coeffprev[2];
            coeffprev[0] = coeff[0];
            coeffprev[1] = coeff[1];
            coeffprev[2] = coeff[2];
        }
        delay(50);
    }
    piLock(0);
    // fclose(output);
    digitalWrite(HEATER_PIN, LOW);
    digitalWrite(FAN_PIN, LOW);
    return 0;
}

void sampletriggered(void)
{
    piLock(0);
    auto millisecs = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    reading now;
    now.timestamp = millisecs;
    now.voltage = D2.getreading();
    data.push_back(now);
    //This line needs to be intelligently edited for multiple x resets.
    if(x.size() == 0)
    {
        run_start = now.timestamp;
    }
    x.push_back((now.timestamp - run_start)/1000.0);
    y.push_back((now.voltage/32768.0)*4096.0);
    // cout << y[y.size()-1] << endl;
    if(y.size() > 1)
    {
        double deriv = (y[y.size() - 1] - y[y.size() - 2])*25;
        deriveaverage.pop_front();
        deriveaverage.push_back(deriv);
        double average = mean(deriveaverage);
        derivs.push_back(average);
    }
    // reading *ptr = &now;
    // fwrite(ptr, sizeof(reading), 1, output);
    piUnlock(0);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
