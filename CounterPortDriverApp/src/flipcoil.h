//flipcoil.h

#include <iostream>
#include <cstdint>
#include <memory>
#include <functional>
#include <map>

#include <iocsh.h>
#include <mutex>
#include <atomic>
#include <epicsExport.h>
#include <epicsThread.h>
#include <epicsEvent.h>
#include <epicsTimer.h>
#include <epicsTypes.h>
#include <asynPortDriver.h>

using namespace std;

#define P_FlipCoilString "SINE_WAVE"
#define NUM_MEASUREMENTS 4
#define COIL_SAMPLES 100
#define COIL_DELTA 0.019
class FlipCoilDriver : public asynPortDriver {
  public:
    FlipCoilDriver(const char *portName);
    void flipCoilTask(void);
    static FlipCoilDriver* getPortDriver();
    static void setPortDriver(FlipCoilDriver* portDriver);
    atomic<float> Avg_Int;

    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

  protected:
    //mutex m;
    //float Avg_Int;
    float Std_Dev;
    int P_FlipCoil;
    
  private:
    static FlipCoilDriver* port_driver;
};
