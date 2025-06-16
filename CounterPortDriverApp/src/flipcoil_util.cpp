//flipcoil_util.cpp

#include "flipcoil_util.h"
#include "flipcoil.h"

void peakIsolator(int& start, int& end, int& peak, vector<float> samples)
{
  int total_samples = samples.size();
  for(int i = 1; i < total_samples; i++)
  {
    
  }
}
float coilIntPeak(int samples, float coil_delta, vector<float> voltage_samples)
{
  
  return 0.0;
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
