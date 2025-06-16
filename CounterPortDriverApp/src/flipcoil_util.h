//flipcoil_util.h

#include <iostream>
#include <cstdint>
#include <memory>
#include <functional>
#include <map>

#include <iocsh.h>
#include <epicsExport.h>
#include <epicsThread.h>
#include <epicsEvent.h>
#include <epicsTimer.h>
#include <epicsTypes.h>
#include <asynPortDriver.h>
#include <vector>
#include <cmath>

using namespace std;

//Function for calculating the time integral of the positive peak returns a float
//TODO figure out if i can get rid of sample aparameters by using a vector
void peakIsolator(int& start, int& end,  int&peak, vector<float> samples);
float coilIntPeak(int samples, float coil_delta, vector<float> voltage_samples); 
void sineWaveTester(vector<float>& coil_samples);
