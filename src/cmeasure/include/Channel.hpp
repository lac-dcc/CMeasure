#ifndef CHANNEL_H
#define CHANNEL_H


//#include "stdafx.h"
#include <cstdlib>
#include <cstdio>
#include <string>
#include <assert.h>
#include <list>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <NIDAQmx.h> // include daqmx libraries

using namespace std;


#define CHANNEL_BUFFER_SIZE 500000L
/** NUMBER_OF_PULSES_TO_FINISH**/
#define CONTROL_VALUE 1
#define MAX_FILE_NAME_SIZE 100
#define NO_TRIGGER 0
#define TRIGGERED_ALL_POINTS 1
#define TRIGGERED_LAST_VALUE 2
#define TRIGGERED_TOTAL_ENERGY 3
#define TRIGGERED_AUTOMATIC 4
#define RESULT_VIEW_SCREEN 0
#define RESULT_VIEW_FILE 1
#define SINGLE_READ_TIMEOUT 5.0
#define SINGLE_READ_BUFER_SIZE 1000L
// Calculate the address of the ith sample of the channel with index
// chIdx in a buffer with nSamples samples and nCh channels.
#define GET_SAMPLE(i,chIdx,nSamples) ( (((nSamples))*(chIdx))+(i) )


class Channel  {
protected:
  float64 maxVoltage;
  string chName;
  string chId;
  vector<float64> samples;
  long ptSample;
  
public:
  float64 partial_sum;
  float64 partial_total_time;
  float64 partial_energy_HD;

  Channel(string chName, string chId, float64 maxVoltage) : samples(CHANNEL_BUFFER_SIZE) {
    this->maxVoltage = maxVoltage;
    this->chName = chName;
    this->chId = chId;
    ptSample = 0;
    partial_sum = 0.0;
    partial_total_time =0.0;
    partial_energy_HD = 0.0;
    
  }


  void writeLastValue(char* fileName); 

  int addSample(float64 sample);

  void writeOutputFile(string fileName, float64 sampleRate);

};

#endif