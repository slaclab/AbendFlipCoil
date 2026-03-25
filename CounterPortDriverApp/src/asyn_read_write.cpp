//asyn_read_write.cpp

#include "flipcoil_util.h"
#include "flipcoil.h"
#include <unistd.h>
//FlipCoilDriver* FlipCoilDriver::_write = nullptr;
asynStatus FlipCoilDriver::_writeRead(const char* buffer)
{
  asynStatus status = asynSuccess; 
  int test;
  size_t nBytesOut, nBytesIn;
  int eomReason;
  test = pasynOctetSyncIO->writeRead(pasynUserPort, buffer, strlen(buffer), cmdBuffer, CMD_BUFFER_SIZE, 5.0, &nBytesOut, &nBytesIn, &eomReason);
  if (nBytesIn > 0)
  {
    asynPrint(pasynUserPort, ASYN_TRACE_FLOW, "Multimeter sent %s\n", cmdBuffer);
  }
   //if (status) {
   //    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
   //      "%s::%s: Error: buffer=%s, status=%d, nbytesTransfered=%d\n",
   //      driverName, functionName, buffer, status, (int)nbytesTransfered);
   //}
  printf("\neomReason: %d", eomReason);
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
  else if(parameter == P_NumSamples)
  {
    if(value > 0)
    {
      num_samples = value;
      hold_time = num_samples * time_delay;
    }

  }
  else if(parameter == P_NumMeasurements)
  {
    if(value > 0)
    {
      num_measurements = value;
    }
  }
  else if(parameter == P_TimerGap)
  {
    if(time_delay > 0)
    {
      time_delay = value;
      hold_time = num_samples * time_delay;
    }
  }
 
  else if(parameter == P_GetMem)
  {
    _writeRead("PRESET NORM\r\n");
    _writeRead("END ALWAYS\r\n");
    _writeRead("ID?");
    _writeRead("TRIG HOLD\r\n");
    _writeRead("MEM LIFO\r\n");
    _writeRead("NPLC 10\r\n");
    sprintf(sendBuffer, "NRDGS %d, TIMER\r\n", num_samples);
    _writeRead(sendBuffer);
    sprintf(sendBuffer, "TIMER %d\r\n", time_delay);
    _writeRead(sendBuffer);
    _writeRead("MCOUNT?\r\n");
    _writeRead("TRIG SGL\r\n");
    sleep(time_delay * num_samples);
    printf("\n\nAfter sleep, should try rmem");
    _writeRead("MCOUNT?\r\n");
    _writeRead("ERRSTR?\r\n");
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

  if (parameter == P_Rmem)
  {
    size_t nBytesOut, nBytesIn;
    int eomReason;
    printf("\n\nThe value we wrote was %f", value);
    pasynOctetSyncIO->flush(pasynUserPort);
    sprintf(sendBuffer, "RMEM 1,%d;\r\n", num_samples);
    pasynOctetSyncIO->write(pasynUserPort, sendBuffer, 11, 5.0, &nBytesOut); //Command that tells the multimeter to send it's readings over gpib to us 

    for (int i = 0; i < num_samples; i++)
    {
      asynStatus status = pasynOctetSyncIO->read(pasynUserPort, cmdBuffer, CMD_BUFFER_SIZE, 5.0, &nBytesIn, &eomReason);
      printf("\nBuffer: %s, eomReason %d, nBytesIn, %d", cmdBuffer, eomReason, nBytesIn);

    }
    pasynOctetSyncIO->write(pasynUserPort, "MEM FIFO\r\n", 8, 5.0, &nBytesOut);
  }

  if (parameter == P_Beep)
  {
    sprintf(sendBuffer, "BEEP\r\n");
  }
  else if (parameter == P_TrigSgl)
  {
    _writeRead("MCOUNT?\r\n");
    _writeRead("TRIG SGL\r\n");
    usleep(1000000* time_delay * num_samples);
    _writeRead("MCOUNT?\r\n");
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
