#include "stdafx.h"
#include <assert.h>
#include <cstdlib>
#include <cstdio>
#include <string>
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



double VSUPPLY = 5.0;
double SHUNT_GAIN = 10;
long SAMPLE_RATE = 48000

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



class ControlChannel : public Channel {
public:
  float64 startLevel;
  bool edge;
  bool active;
  
  ControlChannel(string chName, string chId,  float64 maxVoltage, 
		 float64 startLevel, bool edge) : Channel (chName, chId,  maxVoltage) {
    this->startLevel = startLevel;
    this->active = false;
    this->edge = edge;
  }

  void setState(float64 sample);

  int getBorder(float64 sample);

  bool trigger(float64 sample);

  bool untrigger(float64 sample);

};



class Measurement{
public:
  typedef map< string,Channel* > ChannelList;
  typedef map< string,Channel* >::iterator ChannelListIt;
  ChannelList channels;
  ControlChannel* ctlCh;
  TaskHandle taskHandle;
  float64 sampleRatePerChannel;
  float64 partial_sum;
  int timeout_s;
  int ctlIndex;
  int numberChannels;

  Measurement(string deviceName, float64 sampleRatePerChannel, int timeout_s){
    this->sampleRatePerChannel = sampleRatePerChannel;
    this->timeout_s = timeout_s;
    this->numberChannels = 0;
    this->partial_sum = 0.0;
    DAQmxErrChk(DAQmxResetDevice(deviceName.c_str()));
    DAQmxErrChk(DAQmxCreateTask("",&taskHandle));
  }


  void DAQmxErrChk(int32 error);

  void finishTasks();

  void resetMeasurement(float64 sampleRatePerChannel, int timeout_s);
  
  void addChannel(string name, string id, float64 maxVoltage );

  void addChannel(string name, string id, float64 maxVoltage, float64 startLevel, bool edge);

  void acquireMeasurementsNoTrigger(char* filename, float time_seconds);

  void acquireMeasurementsWithTrigger(char* filename, float time_seconds);

  char* acquireMeasurements(char* filename, int numer_of_measures = 1000);


  //-------------------------------------

  char* acquireLastValueMeasured(char* filename, int numer_of_measures = 1000);

  char* acquireAutomatic(char* filename, int numer_of_measures = 1000);

  char* acquireTotalEnergyMeasured(char* filename, int numer_of_measures = 1000);

  void startMeasure(int measurement_type ,  long numberOfSamples,  int result_view_mode, char* output_file_name = (char*)"output", std::string channel = "canalPrincipal");

  void writeOutputFile(char* filename, string channel);

  void writeLastValue(char* filename, string channel);

  int mean(char* filename);

  void chooseName(char* original_filename,char* outfilename);

};