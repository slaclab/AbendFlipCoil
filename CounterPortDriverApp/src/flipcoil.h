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

class GetterDriver : public asynPortDriver {
  public:
    GetterDriver(const char *portName);
    void getterTask(void);
  protected:
    int P_Getter;
};
