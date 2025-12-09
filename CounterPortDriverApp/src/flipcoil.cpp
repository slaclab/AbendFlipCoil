//FlipCoil.cpp

#include "flipcoil.h"
#include "flipcoil_util.h"
#include <unistd.h>

void flipCoilTask(void *driverPointer);
FlipCoilDriver* FlipCoilDriver::port_driver = nullptr;

FlipCoilDriver::FlipCoilDriver(const char *portName, const char* udp, int addr) : asynPortDriver(
    portName,
    1,
    asynFloat64Mask | asynInt32Mask | asynOctetMask | asynDrvUserMask,
    asynFloat64Mask | asynInt32Mask,
    0,
    1,
    0,
    0)
{

  asynStatus status = pasynOctetSyncIO->connect(udp, addr, &pasynUserPort, 0);
  pasynOctetSyncIO->setInputEos(pasynUserPort, "\r\n", 2);
  pasynOctetSyncIO->setOutputEos(pasynUserPort, "\r\n", 2);
  
  if (status != asynSuccess)
  {
    printf("\nFailed to connect over udp port %s addr %d\n", udp, addr);
    return;
  }
  //Doing a fun thing where im just trying to readwrite fun 

  int temp;
  asynStatus test;
  sleep(5);
  test = pasynOctetSyncIO->flush(pasynUserPort);
  if (test != asynSuccess)
  {
    printf("\n\nFailed to flushed the buffer");
    printf("\n\nError message? %s\n", pasynUserPort->errorMessage);
  }
  size_t nBytesOut, nBytesIn;
  int eomReason;
  const char * buffer = "ID?\r\n";
  test = pasynOctetSyncIO->writeRead(pasynUserPort, buffer, strlen(buffer), cmdBuffer, 256, 5.0, &nBytesOut, &nBytesIn, &eomReason);

  if (test != asynSuccess)
  {
    printf("\n\n The test write read thing didn't work and who knows why\n\n");
    printf("\n\nBytes out %ld, bytes in %ld", nBytesOut, nBytesIn);
    printf("\n\neomReason %d", eomReason);
    printf("\n\nError message? %s\n", pasynUserPort->errorMessage);
    printf("\nBuffer:%s\n", buffer);
    printf("\nCommand Buffer: %s\n", cmdBuffer);
    printf("\nTest variable: %d\n", test);
  }

  createParam(P_FlipCoilString, asynParamFloat64, &P_FlipCoil);
  createParam(P_TrigSglString, asynParamFloat64, &P_TrigSgl);
  createParam(P_BeepString, asynParamFloat64, &P_Beep);
  createParam(P_MemModeString, asynParamInt32, &P_MemMode);
  createParam(P_GetMemString, asynParamInt32, &P_GetMem);
  createParam(P_TrigSmplString, asynParamInt32, &P_NumSamples);

  //status = (asynStatus)(epicsThreadCreate("LujkoFlipCoilTask", epicsThreadPriorityMedium, epicsThreadGetStackSize(epicsThreadStackMedium), (EPICSTHREADFUNC)::flipCoilTask, this) == NULL);
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
  
  sleep(1);
  while(1)
  {
  
    //blmeasbl measurement

    //coilmeasvt
    float vt_pos[NUM_MEASUREMENTS];
    float vt_neg[NUM_MEASUREMENTS];
    float vt_avg[NUM_MEASUREMENTS]; // (pos - -neg) / 2

    //vector<float> coil_samples;
    double variable;
  
    for(int i = 0; i < NUM_MEASUREMENTS; i++)
    {
      vector<float> coil_samples;
      double prev = -3;
      int x = 0;

      //
      getDoubleParam(P_FlipCoil, &prev);
      getDoubleParam(P_FlipCoil, &variable);
      
      while (prev * variable > 0)
      {
        prev = variable;
        getDoubleParam(P_FlipCoil, &variable);
      }
      //As soon as we break this loop we're finding the bits we're trying to integrate 
      vector<float> pos_samples;
      vector<float> neg_samples;
      int crossings = 0;
      while (crossings < 3)
      {
        
        getDoubleParam(P_FlipCoil, &variable);
        if(variable == prev)
        {
          continue;
        }
        if(variable * prev < 0)
        {
          crossings += 1;
        }
        if (crossings == 3)
        {
          printf("Completed sine wave stored\n\n");
          break;
        }
        if (variable < 0)
        {
          neg_samples.push_back(-1 * variable);
        }
        else 
        {
          pos_samples.push_back(variable);
        }
        prev = variable;

      }
      //Then the time integral is calculated within coil_samples coilintpeak
      float pos_peak = coilIntPeak(COIL_DELTA, pos_samples);
      
      //Third a a time integral of the negative samples is calculated coilintpeak
      float neg_peak = -1 * coilIntPeak(COIL_DELTA, neg_samples);
    
      //Check for forward and reverse half cycles, takes results from coilintpeak 
      //TODO Figure out why this if statement is necessary
      printf("Calculated Pos Peak: %f Calculated Neg Peak: %f\n\n", pos_peak, neg_peak);
      /**
      if (neg_peak * pos_peak >= 0)
      {
        printf("Problems, didn't receive coil voltages most likely");
        throw runtime_error("Neg_peak * pos_peak returned a positive number or was zero, should be impossible");
      }
      **/
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
    Avg_Int.store(avg);
    printf("\n\n\nAveraged integral %g", avg);
    //Finding standard deviation
    sum = 0;
    for(int k = 0; k < NUM_MEASUREMENTS; k++)
    {
      sum += pow((vt_avg[k] - avg), 2);
    }
    float std_dev = sqrt(sum / NUM_MEASUREMENTS);
  }
}

FlipCoilDriver* FlipCoilDriver::getPortDriver()
{
  return FlipCoilDriver::port_driver;
}

void FlipCoilDriver::setPortDriver(FlipCoilDriver* portDriver)
{
  FlipCoilDriver::port_driver = portDriver;
}

extern "C" {
  int FlipCoilDriverConfigure(const char* portName, const char* udp, int addr) {
    printf("Trying to initialize the port driver");
    FlipCoilDriver* temp = new FlipCoilDriver(portName, udp, addr);
    FlipCoilDriver::setPortDriver(temp);
    return asynSuccess;
  }
  static const iocshArg FlipCoilArg0 ={"portName", iocshArgString};
  static const iocshArg FlipCoilArg1 ={"udp", iocshArgString};
  static const iocshArg FlipCoilArg2 ={"addr", iocshArgInt};
  static const iocshArg * const FlipCoilArgs[] = {&FlipCoilArg0, &FlipCoilArg1, &FlipCoilArg2};
  static const iocshFuncDef FlipCoilFuncDef = {"FlipCoilDriverConfigure", 3, FlipCoilArgs};
  static void FlipCoilCallFunc(const iocshArgBuf *args)
  {
    printf("Do my arguments not exist");
    FlipCoilDriverConfigure(args[0].sval, args[1].sval, args[2].ival);
  }
  void FlipCoilDriverRegister(void) {
    printf("Successfully registers the driver call function\n\n");
    iocshRegister(&FlipCoilFuncDef, FlipCoilCallFunc);
  }
  epicsExportRegistrar(FlipCoilDriverRegister);
}

static void integralCalc(void)
{
  FlipCoilDriver* driver = FlipCoilDriver::getPortDriver();
  printf("\nCalculated integral: %f\n", driver->Avg_Int.load());
  return;
}
static const iocshFuncDef integralCalcDef = {"integralCalc", 0};
static void integralCallFunc(const iocshArgBuf *args)
{
  integralCalc();
}
void integralCalcRegister(void)
{
  iocshRegister(&integralCalcDef, integralCallFunc);
}
extern "C"
{
  epicsExportRegistrar(integralCalcRegister);
}
