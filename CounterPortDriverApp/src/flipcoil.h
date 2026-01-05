//flipcoil.h

#include <iostream>
#include <cstdint>
#include <memory>
#include <functional>
#include <map>
#include <string>

#include <iocsh.h>
#include <string.h>
#include <mutex>
#include <atomic>
#include <epicsExport.h>
#include <epicsThread.h>
#include <epicsEvent.h>
#include <epicsTimer.h>
#include <epicsTypes.h>
#include <asynPortDriver.h>
#include <asynOctetSyncIO.h>

using namespace std;

#define P_FlipCoilString "SINE_WAVE"

#define P_TrigSglString "TRIG_SGL"
#define P_MemModeString "MEM_MODE"
#define P_GetMemString "GET_MEM"
#define P_TrigSmplString "NUM_SAMPLES"



#define P_TrigModeString "TRIG_MODE"
#define P_TrigSlopeString "TRIG_SLOPE"
#define P_TrigLevelString "TRIG_LEVEL"
#define P_BeepString "BEEP"



#define NUM_MEASUREMENTS 4
#define COIL_SAMPLES 100
#define COIL_DELTA 0.019
class FlipCoilDriver : public asynPortDriver {
  public:
    FlipCoilDriver(const char *portName, const char *udp, int addr);
    void flipCoilTask(void);
    void multimeterTask(void);
    static FlipCoilDriver* getPortDriver();
    static void setPortDriver(FlipCoilDriver* portDriver);
    atomic<float> Avg_Int;
    atomic<float> Avg_Int_Mult;
    virtual asynStatus _writeRead(const char* buffer);
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
    virtual asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);
    //virtual asynStatus writeOctet(asynUser *pasynUser, const char *buffer, size_t nBytes, size_t *nActual);

  protected:
    //mutex m;
    //float Avg_Int;
    float Std_Dev;
    int P_FlipCoil;
    int P_MemMode;
    int P_GetMem;
    int P_TrigSgl;
    int P_Beep;
    int P_NumSamples;

    int num_samples;
    vector<float> vt_pos_mult;
    vector<float> vt_neg_mult;
    vector<float> vt_avg_mult;
    
  private:
    static FlipCoilDriver* port_driver;
    asynUser *pasynUserPort;
    char cmdBuffer[512];
    char sendBuffer[256];
};
