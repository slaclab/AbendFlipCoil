//asyn_read_write.cpp

#include "flipcoil_util.h"
#include "flipcoil.h"
/*
 * Overridden method for writeInt32 from the asynDriver, we overwrite the method because 
 * this method is used to communicate with the voltmeter as well, or at least act as the interface 
 * between the epics records seen by the operators and the voltmeter 
 */
asynStatus FlipCoilDriver::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
  asynStatus status = setIntegerParam(pasynUser->reason, value);
  callParamCallbacks();
  return asynSuccess;
}
