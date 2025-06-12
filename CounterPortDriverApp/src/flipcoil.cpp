//FlipCoil.cpp

#include "flipcoil.h"
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
  status = (asynStatus)(epicsThreadCreate("LujkoFlipCoilTask", epicsThreadPriorityMedium, epicsThreadGetStackSize(epicsThreadStackMedium), (EPICSTHREADFUNC)::getterTask, this) == NULL);
  if (status)
  {
    printf("Thread shot itself for some reason");
    return;
  }
}

void flipCoilTask(void *driverPointer)
{
  FlipCoilDriver *pPvt = (GetterDriver *) driverPointer;
  pPvt->FlipCoilTask();
}

void FlipCoilDriver::flipCoilTask(void)
{
  double variable;
  for(int x = 0; x <= 100; x++)
  {
    sleep(1);
    //printf("Things");
    getDoubleParam(P_FlipCoil, &variable);
    printf("Retrieved variable is %f\n", variable);
  }
}


extern "C" {
  int FlipCoilDriverConfigure(const char* portName) {
    new FlipCoilDriver(portName);
    return asynSuccess;
  }
  static const iocshArg FlipCoilArg0 ={"portName", iocshArgString};
  static const iocshArg * const FlipCoilArgs[] = {&getterArg0};
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

