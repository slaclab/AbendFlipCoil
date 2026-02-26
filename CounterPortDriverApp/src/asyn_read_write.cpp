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
  test = pasynOctetSyncIO->writeRead(pasynUserPort, buffer, strlen(buffer), cmdBuffer, 8192, 5.0, &nBytesOut, &nBytesIn, &eomReason);
   //printf("%s::%s: status=%d, buffer=%s, nbytesTrans=%d\n", driverName, 
   //    functionName, status, buffer, (int)nbytesTransfered);
  if (nBytesIn > 0)
  {
    printf("\nWe got stuff: %s, NumBytesIn: %d", cmdBuffer, nBytesIn);
    char* token = strtok(cmdBuffer, "\r\n");
    vector<float> coil_samples;
    while (token != NULL)
    {
      coil_samples.push_back(strtof(token, nullptr));
      token = strtok(NULL, "\r\n");
    }
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
    //sprintf(sendBuffer, "RMEM 1, %d\r\n", num_samples);
    /**
    _writeRead("END ALWAYS");
    _writeRead("FUNC DCV");
    _writeRead("RANGE AUTO");
    _writeRead("TARM AUTO");
    _writeRead("TIMER 1E0");
    _writeRead("NPLC 0.02");
    _writeRead("NRDGS 1");
    _writeRead("TRIG TIMER");
    _writeRead("ARM");
    **/
    //_writeRead("PRESET NORM");
    /**
    _writeRead("ABORT\r\n");
    _writeRead("MCOUNT?\r\n");
    _writeRead("MEM CLR\r\n");
    
    asynStatus test = pasynOctetSyncIO->flush(pasynUserPort);
    if (test != asynSuccess)
    {
      printf("\n\nFailed to flushed the buffer");
      printf("\n\nError message? %s\n", pasynUserPort->errorMessage);
    }
    
    _writeRead("MEM FIFO\r\n");
    _writeRead("TARM AUTO\r\n");
    //_writeRead("FUNC DCV");//chatgpt
    //_writeRead("NPLC 1");//chatgpt
    //_writeRead("NRDGS 5, TIMER\r\n");
    //_writeRead("TIMER 1\r\n");
    _writeRead("NRDGS 5\r\n");
    _writeRead("TIMER 1\r\n");
    _writeRead("TRIG TIMER\r\n");

    _writeRead("ARM\r\n");
    sleep(10);
    //_writeRead("ID?");
    _writeRead("MCOUNT?\r\n");
    //_writeRead("RMEM 1,5\r\n");

    //_writeRead("MCOUNT?");
    **/ 
    //Gemini time lol fuck me i hate this 
    _writeRead("PRESET NORM\r\n");
    _writeRead("END ALWAYS\r\n");
    _writeRead("ID?");
    //_writeRead("TARM HOLD\r\n"); // might be useless
    _writeRead("TRIG HOLD\r\n");
    _writeRead("MEM FIFO\r\n");
    _writeRead("NPLC 10\r\n");
    _writeRead("NRDGS 5, TIMER\r\n");
    _writeRead("TIMER 3\r\n");
    _writeRead("MCOUNT?\r\n");
    _writeRead("TRIG SGL\r\n");
    sleep(17);
    printf("\n\nAfter sleep, should try rmem");
    _writeRead("MCOUNT?\r\n");
    _writeRead("ERRSTR?\r\n");
    //_writeRead("END OFF\r\n");
    //_writeRead("RMEM 1,5\r\n");
    //multimeterTask();
  }
  else if(parameter == P_NumSamples)
  {
    num_samples = value;
    sprintf(sendBuffer, "NRDGS %d, TIMER\r\n", num_samples);
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

  if (parameter == P_Rmem)
  {
    size_t nBytesOut, nBytesIn;
    int eomReason;
    printf("\n\nThe value we wrote was %f", value);
    pasynOctetSyncIO->flush(pasynUserPort);
    pasynOctetSyncIO->write(pasynUserPort, "RMEM 1,5\r\n", 8, 5.0, &nBytesOut);

    for (int i = 0; i < 5; i++)
    {
      asynStatus status = pasynOctetSyncIO->read(pasynUserPort, cmdBuffer, 8192, 5.0, &nBytesIn, &eomReason);
      printf("\nBuffer: %s, eomReason %d", cmdBuffer, eomReason);
    }
  }

  if (parameter == P_Beep)
  {
    sprintf(sendBuffer, "BEEP\r\n");
  }
  else if (parameter == P_TrigSgl)
  {
    //sprintf(sendBuffer, "TRIG SGL\r\n");
    _writeRead("MEM CLR\r\n");
    _writeRead("MCOUNT?\r\n");
    _writeRead("TRIG SGL\r\n");
    sleep(17);
    _writeRead("MCOUNT?\r\n");
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
