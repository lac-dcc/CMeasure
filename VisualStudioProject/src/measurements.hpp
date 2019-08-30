#include "stdafx.h"
#include <assert.h>
#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include <stdio.h>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>


#include <NIDAQmx.h> // include daqmx libraries

using namespace std;
#define CHANNEL_BUFFER_SIZE 500000L
//#define SHUNT_GAIN 10.1112
//#define VSUPPLY 12.0
#define VSUPPLY 3.0
#define SHUNT_GAIN 1

/**--------SAMPLE RATE----------*/
#define SAMPLE_RATE 4800.0

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

  
public:
  Channel(string chName, string chId, float64 maxVoltage) : samples(CHANNEL_BUFFER_SIZE) {
    this->maxVoltage = maxVoltage;
    this->chName = chName;
    this->chId = chId;
    ptSample = 0;
    partial_sum = 0.0;
    partial_total_time =0.0;
    partial_energy_HD = 0.0;
    
  }


  void writeLastValue(char* fileName){
    FILE* output = fopen(fileName,"a+"); 
    fprintf(output,"%f %f\n",partial_total_time,partial_sum);
    fclose(output);
  }

  
  int addSample(float64 sample) {
    samples[ptSample] = sample;
    
    ptSample = ptSample + 1;
    // Buffer overflow
    if(ptSample == (CHANNEL_BUFFER_SIZE -1)){
      printf("Warning: Channel %s buffer overflow\n ---Just for testing purpose---\n", chName.c_str());
      ptSample = 0;
      return (1); //houve estouro
    }

    return (0); //funcionamento normal
  }

  void writeOutputFile(string fileName, float64 sampleRate){
    ofstream output;

    output.open(fileName);

    for(long i = 0; i != ptSample; i++){ 
      output << (ptSample*(1/sampleRate)) << "  " << samples[i] << endl; 
    }

    output.close();
  }

};


class ControlChannel : public Channel {
public:
  float64 startLevel;
  bool edge;
  bool active;

  void setState(float64 sample){
    active = (sample > startLevel)? edge : !edge;
  }

  int getBorder(float64 sample){
    bool lastState = active;
    setState(sample);
    // Positive border
    if(this->edge){
      // It is in level H
      if(lastState ) return (sample >startLevel)? 0 : -1;
      // It is in level L
      else return (sample < startLevel)? 0 : 1;
    // Neg border
    } else {
      // It is in level L
      if(lastState) return (sample < startLevel)? 0 : 1;
      // It is in level H
      else return (sample > startLevel)? 0 : -1;
    }

  }


public:
  /**
   * Initialize the control channel
   **/
  ControlChannel(string chName, string chId,  float64 maxVoltage, 
		 float64 startLevel, bool edge) : Channel (chName, chId,  maxVoltage) {
    this->startLevel = startLevel;
    this->active = false;
    this->edge = edge;
  }



  bool trigger(float64 sample){
    int borda = getBorder(sample);
    if ( (edge && (borda == 1)) ||
              (!edge && (borda == -1)) ) return true;
    else return false;
  }

  bool untrigger(float64 sample){
    int borda = getBorder(sample);
    return  ( (edge && (borda == -1)) ||
              (!edge &&(borda == 1)) );
  }

};

#define SINGLE_READ_TIMEOUT 5.0
#define SINGLE_READ_BUFER_SIZE 1000L
// Calculate the address of the ith sample of the channel with index
// chIdx in a buffer with nSamples samples and nCh channels.
#define GET_SAMPLE(i,chIdx,nSamples) ( (((nSamples))*(chIdx))+(i) )

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

  void DAQmxErrChk(int32 error){
    char        errBuff[4096]={'\0'};

    if( DAQmxFailed(error) ){
      finishTasks();
      DAQmxGetExtendedErrorInfo(errBuff,2048);
      cout << "DAQmx Error: " << errBuff << endl;;
      cout << "End of program, press Enter key to quit" << endl;
      getchar();

      exit(EXIT_FAILURE);
    }
  }

  void finishTasks(){
    if( taskHandle!=0 ) {
      /*********************************************/
      // DAQmx Stop Code
      /*********************************************/
      DAQmxStopTask(taskHandle);
      DAQmxClearTask(&taskHandle);

    }
  }


public:
  Measurement(string deviceName, float64 sampleRatePerChannel, int timeout_s){
    this->sampleRatePerChannel = sampleRatePerChannel;
    this->timeout_s = timeout_s;
    this->numberChannels = 0;
    this->partial_sum = 0.0;
    DAQmxErrChk(DAQmxResetDevice(deviceName.c_str()));
    DAQmxErrChk(DAQmxCreateTask("",&taskHandle));

  }

  void resetMeasurement(float64 sampleRatePerChannel, int timeout_s)
  {
    this->sampleRatePerChannel = sampleRatePerChannel;
    this->timeout_s = timeout_s;
    this->partial_sum = 0.0;
  }

  
  void addChannel(string name, string id, float64 maxVoltage ){
    this->channels[name] = new Channel(name, id, maxVoltage);
    this->numberChannels++;
    // Init the DAQmx Channel
    DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle,id.c_str(),name.c_str(),DAQmx_Val_Diff,
					  -maxVoltage,maxVoltage,DAQmx_Val_Volts,NULL));
    this->sampleRatePerChannel = SAMPLE_RATE/numberChannels;

  }

  void addChannel(string name, string id, float64 maxVoltage, float64 startLevel, bool edge){
    assert(ctlCh != NULL);
    ctlCh = new ControlChannel(name,id,maxVoltage, startLevel,edge);
    this->channels[name] = ctlCh;
    ctlIndex = numberChannels;
    this->numberChannels++;
    // Init the DAQmx Channel
    DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle,id.c_str(),name.c_str(),DAQmx_Val_Diff,
					  -maxVoltage,maxVoltage,DAQmx_Val_Volts,NULL));

  };



  void acquireMeasurementsNoTrigger(char* filename, float time_seconds){
    bool finished = false;
    float64 buffer[SINGLE_READ_BUFER_SIZE];
    char out[MAX_FILE_NAME_SIZE];
    int32 samplesRead;
    long totalSamplesRead = 0;
    // Config the timing
    cout<<"Measuring";
    DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle,"",this->sampleRatePerChannel,
				      DAQmx_Val_Rising,DAQmx_Val_ContSamps,SINGLE_READ_BUFER_SIZE));
    
    ofstream output;
    chooseName(filename,out);
    output.open(out);

    while(!finished){
      DAQmxErrChk(DAQmxReadAnalogF64(taskHandle,-1,SINGLE_READ_TIMEOUT,DAQmx_Val_GroupByChannel,
				     buffer,SINGLE_READ_BUFER_SIZE/this->numberChannels,&samplesRead,NULL));
      for(int currSample = 0; currSample < samplesRead; currSample++){
        int i = 0;
        for(ChannelListIt currCh = channels.begin(); currCh != channels.end(); currCh++,i++){
	      currCh->second->partial_sum += (VSUPPLY*SHUNT_GAIN*buffer[currSample]/sampleRatePerChannel); //sum energy
          currCh->second->partial_total_time += (1/sampleRatePerChannel); //sum time

        if (currCh == channels.begin()) output << currCh->second->partial_total_time << "  " << VSUPPLY*SHUNT_GAIN*buffer[currSample]/sampleRatePerChannel << endl;
    }
        
        totalSamplesRead++;
        if(totalSamplesRead >= time_seconds*SAMPLE_RATE){
	       finished = true;
	       break;
         }
	     }
    }
      

    output.close();
    cout<<"Finished";
   
    finishTasks();
  }

  void acquireMeasurementsWithTrigger(char* filename, float time_seconds){
    bool finished = false;
	bool started = false;
	bool firstSample = true;

    float64 buffer[SINGLE_READ_BUFER_SIZE];
    char out[MAX_FILE_NAME_SIZE];
    int32 samplesRead;
    long totalSamplesRead = 0;
	int currSample;

    // Config the timing
    DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle,"",this->sampleRatePerChannel,
				      DAQmx_Val_Rising,DAQmx_Val_ContSamps,SINGLE_READ_BUFER_SIZE));
    
    ofstream output;
    chooseName(filename,out);
    output.open(out);

	   // Wait for trigger
    while(!started){
      DAQmxErrChk(DAQmxReadAnalogF64(taskHandle,-1,SINGLE_READ_TIMEOUT,DAQmx_Val_GroupByChannel,
				     buffer,SINGLE_READ_BUFER_SIZE/this->numberChannels,&samplesRead,NULL));
      for(currSample= 0; currSample < samplesRead; currSample++){
	      // Check if control channel has been triggered
        if(firstSample){
          ctlCh->setState(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]);
          firstSample = false;
        }

        if( ctlCh->trigger(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]) == true ){
	        started = true;
          cout << "Triggered" << endl;
        }
      }
    }

	cout<< "Start Acquiring Data" << endl;
    // Sample channels 
    long savedSample = 0;
    int ret = 0;
    currSample--;

    while(!finished){
      DAQmxErrChk(DAQmxReadAnalogF64(taskHandle,-1,SINGLE_READ_TIMEOUT,DAQmx_Val_GroupByChannel,
				     buffer,SINGLE_READ_BUFER_SIZE/this->numberChannels,&samplesRead,NULL));
      for(int currSample = 0; currSample < samplesRead; currSample++){
        int i = 0;
        for(ChannelListIt currCh = channels.begin(); currCh != channels.end(); currCh++,i++){
	      currCh->second->partial_sum += (VSUPPLY*SHUNT_GAIN*buffer[currSample]/sampleRatePerChannel); //sum energy
          currCh->second->partial_total_time += (1/sampleRatePerChannel); //sum time

        if (currCh == channels.begin()) output << currCh->second->partial_total_time << "  " << VSUPPLY*SHUNT_GAIN*buffer[currSample]/sampleRatePerChannel << endl;
    }
        
        totalSamplesRead++;
        if(totalSamplesRead >= time_seconds*SAMPLE_RATE){
	       finished = true;
	       break;
         }
		savedSample++;
		  if( ctlCh->untrigger(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]) ){
	      finished = true;
          cout << "Untriggered" << endl;
	      break;
	      }
	     }

    }
      

    output.close();
    cout<<"Finished";
   
    finishTasks();
  }

  char* acquireMeasurements(char* filename, int numer_of_measures = 1000){
    DAQmxResetDevice("Dev3");
	char out[MAX_FILE_NAME_SIZE];

	for(int i = 0; i<numer_of_measures ;i++)
    {
	float64 buffer[SINGLE_READ_BUFER_SIZE];
    double energy = 0.0;
    int32 samplesRead=1;
    bool started = false;
    bool firstSample = true;
    bool finished = false;
    int currSample;

	// Config the timing
    DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle,"",this->sampleRatePerChannel,
				      DAQmx_Val_Rising,DAQmx_Val_ContSamps,SINGLE_READ_BUFER_SIZE));

	ofstream output;
    chooseName(filename,out);
    output.open(out);


   // Wait for trigger AQUI!!!
    while(!started){
		
      DAQmxErrChk(DAQmxReadAnalogF64(taskHandle,-1,SINGLE_READ_TIMEOUT,DAQmx_Val_GroupByChannel,
				     buffer,SINGLE_READ_BUFER_SIZE/this->numberChannels,&samplesRead,NULL));
      for(currSample= 0; currSample < samplesRead; currSample++){
	      // Check if control channel has been triggered
		 
        if(firstSample){
          ctlCh->setState(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]);
          firstSample = false;
		  started = true;
        }

        if( ctlCh->trigger(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]) == true ){
	        started = true;
         // cout << "Triggered" << endl;
        }
      }
    }

    //cout<< "Start Acquiring Data" << endl;

	//cout << "Sample Rate: " << sampleRatePerChannel << endl;
	//cout << "Sample Read: " << samplesRead << endl;
    // Sample channels 
    long savedSample = 0;
    int ret = 0;
    currSample--;
    while(!finished){
      // currSample reads loops through the number of samples taken for a single channel
      // It is not set to 0 here because a trigger may have happened in the middle of a read
      for(; currSample < samplesRead; currSample++){
		    int i=0;
        for(ChannelListIt currCh = channels.begin(); currCh != channels.end(); currCh++,i++){
			    currCh->second->addSample(buffer[GET_SAMPLE(currSample,i,samplesRead)]);
				currCh->second->partial_total_time += (1/sampleRatePerChannel); //sum time
				 if (currCh == channels.begin()) 
				 {
					 energy += VSUPPLY*SHUNT_GAIN*buffer[currSample]/sampleRatePerChannel;
					 output << currCh->second->partial_total_time << "  " << VSUPPLY*SHUNT_GAIN*buffer[currSample] << endl;
				 }
            
				 
	      }	
	      savedSample++;
	      if( ctlCh->untrigger(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]) ){
	        finished = true;
          //cout << "Untriggered" << endl;
	        break;
	      }
	
      }
      DAQmxErrChk(DAQmxReadAnalogF64(taskHandle,-1,SINGLE_READ_TIMEOUT,DAQmx_Val_GroupByChannel,
				     buffer,SINGLE_READ_BUFER_SIZE,&samplesRead,NULL));
      currSample = 0;
    }	

	cout << i << ":  " << energy << endl;

	output.close();
    finishTasks();
	}
    return out;
  
  }


  //-------------------------------------

  char* acquireLastValueMeasured(char* filename, int numer_of_measures = 1000){

    float64 time  = 0;
    float64 energy = 0;
    float64 energy_HD = 0;
    char output[MAX_FILE_NAME_SIZE];

    chooseName(filename,output);
    FILE* out = fopen(output,"a+");

    for(int i = 0; i<numer_of_measures ;i++)
    {

    float64 buffer[SINGLE_READ_BUFER_SIZE];
    float64 buffer_HD[SINGLE_READ_BUFER_SIZE];
    int32 samplesRead;
    bool started = false;
    bool firstSample = true;
    bool finished = false;
    int currSample;
    
    // Config the timing
    DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle,"",this->sampleRatePerChannel,
				      DAQmx_Val_Rising,DAQmx_Val_ContSamps,SINGLE_READ_BUFER_SIZE));

    cout<<"Waiting for  trigger os mesaure number " << i <<endl;

    // Wait for trigger
    while(!started){
      DAQmxErrChk(DAQmxReadAnalogF64(taskHandle,-1,SINGLE_READ_TIMEOUT,DAQmx_Val_GroupByChannel,
				     buffer,SINGLE_READ_BUFER_SIZE/this->numberChannels,&samplesRead,NULL));
      for(currSample= 0; currSample < samplesRead; currSample++){
	      // Check if control channel has been triggered
        if(firstSample){
          ctlCh->setState(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]);
          firstSample = false;
        }

        if( ctlCh->trigger(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]) == true ){
	        started = true;
          cout << "Triggered" << endl;
        }
      }
    }
    cout<< "Start Acquiring Data" << endl;
    // Sample channels 
    long savedSample = 0;
    int ret = 0;
    currSample--;
    while(!finished){
      // currSample reads loops through the number of samples taken for a single channel
      // It is not set to 0 here because a trigger may have happened in the middle of a read
      for(; currSample < samplesRead; currSample++){
		    int i=0;
        for(ChannelListIt currCh = channels.begin(); currCh != channels.end(); currCh++,i++){
          currCh->second->partial_sum += (VSUPPLY*SHUNT_GAIN*buffer[currSample]/sampleRatePerChannel); //sum energy
          currCh->second->partial_total_time += (1/sampleRatePerChannel); //sum time
          currCh->second->partial_energy_HD += (5*SHUNT_GAIN*(buffer[GET_SAMPLE(currSample,2,samplesRead)])/sampleRatePerChannel); //sum energy
          
	      }	
	      if( ctlCh->untrigger(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]) ){ //#define GET_SAMPLE(i,chIdx,nSamples) ( (((nSamples))*(chIdx))+(i) ) 
	        finished = true;
          cout << "Untriggered" << endl;
	        break;
	      }
	
      }
      DAQmxErrChk(DAQmxReadAnalogF64(taskHandle,-1,SINGLE_READ_TIMEOUT,DAQmx_Val_GroupByChannel,
				     buffer,SINGLE_READ_BUFER_SIZE,&samplesRead,NULL));
      currSample = 0;



      //--------------------------------------------------------------------------------------

    }

    
    
    fprintf(out,"%f %f\n",ctlCh->partial_total_time,channels["canalPrincipal"]->partial_sum);
    fflush(out);
    cout<<"Finished mesaure number" << i <<endl<<endl;
   

    ctlCh->partial_total_time = 0;
    ctlCh->partial_sum = 0;
    ctlCh->partial_energy_HD = 0;

    channels["canalPrincipal"]->partial_sum = 0;
    channels["canalPrincipal"]->partial_energy_HD = 0;
    

    }//------endloop------

    fclose(out);
  finishTasks();

  return (output);
  }


  char* acquireAutomatic(char* filename, int numer_of_measures = 1000){

    float64 time  = 0;
    float64 energy = 0;
    float64 energy_HD = 0;
	int ctrl_end = 0;
    char output[MAX_FILE_NAME_SIZE];

    chooseName(filename,output);
    FILE* out = fopen(output,"a+");

    for(int i = 0; i<numer_of_measures ;i++)
    {

    float64 buffer[SINGLE_READ_BUFER_SIZE];
    float64 buffer_HD[SINGLE_READ_BUFER_SIZE];
    int32 samplesRead;
    bool started = false;
    bool firstSample = true;
    bool finished = false;
    int currSample;
    
    // Config the timing
    DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle,"",this->sampleRatePerChannel,
				      DAQmx_Val_Rising,DAQmx_Val_ContSamps,SINGLE_READ_BUFER_SIZE));

    cout<<"Waiting for  trigger os mesaure number " << i <<endl;

    // Wait for trigger
    while(!started){
      DAQmxErrChk(DAQmxReadAnalogF64(taskHandle,-1,SINGLE_READ_TIMEOUT,DAQmx_Val_GroupByChannel,
				     buffer,SINGLE_READ_BUFER_SIZE/this->numberChannels,&samplesRead,NULL));
      for(currSample= 0; currSample < samplesRead; currSample++){
	      // Check if control channel has been triggered
        if(firstSample){
          ctlCh->setState(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]);
          firstSample = false;
        }

        if( ctlCh->trigger(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]) == true ){
	        started = true;
          cout << "Triggered" << endl;
        }
      }
    }
    cout<< "Start Acquiring Data" << endl;
    // Sample channels 
    long savedSample = 0;
    int ret = 0;
    currSample--;
    while(!finished){
      // currSample reads loops through the number of samples taken for a single channel
      // It is not set to 0 here because a trigger may have happened in the middle of a read
      for(; currSample < samplesRead; currSample++){
		    int i=0;
        for(ChannelListIt currCh = channels.begin(); currCh != channels.end(); currCh++,i++){
          currCh->second->partial_sum += (VSUPPLY*SHUNT_GAIN*buffer[currSample]/sampleRatePerChannel); //sum energy
          currCh->second->partial_total_time += (1/sampleRatePerChannel); //sum time
          currCh->second->partial_energy_HD += (5*SHUNT_GAIN*(buffer[GET_SAMPLE(currSample,2,samplesRead)])/sampleRatePerChannel); //sum energy
          
	      }	
	      if( ctlCh->untrigger(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]) ){ //#define GET_SAMPLE(i,chIdx,nSamples) ( (((nSamples))*(chIdx))+(i) ) 
	        
			  ctrl_end++;
			  if (ctrl_end == CONTROL_VALUE)
			  {
				  ctrl_end = 0;
				finished = true;
				cout << "Untriggered" << endl;
				break;
			  }
	      }
	
      }
      DAQmxErrChk(DAQmxReadAnalogF64(taskHandle,-1,SINGLE_READ_TIMEOUT,DAQmx_Val_GroupByChannel,
				     buffer,SINGLE_READ_BUFER_SIZE,&samplesRead,NULL));
      currSample = 0;



      //--------------------------------------------------------------------------------------

    }

    
    
    fprintf(out,"%f %f\n",ctlCh->partial_total_time,channels["canalPrincipal"]->partial_sum);
    fflush(out);
    cout<<"Finished mesaure number" << i <<endl<<endl;
   

    ctlCh->partial_total_time = 0;
    ctlCh->partial_sum = 0;
    ctlCh->partial_energy_HD = 0;

    channels["canalPrincipal"]->partial_sum = 0;
    channels["canalPrincipal"]->partial_energy_HD =- 0;
    

    }//------endloop------

    fclose(out);
  finishTasks();

  return (output);
  }



  char* acquireTotalEnergyMeasured(char* filename, int numer_of_measures = 1000){

    float64 time  = 0;
    float64 energy = 0;
    char output[MAX_FILE_NAME_SIZE];

    for(int i = 0; i<numer_of_measures ;i++)
    {

    float64 buffer[SINGLE_READ_BUFER_SIZE];
    int32 samplesRead;
    bool started = false;
    bool firstSample = true;
    bool finished = false;
    int currSample;
    
    // Config the timing
    DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle,"",this->sampleRatePerChannel,
				      DAQmx_Val_Rising,DAQmx_Val_ContSamps,SINGLE_READ_BUFER_SIZE));

    cout<<"Waiting for  trigger os mesaure number " << i <<endl;

    // Wait for trigger
    while(!started){
      DAQmxErrChk(DAQmxReadAnalogF64(taskHandle,-1,SINGLE_READ_TIMEOUT,DAQmx_Val_GroupByChannel,
				     buffer,SINGLE_READ_BUFER_SIZE/this->numberChannels,&samplesRead,NULL));
      for(currSample= 0; currSample < samplesRead; currSample++){
	      // Check if control channel has been triggered
        if(firstSample){
          ctlCh->setState(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]);
          firstSample = false;
        }

        if( ctlCh->trigger(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]) == true ){
	        started = true;
          cout << "Triggered" << endl;
        }
      }
    }
    cout<< "Start Acquiring Data" << endl;
    // Sample channels 
    long savedSample = 0;
    int ret = 0;
    currSample--;
    while(!finished){
      // currSample reads loops through the number of samples taken for a single channel
      // It is not set to 0 here because a trigger may have happened in the middle of a read
      for(; currSample < samplesRead; currSample++){
		    int i=0;
        for(ChannelListIt currCh = channels.begin(); currCh != channels.end(); currCh++,i++){
          currCh->second->partial_sum += (VSUPPLY*SHUNT_GAIN*buffer[currSample]/sampleRatePerChannel); //sum energy
          currCh->second->partial_total_time += (1/sampleRatePerChannel); //sum time
	      }	
	      if( ctlCh->untrigger(buffer[GET_SAMPLE(currSample,ctlIndex,samplesRead)]) ){
	        finished = true;
          cout << "Untriggered" << endl;
	        break;
	      }


	
      }
      DAQmxErrChk(DAQmxReadAnalogF64(taskHandle,-1,SINGLE_READ_TIMEOUT,DAQmx_Val_GroupByChannel,
				     buffer,SINGLE_READ_BUFER_SIZE,&samplesRead,NULL));
      currSample = 0;

      
      energy = ctlCh->partial_sum;
      time = ctlCh->partial_total_time;

    }


    }//------endloop------

    
    chooseName(filename,output);
    FILE* out = fopen(output,"a+");
    fprintf(out,"%f %f\n",time,energy);
    fflush(out);
    fclose(out);
    
  finishTasks();
  return(output);
  }


  void startMeasure(int measurement_type ,  long numberOfSamples,  int result_view_mode, char* output_file_name = (char*)"output", std::string channel = "canalPrincipal")
  {
    char buff[100];
    switch(measurement_type)
    {
    case(0): { acquireMeasurementsNoTrigger(output_file_name,numberOfSamples);  break;}
    case(1): { acquireMeasurements(output_file_name, numberOfSamples); break; }
    case(2): { strcpy(buff, acquireLastValueMeasured(output_file_name, numberOfSamples)); mean(buff);  break; }
    case(3): { acquireTotalEnergyMeasured(output_file_name, numberOfSamples);  break; }
	case(4): { strcpy(buff, acquireAutomatic(output_file_name, numberOfSamples)); mean(buff);  break; }
    }
  }

  void writeOutputFile(char* filename, string channel){
    channels[channel]->writeOutputFile(filename,sampleRatePerChannel);
  }

  void writeLastValue(char* filename, string channel){
    channels[channel]->writeLastValue(filename);
  }


  int mean(char* filename)
  {
    
  FILE* input;
  input = fopen(filename,"r+");
  if( input == NULL ) { cout<< "File not found"<<endl; system("pause"); return(EXIT_FAILURE); }
  
  float sum_energy = 0.0;
  float sum_time = 0.0;
  float mean_energy = 0.0;
  float mean_time = 0.0;
  int iter = 0;
  int i = 0;
  while(!feof(input))
  {
    fscanf(input,"%f",&sum_time);
    fscanf(input,"%f",&sum_energy);

    mean_energy += sum_energy;
    mean_time += sum_time;

    i++;
  }

  mean_energy /= i;
  mean_time /= i;

  fprintf(input,"\n\nTempo Medio: %f  -- Energia Media:  %f", mean_time, mean_energy);
  fclose(input);
  }



  void chooseName(char* original_filename,char* outfilename)
  {
    FILE* in;
    char curr_filename[MAX_FILE_NAME_SIZE];
    strcpy(curr_filename,original_filename);
    char buff[MAX_FILE_NAME_SIZE];
    char buff_opening[MAX_FILE_NAME_SIZE];
    for(int i =1;i<100;i++)
    {
      strcpy(buff_opening,curr_filename);
      strcat(buff_opening,".txt");
      in = fopen(buff_opening,"r");

      if( in == NULL) { strcpy(outfilename,curr_filename); strcat(outfilename,".txt"); return; }

      strcpy(curr_filename,original_filename);
      
      itoa(i,buff,10);
      strcat(curr_filename,buff);
      fclose(in);
    }
  }





};