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
#define P_WaveformString "WAVEFORM"
#define P_IntegralAvgString "INTEGRAL_AVG"
#define P_IntegralNegString "INTEGRAL_NEG"
#define P_IntegralPosString "INTEGRAL_POS"
#define P_ResetString "RESET"



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
    int P_MemMode; //Covered  TODO:Maybe remove this, think about if user should have access to this?
    int P_GetMem; //Cpvered 
    int P_TrigSgl; //Covered 
    int P_Beep; //Covered 
    int P_NumSamples; //Covered 
    int P_Rmem; //Covered 
    int P_TimerGap; //Covered 
    int P_NumMeasurements; //Covered 
    int P_Waveform; //Covered
    int P_IntegralAvg;
    int P_IntegralPos;
    int P_IntegralNeg;
    int P_Reset;
    


    int num_samples = 170; //Number of samples taken per integal calculation
    int num_measurements = 1; //Amount of times to repeat integrap calculation
    int time_delay = 1; // Delay between each sample taken during integral calculation given in seconds
    int hold_time = num_samples * time_delay; //We must hold for a certain amount of itme before sending commands to multimet to not crash it 
    vector<float> vt_pos_mult;
    vector<float> vt_neg_mult;
    vector<float> vt_avg_mult;
    
  private:
    static FlipCoilDriver* port_driver;
    asynUser *pasynUserPort;
    char cmdBuffer[CMD_BUFFER_SIZE];
    char sendBuffer[SEND_BUFFER_SIZE];
};
