#include "../include/Channel.hpp"
// Channel

void Channel::writeLastValue(char* fileName) {
  FILE* output = fopen(fileName,"a+"); 
  fprintf(output,"%f %f\n",partial_total_time,partial_sum);
  fclose(output);
}

int Channel::addSample(float64 sample) {
  samples[ptSample] = sample;
   
  ptSample = ptSample + 1;
  // Buffer overflow
  if(ptSample == (CHANNEL_BUFFER_SIZE -1)) {
    printf("Warning: Channel %s buffer overflow\n ---Just for testing purpose---\n", chName.c_str());
    ptSample = 0;
    return (1); //houve estouro
  }

  return (0); //funcionamento normal
}

void Channel::writeOutputFile(string fileName, float64 sampleRate) {
  ofstream output;
  output.open(fileName);

  for(long i = 0; i != ptSample; i++) { 
    output << (ptSample*(1/sampleRate)) << "  " << samples[i] << endl; 
  }

  output.close();
}