//flipcoil_util.cpp

#include "flipcoil_util.h"
#include "flipcoil.h"
#include<unistd.h>
#include <sstream>

/**
 * @brief Function for thread calculating field strength
 *
 * This function sends commands to an Hp3458a multimeter,
 * These commands set the multimeter to a known state and then 
 * prompt the multimeter to take some amount of readings, as set by 
 * the user through a PV otherwise a default value is used.
 * These readings are then used to calculate field strength 
 * readings which are stored into separate EPICS PVs for retreival
 * by the user.
 **/
void FlipCoilDriver::multimeterTask(void)
{
  // Block of commands for setting the multimeter to a known state 
  asynPrint(pasynUserPort, ASYN_TRACE_FLOW, "Beginning the multimeter task\n");
  pasynOctetSyncIO->flush(pasynUserPort); //Purge command buffer of incomplete data 
  _writeRead("PRESET NORM\r\n"); //Set multimeter to known settings defined in more detail in manual 
  _writeRead("TRIG HOLD\r\n"); //Tells multimeter to wait for trigger signal
  _writeRead("MEM FIFO\r\n"); //Turns memory on in the multimeter, stores samples first in first out 
  _writeRead("NPLC 10\r\n"); //Add some noise filtering 
  _writeRead("NRDGS 170, TIMER\r\n"); //Tells the multimeter it take an amount of readings with a gap indicated by the timer between each reading 
  _writeRead("TIMER 1\r\n"); //Tells the multimeter how long to wait between each reading 
  int coil_delta = 1;
  //Assorted variable declarations 
  float prev = 0; //Storage for the previous variable 
  vector<float> pos_samples; //Vector for positive samples of the waveform
  vector<float> neg_samples; //Vector for negative samples of the waveform 
  int crossings = 0; //Tracker for seen zero crossings 
  size_t nBytesOut, nBytesIn; //Variables for pasynOctet calls 
  int eomReason; // Another variable for pasynOctet calls, call will put if terminal character encountered in this variable 
 

  //While loop where we take readings and isolate one period of the waveform we're analyzing 
  while (crossings < 3)
  {
    _writeRead("TRIG SGL\r\n"); //Command triggers the multimeter to begin readings 
    sleep(172); //TODO: define this based on user provided Timer time 
    pasynOctetSyncIO->write(pasynUserPort, "RMEM 1,170;\r\n", 11, 5.0, &nBytesOut); //Command that tells the multimeter to send it's readings over gpib to us 
    //For loop that retrieves readings from the multimeter TODO: Define this on user provided num samples.
    for (int i = 0; i < 170; i++)
    {
      //Read command, if there's an error well shoot
      asynStatus read_status = pasynOctetSyncIO->read(pasynUserPort, cmdBuffer, CMD_BUFFER_SIZE, 5.0, &nBytesIn, &eomReason);
      if (read_status != asynSuccess)
      {
        asynPrint(pasynUserPort, ASYN_TRACE_ERROR, "Unable to read from multimeter, please restart thread and multimeter, error message: %s\n", pasynUserPort->errorMessage);
        return;
      }

      //process the readings, strip the comma, convert to a float 
      char * token = strtok(cmdBuffer, ",");
      float reading = stof(token);
      asynPrint(pasynUserPort, ASYN_TRACE_FLOW, "Reading: %f, reading no: %d, crossing: %d\n", reading, i, crossings);
      
      //Check for a zero crossing, basically pos * neg = neg so anytime current reading * prev reading < 0 we've got a zero crossing 
      if (reading * prev < 0)
      {
        crossings += 1;
      }
      if (crossings >= 3)
      {
        asynPrint(pasynUserPort, ASYN_TRACE_FLOW, "Read complete cycle\n");
        continue;
      }
      //Check we've found at least one zero crossing before storing data, if we haven't this reading's cycle will be incomplete.
      else if (crossings > 0)
      {
        if (reading < 0)
        {
          neg_samples.push_back(-1 * reading); // we multiply by -1 to make values positive so that the integral calc works properly 
        }
        else 
        {
          pos_samples.push_back(reading);
        }
      }
      prev = reading;
    }
    // TODO: Figure out if this is necessary,best practice would be get enough samples that we don't need another batch
    pasynOctetSyncIO->write(pasynUserPort, "MEM FIFO\r\n", 8, 5.0, &nBytesOut); //In case we need another set of samples, turn the memory back on/

  }
  //Then the time integral is calculated within coil_samples coilintpeak
  float pos_peak = coilIntPeak(coil_delta, pos_samples);
  
  //Third a a time integral of the negative samples is calculated coilintpeak
  float neg_peak = -1 * coilIntPeak(coil_delta, neg_samples);
  
  //In theory we'll want to track over time or do multiple measurements, store measurements from measured cycle for further use.
  printf("Calculated Pos Peak: %f Calculated Neg Peak: %f\n\n", pos_peak, neg_peak);
  if (pos_peak > 0 )
  {
    vt_pos_mult.push_back(pos_peak);
    vt_neg_mult.push_back(neg_peak);
  }
  else 
  {
    vt_neg_mult.push_back(pos_peak);
    vt_pos_mult.push_back(neg_peak);
  }
  vt_avg_mult.push_back((vt_pos_mult.back() - vt_neg_mult.back()) / 2);

  //Finding the mean of integrated voltage values
  float sum = 0;
  for(float x : vt_avg_mult)
  {
    sum += x;
  }

  //TODO Pass this by reference? or make the task return this, im not sure what it's used for
  float avg = sum / vt_avg_mult.size();
  Avg_Int_Mult.store(avg);
  printf("\n\n\nAveraged integral %g", avg);
  num_measurements += 1;
  //Finding standard deviation
  sum = 0;
  for(float x : vt_avg_mult)
  {
    sum += pow((x - avg), 2);
  }
  float std_dev = sqrt(sum / num_measurements);
  Std_Dev = std_dev;
}

/**
 * @brief Isolates a the first peak value in a sine wave or similar function
 * modifies passed in start end and peak values to correspond with
 * zero crossing points and the found peak value.
 * 
 * @param start Variable to store the index of the first encountered zero crossing before the peak
 * @param end   Variable to store the index of the first zero crossing after the peak
 * @param peak  Variable to store the index of the found peak value 
 * @param samples Vector that contains the wave data we search through to find start end and peak indices
 */
void peakIsolator(int& start, int& end, int& peak, vector<float> samples)
{
  int total_samples = samples.size();
  peak = -1;
  end = -1;
  start = -1;
  //Find peak
  for(int i = 1; i < total_samples; i++)
  {
    if(samples[i] > samples[peak])
    {
      peak = i;
    }
  }
  start = 0;
  for(int j = peak; j >= 1; j--)
  {
    if(samples[j-1] <= 0 && samples[j] >= 0 )
    {
      start = j -1;
      break;
    }
  }

  for(int k = peak; k < total_samples-1; k++)
  {
    if(samples[k] >= 0 && samples[k+1] <= 0)
    {
      end = k+1;
      break;
    }
  }
}
/**
 * @brief calculates an integral of the voltage generated by the rotating coil 
 *
 * @param coil_delta time between voltage samples
 * @param voltage_samples sampled voltages
 * @return Calculated integral of the voltage 
 */
float coilIntPeak(float coil_delta, vector<float> &voltage_samples)
{

  //So the original way was a little bit weird
  //Just going to do a full trapezoid rule for the entire hting
  //Rather than mix and match 3 different integrals
  //printf("\n\n\n\n\n");
  float to_ret = 0;
  for(int i = 0; i < int(voltage_samples.size()); i++)
  {
    printf("Sample, %d, %f\n", i, voltage_samples[i]);
    if(voltage_samples[i] < 0)
    {
      continue;
    }
    if(i == 0 || i == int(voltage_samples.size()) - 1)
    {
      to_ret += voltage_samples[i];

    }
    else
    {
      to_ret += 2*voltage_samples[i];
    }

  }
  printf("\n\nCalculated integral to return, %f, coil delta %f", to_ret, coil_delta);
  
  return (coil_delta / 2) * to_ret;
}

/**
 * @brief Generates a sine wave stored in coil_samples.
 *
 * @param coil_samples: vector that generated values of a sine wave can be placed
 **/
void sineWaveTester(vector<float>& coil_samples)
{
  //Do something in here that inserts a sine wave at some point
  const float pi = 3.1415926535;
  const int amplitude = 1;
  const float offset = 2 * pi / 10;
  for(int i = 0; i < 200; i++)
  {
    coil_samples.push_back(amplitude * sin(2 * pi * i / 200 + offset));
  }
}
