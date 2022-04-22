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
#include <fstream>
#include <string>
#include <cstring>
#include <iomanip>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include "ADC.h"
#include "GaussNewton3.h"
#include "qiagen.h"

#define DEVICE_ID 0x48
#define TEMP_ID 0x49
#define HEATER_PIN 21
#define FAN_PIN 22
#define SMOOTHING 125
#define CONVERGENCE_THRESHOLD 1
#define RT_LENGTH 600
#define CALIBRATION_MIN 400

// Currently unused. We'll see if we need it.
#define CALIBRATION_MAX 2200

const double max_vals[3] = {500, 60, 10};
const double min_vals[3] = {-500, -60, -10};
const int iter_thresh = 24;

using namespace std;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;


void sampletriggered(void);
void readPCR(void);
void setupQiagen(void);
void openFiles(void);
bool isInBounds();
void setupADC(void);
void writetograph(void);
double heatquery(double temp);
void waittotemp(double temp);




//Declare reading struct.
struct reading
{
    int timestamp;
    int voltage;
};

struct tempquery
{
    double temp;
    double fluoresence;
};

//Declare global variables/variables that need to persist in memory. There might be a better way, but with threading this is the best I've come up with.
adc D2(DEVICE_ID, 0x8000, 0x7FFF);
adc TEMP(TEMP_ID, 0x8000, 0x7FFF);
vector<reading> data;
vector<double> tempkey;
vector<double> fluorkey;
vector<double> x;
vector<double> y;
bool RTdone = false;
vector<double> xderivs;
vector<double> derivs = {0};
deque<double> yaverage(SMOOTHING,0);
deque<double> derivaverage(SMOOTHING,0);
double yout;
fstream coeff_out;
fstream pcr_out;
int cycles = 0;

FILE *output;
int datapoints = 0;
double run_start = 0;
qiagen sens1("/dev/ttyUSB0");
qiagen sens2("/dev/ttyUSB1");

// Generates the vector of heats and corresponding fluorescences. Data acquired from a melt test. 
void heatgen(const double coeff[3], bool trigger)
{
    int startindex = 0;
    if(trigger == true)
    {
        startindex = 74;
    }
    vector<double> temps = {55,55.5,55.83333333,56.16666667,56.5,56.83333333,57.16666667,57.5,57.83333333,58.16666667,58.5,58.83333333,59.16666667,59.5,59.83333333,60.16666667,60.5,60.83333333,61.16666667,61.5,61.83333333,62.16666667,62.5,62.83333333,63.16666667,63.5,63.83333333,64.16666667,64.5,64.83333333,65.16666667,65.5,65.83333333,66.16666667,66.5,66.83333333,67.16666667,67.5,67.83333333,68.16666667,68.5,68.83333333,69.16666667,69.5,69.83333333,70.16666667,70.5,70.83333333,71.16666667,71.5,71.83333333,72.16666667,72.5,72.83333333,73.16666667,73.5,73.83333333,74.16666667,74.5,74.83333333,75.16666667,75.5,75.83333333,76.16666667,76.5,76.83333333,77.16666667,77.5,77.83333333,78.16666667,78.5,78.83333333,79.16666667,79.5,79.83333333,80.16666667,80.5,80.83333333,81.16666667,81.5,81.83333333,82.16666667,82.5,82.83333333,83.16666667,83.5,83.83333333,84.16666667,84.5,84.83333333,85.16666667,85.5,85.83333333,86.16666667,86.5,86.83333333,87.16666667,87.5,87.83333333,88.16666667,88.5,88.83333333,89.16666667,89.5,89.83333333,90.16666667,90.5,90.83333333,91.16666667,91.5,91.83333333,92.16666667,92.5,92.83333333,93.16666667,93.5,93.83333333,94.16666667,94.5,95,95.5,96};
    vector<double> zscores = {-1.615167831,-1.544943143,-1.498126685,-1.451310225,-1.404493766,-1.357677308,-1.310860848,-1.26404439,-1.217227931,-1.170411471,-1.123595013,-1.076778555,-1.029962095,-0.983145636,-0.936329178,-0.889512718,-0.84269626,-0.795879801,-0.749063342,-0.702246883,-0.655430425,-0.608613965,-0.561797507,-0.514981048,-0.468164588,-0.42134813,-0.374531671,-0.327715212,-0.280898753,-0.234082295,-0.187265835,-0.140449377,-0.093632918,-0.046816458,0,0.046816458,0.093632918,0.140449377,0.187265835,0.234082295,0.280898753,0.327715212,0.374531671,0.42134813,0.468164588,0.514981048,0.561797507,0.608613965,0.655430425,0.702246883,0.749063342,0.795879801,0.84269626,0.889512718,0.936329178,0.983145636,1.029962095,1.076778555,1.123595013,1.170411471,1.217227931,1.26404439,1.310860848,1.357677308,1.404493766,1.451310225,1.498126685,1.544943143,1.591759601,1.638576061,1.68539252,1.732208978,1.779025438,1.825841896,-2.283747771,-2.204997846,-2.126247924,-2.047498001,-1.968748077,-1.889998154,-1.811248232,-1.732498307,-1.653748385,-1.574998463,-1.496248538,-1.417498616,-1.338748693,-1.259998769,-1.181248846,-1.102498924,-1.023748999,-0.944999077,-0.866249155,-0.78749923,-0.708749308,-0.629999386,-0.551249461,-0.472499539,-0.393749616,-0.314999692,-0.236249769,-0.157499847,-0.078749922,0,0.078749922,0.157499847,0.236249769,0.314999692,0.393749616,0.472499539,0.551249461,0.629999386,0.708749308,0.78749923,0.866249155,0.944999077,1.023748999,1.102498924,1.181248846,1.299373731,1.417498616,1.5356235};

    int j = startindex;
    int i = 0;
    while(i < y.size() && j < zscores.size())
    {
        while((x[i]-coeff[1])/abs(coeff[2]) < zscores[j] && i < y.size())
        {
            fluorkey.push_back(y[i]);
            tempkey.push_back(temps[j]);
            i++;
        }
        cout << "Temp " << temps[j] << " complete." << endl;
        j++;
    }
}

double heatquery(double temp)
{
    for(int i = 0; i < fluorkey.size(); i++)
    {
        if(tempkey[i] > temp)
        {
            return fluorkey[i];
        }
    }
    cout << "Heatquery failed." << endl;
    return -1;
}

double fluorquery(double fluor)
{
    for(int i = 0; i < fluorkey.size(); i++)
    {
        if(fluorkey[i] > fluor)
        {
            return tempkey[i];
        }
    }
    // cout << "Fluorquery failed." << endl;
    return -1;
}

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
    xderivs.clear();
    derivs.clear();
}

// Shifts the shift between heating and cooling. Takes the current mode as an argument, returns the new one.
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
        digitalWrite(FAN_PIN,LOW);
        readPCR();
        digitalWrite(HEATER_PIN, HIGH);
        return true;
    }
}

// Delays to a point based on the fitted coefficients and the value of the derivative, depending on a fraction of peak threshold. Returns the time in milliseconds after program start.
int delaytopointBill(const double coeff[3], double thresh)
{
    while(abs(derivs[derivs.size()-1]) > abs(coeff[0] * thresh))
    {
        writetograph();
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

void writetograph()
{
    piLock(2);
    fstream fs;
    fs.open("phpoutput.txt", fstream::out | fstream::trunc);
    float voltage = (TEMP.getreading()*4.096)/32767.0;
    float temperature = (voltage - 1.25)/0.005;
    double lastderiv = derivs[derivs.size()-1];
    fs << y[y.size()-1] << endl;
    if (RTdone)
    {
        double tempfromfluor = fluorquery(y[y.size() - 1]);
        if(tempfromfluor > 0)
        {
            fs << tempfromfluor << endl;
        }
        else
        {
            fs << 75 << endl;
        }
    }
    else
    {
        double tempfromfluor = 60;
        fs << tempfromfluor << endl;
    }
    fs << temperature << endl;
    fs.close();
    piUnlock(2);
}

void holdtemp(double temp, double time)
{
    double fluortarget = heatquery(temp);
    cout << "Falling." << endl;
    while(y[y.size()-1] > fluortarget)
    {
        writetograph();
        delay(10);
    }
    int start = 0;
    double ycurrent;
    cout << "Activate hold." << endl;
    while(start < time * 100)
    {
        ycurrent = y[y.size()-1];
        if (ycurrent < fluortarget - 20)
        {
            digitalWrite(HEATER_PIN, HIGH);
        }
        else if (ycurrent > fluortarget)
        {
            digitalWrite(HEATER_PIN, LOW);
        }
        writetograph();
        delay(10);
        start++;
    }
}

void holdtemp(double frac, double minfluor, double maxfluor, double time)
{
    double fluortarget = minfluor + (maxfluor - minfluor)*frac;
    cout << "Falling." << endl;
    while(y[y.size()-1] > fluortarget)
    {
        writetograph();
        delay(10);
    }
    int start = 0;
    double ycurrent;
    cout << "Activate hold." << endl;
    while(start < time * 100)
    {
        ycurrent = y[y.size()-1];
        if (ycurrent < fluortarget)
        {
            digitalWrite(HEATER_PIN, HIGH);
        }
        else if (ycurrent > fluortarget+20)
        {
            digitalWrite(HEATER_PIN, LOW);
        }
        writetograph();
        delay(10);
        start++;
    }

}

void waittotemp(double temp)
{
    double fluortarget = heatquery(temp);
    cout << "Falling." << endl;
    while(y[y.size()-1] > fluortarget)
    {
        writetograph();
        delay(10);
    }
}

void runRT()
{
    /*sens1.LED_off(2);
    sens1.LED_power(1,170);
    sens1.LED_on(1);*/
    bool tempflag = false;
    double temp_frac = 0.786;
    double coeff[3];
    double iter;
    double coeffprev[3];
    controltriggered();
    delay(1000);
    int sec_hold = RT_LENGTH;
    digitalWrite(HEATER_PIN, HIGH);
    digitalWrite(FAN_PIN, LOW);
    double beta0[3] = {0,0,2};
    bool past_the_hump = false;
    bool doubleheight = false;
    bool dtrigger = false;
    while(tempflag == false)
    {
        piLock(0);
        auto maxlocation = max_element(begin(derivs), end(derivs));
        beta0[0] = *maxlocation;
        beta0[1] = x[distance(derivs.begin(), maxlocation)];
        GaussNewton3(xderivs, derivs, beta0, min_vals, max_vals, coeff, &iter);
        if(coeff[1] <= x[x.size()-1])
        {
            past_the_hump = true;
        }
        else
        {
            past_the_hump = false;
        }
        piUnlock(0);
        if (iter < 24 && abs(coeff[0]) > 10 && coeff[1] > 5)  
        {
            if (past_the_hump == true && abs(coeffprev[0] - coeff[0]) < CONVERGENCE_THRESHOLD && abs(coeffprev[1] - coeff[1]) < CONVERGENCE_THRESHOLD && abs(abs(coeffprev[2]) - abs(coeff[2])) < CONVERGENCE_THRESHOLD)
            {
                cout << "Coeff: " << coeff[0] << " " << coeff[1] << " " << coeff[2] << endl;
                /*double comparative = 1000;
                int stopindex;
                int i = 1;
                while(comparative > 1 && i < derivs.size()-2)
                {
                    if(derivs[derivs.size()-i] < comparative)
                    {
                        comparative = derivs[derivs.size()-i];
                        stopindex = derivs.size()-i + 60;
                    }
                    i++;
                }*/
                delaytopointBill(coeff, 0.005);
                heatgen(coeff, dtrigger);
                if(dtrigger == false)
                {
                    controltriggered();
                    dtrigger = true;
                }
                else
                {
                    digitalWrite(HEATER_PIN, LOW);
                    tempflag = true;
                    controltriggered();
                }
                /* double maxfluor = y[y.size()-1];
                double minfluor = y[stopindex];*/
            }
            coeffprev[0] = coeff[0];
            coeffprev[1] = coeff[1];
            coeffprev[2] = coeff[2];
        }
        delay(25);
        writetograph();
    }
    RTdone = true;
    //holdtemp(65, 600);
    //holdtemp(90,60);
    /*sens1.LED_off(1);
    sens1.LED_power(1,45);*/
}

void readPCR()
{
    sens1.LED_off(2);
    sens1.LED_on(1);
    sens1.setMethod(1);
    sens1.startMethod();
    delay(500);
    double PCRread = sens1.measure();
    sens1.stopMethod();
    pcr_out << PCRread << ",";
    sens1.LED_off(1);

    sens2.LED_on(1);
    sens2.setMethod(1);
    sens2.startMethod();
    delay(500);
    PCRread = sens2.measure();
    sens2.stopMethod();
    pcr_out << PCRread << ",";
    sens2.LED_off(1); 

/*    sens2.LED_on(2);
    sens2.setMethod(3);
    sens2.startMethod();
    delay(500);
    PCRread = sens2.measure();
    sens2.stopMethod();
    pcr_out << PCRread;
    sens2.LED_off(2); */

    pcr_out << endl;
    sens1.LED_on(2);
    delay(1000);
}

void calibrategain()
{
    int basegain = 80;
    double reading;
    cout << "Calibrating gain." << endl;
    do
    {
        cout << "Testing at gain of: " << basegain << "." << endl;
        sens1.LED_off(2);
        sens1.LED_power(2,basegain);
        sens1.LED_on(2);
        delay(200);
        reading = (D2.getreading()/32768.0)*4096.0;
        cout << "Reading: " << reading << endl;
        yout = reading;
        basegain += 10;
        if(basegain > 224)
        {
            cout << "Couldn't find a suitable gain. Sample not present or illprepared." << endl;
            break;
        }
        writetograph();
    } while (reading < CALIBRATION_MIN);
}

int main()
{
    readPCR();
    string coeffstorage("tb3apr21.txt");
    string pcrstorage("tb3apr21pcr.txt");
    sens1.writeqiagen(0, {00,04});
    sens1.writeqiagen(1, {00,00});
    sens2.writeqiagen(0, {00,200});
    sens2.writeqiagen(1, {00,00});
    //sens1.LED_power(2,190);    
    sens1.LED_power(1,50);
    sens2.LED_power(2,95);
    sens2.LED_power(1,40);
    sens2.LED_off(2);
    sens2.LED_off(1);
    sens1.LED_off(1);
    sens1.setMode(0);
	sens1.setMethod(1);
    sens1.LED_on(2);
    sens2.setMode(0);
    sens2.setMethod(1);
    coeff_out.open(coeffstorage,std::ios_base::out);
    pcr_out.open(pcrstorage,std::ios_base::out);
    pcr_out << "Time    " << "FAM   " << "Cy5   " << endl;
    if(!coeff_out.is_open())
    {
        cout << "Failed to open " << coeffstorage << endl;
        return 1;
    }

    output = fopen("binaryoutput.bin","wb");
	if(output == NULL)
	{
		cout << "File failed to open." << endl;
		return -1;
	} 
    int mode = 2;
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
	D2.SetGain(1);
	D2.SetMode(0);
	D2.SetSPS(5);
	D2.SetCompPol(1);
	D2.SetCompQueue(0);
    D2.SetMultiplex(3,-1);
    TEMP.SetGain(1);
	TEMP.SetMode(0);
	TEMP.SetSPS(5);
	TEMP.SetCompPol(1);
	TEMP.SetCompQueue(0);
    TEMP.SetMultiplex(0,-1);
    
    
    double beta0[3] = {10, 10, 1};
    double lb[3] = {min_vals[0], min_vals[1], min_vals[2]};
    double ub[3] = {max_vals[0], max_vals[1], max_vals[2]};
    double coeff[3];
    double iter;
    double coeffprev[3];
    double coeffdouble[3];
    double thresh = 0.005;
    double threshcool = 0.135;
    double dthreshheat = 0.1;
    double dthreshcool = 0.5;
    coeffprev[0] = 0;
    coeffprev[1] = 0;
    coeffprev[2] = 0;
    int cutoff = 50;
    int triggertime;
    bool past_the_hump;
    delay(100);
    wiringPiISR(2,INT_EDGE_RISING,sampletriggered);
    cout << "Attempting to run RT step: " << endl;
    sens1.LED_on(2);
    D2.SetMultiplex(0,1);
    calibrategain();
    controltriggered();
    runRT();
    digitalWrite(HEATER_PIN,LOW);
    waittotemp(60);
    controltriggered();
    delay(1000);

    // Begin heating.
    digitalWrite(HEATER_PIN, HIGH);
    digitalWrite(FAN_PIN,LOW);   
    bool state = true;
    delay(100);
    while (cycles < cutoff)
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
        GaussNewton3(xderivs, derivs, beta0, lb, ub, coeff, &iter);
        if(coeff[1] < x[x.size()-1])
        {
            past_the_hump = true;
        }
        else
        {
            past_the_hump = false;
        }
        if(x[x.size()-1] > 75)
        {
            cout << "Cycled for too long, error thrown." << endl;
            digitalWrite(FAN_PIN,LOW);
            digitalWrite(HEATER_PIN,LOW);
            return 0;
        }
        piUnlock(0);
        if (iter < 24 && abs(coeff[0]) > 10 && coeff[1] > 1)  
        {
            if (past_the_hump == true && abs(coeffprev[0] - coeff[0]) < CONVERGENCE_THRESHOLD && abs(coeffprev[1] - coeff[1]) < CONVERGENCE_THRESHOLD && abs(abs(coeffprev[2]) - abs(coeff[2])) < CONVERGENCE_THRESHOLD)
            {
                cout << coeff[0] << "   " << coeff[1] << "  " << coeff[2]  << "   " << "  Iter: " << iter << endl;
                if(doublehump == false)
                {
                    if(state == true)
                    {
                        triggertime = delaytopointBill(coeff,thresh);
                    }
                    else
                    {
                        triggertime = delaytopointBill(coeff,threshcool);
                    }
                    cout << "Control triggered at " << triggertime << " milliseconds after run initiation." << endl;
                    coeff_out << triggertime << "," << coeff[0] << "," << coeff[1] << "," << coeff[2] << "," << triggertime - run_start << endl; 
                    if(state == false)
                    {
                        pcr_out << triggertime << ",";
                    }
                    state = modeshift(state);
                    controltriggered();
                    if(state == true)
                    {
                        //lb[0] = -max_vals[0];
                        //ub[0] = -min_vals[0];
                    }
                    else
                    {
                        //lb[0] = min_vals[0];
                        //ub[0] = max_vals[0];
                        cout << "Cycle " << cycles << " complete." << endl;
                        cycles++;
                    }
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
                            triggertime = delaytopointBill(coeff,thresh);
                            //lb[0] = -max_vals[0];
                            //ub[0] = -min_vals[0];
                        }
                        else
                        {
                            triggertime = delaytopointBill(coeff,threshcool);
                            //lb[0] = min_vals[0];
                            //ub[0] = max_vals[0];
                            pcr_out << triggertime << ",";
                            cycles++;
                        }
                        state = modeshift(state);
                        cout << "Cycle " << cycles << " complete." << endl;
                        dtrigger = false;
                    }
                    cout << "Control triggered at " << triggertime << " milliseconds after run initiation." << endl;
                    coeff_out << triggertime << "," << coeff[0] << "," << coeff[1] << "," << coeff[2] << "," << triggertime - run_start << endl; 
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
        writetograph();
        delay(25);
    }
    piLock(0);
    // fclose(output);
    digitalWrite(HEATER_PIN, LOW);
    digitalWrite(FAN_PIN, LOW);
    sens1.LED_off(2);
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
    double ycurrent =(now.voltage/32768.0)*4096.0;
    if(yaverage.size() >= SMOOTHING)
    { 
        yaverage.pop_front();
    }
    yaverage.push_back(ycurrent);
    double average = mean(yaverage);
    yout = average;
    y.push_back(average);
    // cout << y[y.size()-1] << endl;
    if(y.size() > SMOOTHING)
    {
        xderivs.push_back((now.timestamp - run_start)/1000.0);
        double deriv = (y[y.size() - 1] - y[y.size() - 2])*25;
        double dderiv = (y[y.size() - 1] - y[y.size() - 2])*25;
        //derivaverage.pop_front();
        //derivaverage.push_back(deriv);
        //average = mean(derivaverage);
        derivs.push_back(deriv);
    }
    reading *ptr = &now;
    fwrite(ptr, sizeof(reading), 1, output);
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
