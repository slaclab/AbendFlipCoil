//FlipCoil.cpp

#include "flipcoil.h"
#include "flipcoil_util.h"
#include <unistd.h>

void flipCoilTask(void *driverPointer);

FlipCoilDriver::FlipCoilDriver(const char *portName): asynPortDriver(
    portName,
    1,
    asynFloat64Mask,
    asynFloat64Mask,
    0,
    1,
    0,
    0
    )
{
  createParam(P_FlipCoilString, asynParamFloat64, &P_FlipCoil);
  asynStatus status;
  status = (asynStatus)(epicsThreadCreate("LujkoFlipCoilTask", epicsThreadPriorityMedium, epicsThreadGetStackSize(epicsThreadStackMedium), (EPICSTHREADFUNC)::flipCoilTask, this) == NULL);
  if (status)
  {
    printf("Thread shot itself for some reason");
    return;
  }
}

void flipCoilTask(void *driverPointer)
{
  FlipCoilDriver *pPvt = (FlipCoilDriver *) driverPointer;
  pPvt->flipCoilTask();
}

void FlipCoilDriver::flipCoilTask(void)
{
  
  double variable;
  vector<float> coil_samples;
  sleep(10);
  double prev = -3;
  int x = 0;
  while (x < 1000)
  {
    
    //printf("Things");
    getDoubleParam(P_FlipCoil, &variable);
    if (variable != prev)
    {
      coil_samples.push_back(variable);
      prev = variable;
      printf("Retrieved variable is %f\n", variable);
      x += 1;
    }
    //coil_samples.push_back(variable);
  }
  
  //blmeasbl measurement

  //coilmeasvt
  float vt_pos[NUM_MEASUREMENTS];
  float vt_neg[NUM_MEASUREMENTS];
  float vt_avg[NUM_MEASUREMENTS]; // (pos - -neg) / 2

  //vector<float> coil_samples;
  float neg_peak = -1;
  float pos_peak = 1;

  for(int i = 0; i < NUM_MEASUREMENTS; i++)
  {
    //First coil_samples is filled with the values from a sine wave
    sineWaveTester(coil_samples);
    /**
    for(float val : coil_samples)
    {
      printf("%g\n", val);
    }
    **/
    //Then the time integral is calculated within coil_samples coilintpeak
    //TODO: Comment the line back in
    float pos_peak = coilIntPeak(COIL_DELTA, coil_samples);
    
    for (float& val: coil_samples)
    {
      //Flip values around the x axis to calculate the negative peak
      val *= -1;
    }
    //Third a a time integral of the negative samples is calculated coilintpeak
    //TODO: Comment the line under this back in figure out why it won't compile
    float neg_peak = -1 * coilIntPeak(COIL_DELTA, coil_samples);
    
    //Check for forward and reverse half cycles, takes results from coilintpeak 
    if (neg_peak * pos_peak >= 0)
    {
      printf("Problems, didn't receive coil voltages most likely");
      throw runtime_error("Neg_peak * pos_peak returned a positive number or was zero, should be impossible");
    }
    if (pos_peak > 0 )
    {
      vt_pos[i] = pos_peak;
      vt_neg[i] = neg_peak;
    }
    else 
    {
      vt_pos[i] = neg_peak;
      vt_neg[i] = pos_peak;
    }
    vt_avg[i] = (vt_pos[i] - vt_neg[i]) / 2;
    //printf("Integrated calculation, %g\n", vt_avg[i]);

  }
  
  //Finding the mean of integrated voltage values
  float sum = 0;
  for(int j = 0; j < NUM_MEASUREMENTS; j++)
  {
    sum += vt_avg[j];
  }

  //TODO Pass this by reference? or make the task return this, im not sure what it's used for
  float avg = sum / NUM_MEASUREMENTS;
  printf("\n\n\nAveraged integral %g", avg);
  //Finding standard deviation
  sum = 0;
  for(int k = 0; k < NUM_MEASUREMENTS; k++)
  {
    sum += pow((vt_avg[k] - avg), 2);
  }
  float std_dev = sqrt(sum / NUM_MEASUREMENTS);
}


extern "C" {
  int FlipCoilDriverConfigure(const char* portName) {
    new FlipCoilDriver(portName);
    return asynSuccess;
  }
  static const iocshArg FlipCoilArg0 ={"portName", iocshArgString};
  static const iocshArg * const FlipCoilArgs[] = {&FlipCoilArg0};
  static const iocshFuncDef FlipCoilFuncDef = {"FlipCoilDriverConfigure", 1, FlipCoilArgs};
  static void FlipCoilCallFunc(const iocshArgBuf *args)
  {
    FlipCoilDriverConfigure(args[0].sval);
  }
  void FlipCoilDriverRegister(void) {
    iocshRegister(&FlipCoilFuncDef, FlipCoilCallFunc);
  }
  epicsExportRegistrar(FlipCoilDriverRegister);
}

