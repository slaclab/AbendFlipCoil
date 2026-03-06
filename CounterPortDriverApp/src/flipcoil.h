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

//Strings to connect with epics pv's
//TODO: Sort by alphabetical order lmao
#define P_FlipCoilString "SINE_WAVE"
#define P_TrigSglString "TRIG_SGL"
#define P_MemModeString "MEM_MODE"
#define P_GetMemString "GET_MEM"
#define P_TrigModeString "TRIG_MODE"
#define P_TrigSlopeString "TRIG_SLOPE"
#define P_TrigLevelString "TRIG_LEVEL"
#define P_BeepString "BEEP"
#define P_RmemString "RMEM"

//Important user defined parameter PV strings
#define P_TrigSmplString "NUM_SAMPLES"
#define P_TimerGapString "TIMER_GAP"
#define P_MultiTaskString "MULTI_TASK"
#define P_RepeatString "REPEATS"



#define CMD_BUFFER_SIZE 256 //Sort of arbitrary, I just can't really think of anything larger that the multimeter would send 
#define SEND_BUFFER_SIZE 128 //Also arbitrary, larger command sequences can just be sent with multiple write commands
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

  protected:
    float Std_Dev;
    int P_FlipCoil;
    int P_MemMode;
    int P_GetMem;
    int P_TrigSgl;
    int P_Beep;
    int P_NumSamples;
    int P_Rmem;

    int num_samples;
    int num_measurements;
    vector<float> vt_pos_mult;
    vector<float> vt_neg_mult;
    vector<float> vt_avg_mult;
    
  private:
    static FlipCoilDriver* port_driver;
    asynUser *pasynUserPort;
    char cmdBuffer[CMD_BUFFER_SIZE];
    char sendBuffer[SEND_BUFFER_SIZE];
};
