#ifndef M_H
#define M_H

#include "ControlChannel.hpp"

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

  static std::string name(int id);

};

#endif