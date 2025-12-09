//asyn_read_write.cpp

#include "flipcoil_util.h"
#include "flipcoil.h"
//FlipCoilDriver* FlipCoilDriver::_write = nullptr;
asynStatus FlipCoilDriver::_writeRead(const char* buffer)
{
  asynStatus status = asynSuccess; 
  int test;
  size_t nBytesOut, nBytesIn;
  int eomReason;
  test = pasynOctetSyncIO->writeRead(pasynUserPort, buffer, strlen(buffer), cmdBuffer, 256, 5.0, &nBytesOut, &nBytesIn, &eomReason);
   //printf("%s::%s: status=%d, buffer=%s, nbytesTrans=%d\n", driverName, 
   //    functionName, status, buffer, (int)nbytesTransfered);
  if (nBytesIn > 0)
  {
    printf("\nWe got stuff: %s", cmdBuffer);
  }
   //if (status) {
   //    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
   //      "%s::%s: Error: buffer=%s, status=%d, nbytesTransfered=%d\n",
   //      driverName, functionName, buffer, status, (int)nbytesTransfered);
   //}
  return(status);
}

/*
 * Overridden method for writeInt32 from the asynDriver, we overwrite the method because 
 * this method is used to communicate with the voltmeter as well, or at least act as the interface 
 * between the epics records seen by the operators and the voltmeter 
 */
asynStatus FlipCoilDriver::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
  int parameter = pasynUser->reason;
  sprintf(sendBuffer, "");

  if(parameter == P_MemMode)
  {
    switch (value)
    {
      case 0:
        sprintf(sendBuffer, "MEM OFF\r\n");
        break;
      case 1:
        sprintf(sendBuffer, "MEM LIFO\r\n");
        break;
      case 2:
        sprintf(sendBuffer, "MEM FIFO\r\n");
        break;
    }
  }
  else if(parameter == P_GetMem)
  {
    sprintf(sendBuffer, "RMEM 1, %d\r\n", num_samples);
  }
  else if(parameter == P_NumSamples)
  {
    num_samples = value;
    sprintf(sendBuffer, "NRDGS %d, AUTO\r\n", num_samples);
  }

  if(strlen(sendBuffer) > 0)
  {
    _writeRead(sendBuffer);
  }
  
  asynStatus status = setIntegerParam(pasynUser->reason, value);
  callParamCallbacks();
  return asynSuccess;
}

asynStatus FlipCoilDriver::readInt32(asynUser *pasynUser, epicsInt32 *value)
{
  int parameter = pasynUser->reason;
  asynStatus status = getIntegerParam(pasynUser->reason, value);
  callParamCallbacks();
  return asynSuccess;

}

asynStatus FlipCoilDriver::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
  int parameter = pasynUser->reason;
  sprintf(sendBuffer, "");

  if (parameter == P_Beep)
  {
    sprintf(sendBuffer, "BEEP\r\n");
  }
  else if (parameter == P_TrigSgl)
  {
    sprintf(sendBuffer, "TRIG SGL\r\n");
  }

  if(strlen(sendBuffer) > 0)
  {
    _writeRead(sendBuffer);
  }

  asynStatus status = setDoubleParam(pasynUser->reason, value);
  callParamCallbacks();
  return asynSuccess;
}

asynStatus FlipCoilDriver::readFloat64(asynUser *pasynUser, epicsFloat64 *value)
{
  int parameter = pasynUser->reason;
  asynStatus status = getDoubleParam(pasynUser->reason, value);
  callParamCallbacks();
  return asynSuccess;

}
