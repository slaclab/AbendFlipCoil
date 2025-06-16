//flipcoil.h

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

using namespace std;

#define P_FlipCoilString "SCOPE_FLIPCOIL"
#define NUM_MEASUREMENTS 4
#define COIL_SAMPLES 1000
#define COIL_DELTA 0.019
class FlipCoilDriver : public asynPortDriver {
  public:
    FlipCoilDriver(const char *portName);
    void flipCoilTask(void);
  protected:
    int P_FlipCoil;
};
