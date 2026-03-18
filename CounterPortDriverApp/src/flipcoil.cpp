//FlipCoil.cpp

#include "flipcoil.h"
#include "flipcoil_util.h"
#include <unistd.h>

void multimeterTask(void *driverPointer);
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
  num_measurements = 1;
  
  if (status != asynSuccess)
  {
    printf("\nFailed to connect over udp port %s addr %d\n", udp, addr);
    return;
  }

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
  //_writeRead("CLEAR");

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
  createParam(P_MemModeString, asynParamInt32, &P_MemMode);
  createParam(P_GetMemString, asynParamInt32, &P_GetMem);
  createParam(P_TrigSglString, asynParamFloat64, &P_TrigSgl);
  createParam(P_BeepString, asynParamFloat64, &P_Beep);
  createParam(P_TrigSmplString, asynParamInt32, &P_NumSamples);
  createParam(P_RmemString, asynParamFloat64, &P_Rmem);
  createParam(P_TimerGapString, asynParamInt32, &P_TimerGap);
  createParam(P_RepeatString, asynParamInt32, &P_NumMeasurements);
  createParam(P_WaveformString, asynParamInt32Array, &P_Waveform);


  status = (asynStatus)(epicsThreadCreate("LujkoMultimeterTask", epicsThreadPriorityMedium, epicsThreadGetStackSize(epicsThreadStackMedium), (EPICSTHREADFUNC)::multimeterTask, this) == NULL);
  if (status)
  {
    printf("Thread shot itself for some reason");
    return;
  }
}

void multimeterTask(void *driverPointer)
{
  FlipCoilDriver *pPvt = (FlipCoilDriver *) driverPointer;
  //pPvt->flipCoilTask();
  pPvt->multimeterTask();
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
