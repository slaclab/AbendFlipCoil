//flipcoil_util.cpp

#include "flipcoil_util.h"
#include "flipcoil.h"
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
float coilIntPeak(float coil_delta, vector<float> voltage_samples)
{

  //So the original way was a little bit weird
  //Just going to do a full trapezoid rule for the entire hting
  //Rather than mix and match 3 different integrals
  //printf("\n\n\n\n\n");
  float to_ret = 0;
  for(int i = 0; i < voltage_samples.size(); i++)
  {
    if(voltage_samples[i] < 0)
    {
      continue;
    }
    if(i == 0 || i == voltage_samples.size() - 1)
    {
      to_ret += voltage_samples[i];

    }
    else
    {
      to_ret += 2*voltage_samples[i];
    }

  }
  //printf("\n\n\n\n\n\n");
  
  return (coil_delta / 2) * to_ret;
}

void sineWaveTester(vector<float>& coil_samples)
{
  //Do something in here that inserts a sine wave at some point
  const float pi = 3.1415926535;
  const int amplitude = 1;
  const float offset = 2 * pi / 10;
  for(int i = 0; i < COIL_SAMPLES; i++)
  {
    coil_samples.push_back(amplitude * sin(2 * pi * i / COIL_SAMPLES + offset));
  }
}
